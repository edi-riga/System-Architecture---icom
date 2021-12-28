#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

#include "icom.h"
#include "notification.h"
#include "simple_timer.h"

#define AVERAGING_TEST_COUNT     (10)
#define TEST_SIZE_MIN            (4)
#define TEST_SIZE_LOG_INCREMENTS (23)
#define TEST_SIZE_MAX            (TEST_SIZE_MIN << TEST_SIZE_LOG_INCREMENTS)
#define STATIC_ARRAY_SIZE(a)     (sizeof(a)/sizeof(*a))


////////////////////////////////////////////////////////////////////////////////
// CUSTOM TYPE DEFINITIONS
////////////////////////////////////////////////////////////////////////////////
/* thread data structure */
typedef struct {
  icom_t    *icom;
  void      *buf;
  unsigned   bufSize;
} threadPdata_t;


////////////////////////////////////////////////////////////////////////////////
// GLOBALS
////////////////////////////////////////////////////////////////////////////////
/* communication scenarios for benchmarking */
const char *g_com_strings[][2] = {
  {"socket_tx|default|127.0.0.1:8889", "socket_rx|default|*:8889"},
};


////////////////////////////////////////////////////////////////////////////////
// DISPLAYING RESULTS TO THE TERMINAL
////////////////////////////////////////////////////////////////////////////////
float disp_bytesGetNum(uint64_t bytes){
  if(bytes >= 1024*1024*1024){   // GB
    return bytes/(1024*1024*1024);

  } else if(bytes >= 1024*1024){ // MB
    return bytes/(1024*1024);

  } else if(bytes >= 1024){      // KB
    return bytes/(1024);

  } else {                       // bytes
    return bytes;
  } 
}

const char* disp_bytesGetUnits(uint64_t bytes){
  if(bytes >= 1024*1024*1024){   // GB
    return "GB";

  } else if(bytes >= 1024*1024){ // MB
    return "MB";

  } else if(bytes >= 1024){      // KB
    return "KB";

  } else {                       // bytes
    return "B";
  } 
}

float disp_speedGetNum(uint64_t bps){
  if(bps >= 1000*1000*1000){   // Gbps
    return bps/(1000*1000*1000);

  } else if(bps>= 1000*1000){  // Mbps
    return bps/(1000*1000);

  } else if(bps>= 1000){       // Kbps
    return bps/(1000);

  } else {                     // bps
    return bps;
  } 
}

const char* disp_speedGetUnits(uint64_t bps){
  if(bps>= 1000*1000*1000){    // Gbps
    return "Gbps";

  } else if(bps >= 1000*1000){ // Mbps
    return "Mbps";

  } else if(bps >= 1000){      // Kbps
    return "Kbps";

  } else {                     // bps
    return "bps";
  } 
}

static inline void disp_scenarios(){
  _I("### SCENARIOS ###");
  for(int i=0; i<STATIC_ARRAY_SIZE(g_com_strings); i++){
    _I("Scenario - %d (Tx: \"%s\", Rx: \"%s\")", 0, g_com_strings[i][0], g_com_strings[i][1]);
  }
}

static inline void disp_results(
uint64_t transferSizes[TEST_SIZE_LOG_INCREMENTS],
uint64_t timing[STATIC_ARRAY_SIZE(g_com_strings)][TEST_SIZE_LOG_INCREMENTS])
{
  _I("### RESULT TABLE ###"); 
  /* header */
  printf(" S |");
  for(int i=0; i<TEST_SIZE_LOG_INCREMENTS; i++){
    printf("%5.1f %-4s|", 
      disp_bytesGetNum(transferSizes[i]),
      disp_bytesGetUnits(transferSizes[i]));
  }

  /* body */
  for(int s=0; s<STATIC_ARRAY_SIZE(g_com_strings); s++){
    printf("\n%2u:|", s);
    for(int i=0; i<TEST_SIZE_LOG_INCREMENTS; i++){
      printf("%5.1f %-4s|", 
        disp_speedGetNum(transferSizes[i]/(((float)timing[s][i])/10e6)),
        disp_speedGetUnits(transferSizes[i]/(((float)timing[s][i])/10e6)));
  }}

  putchar('\n');
}


////////////////////////////////////////////////////////////////////////////////
// PLOTTING
////////////////////////////////////////////////////////////////////////////////
int plot(const char *title, uint64_t *xdata, uint64_t *ydata, uint32_t sampleCount){
  FILE *fd_gnuplot;
  const char *commands[] = {
    "set style line 1 linecolor rgb '#0060ad' linetype 1 linewidth 2 pointtype 7 pointsize 1.5"
  };

  /* open gnuplot for pipeing in persistent mode (will continue execution) */
  fd_gnuplot = popen("gnuplot -persistent", "w");
  if(!fd_gnuplot){
    _E("Failed to open gnuplot");
    return -1;
  }

  /* pipe random commands */
  for(int i=0; i<sizeof(commands)/sizeof(*commands); i++){
    _I("%s", commands[i]);
    fprintf(fd_gnuplot, "%s \n", commands[i]);
  }

  /* pipe plot command */
  fprintf(fd_gnuplot, "plot '-' with linespoints linestyle 1\n");

  /* pipe data */
  for(int i=0; i<sampleCount; i++){
    fprintf(fd_gnuplot, "%lu %lu \n", xdata[i], ydata[i]);
  }

  /* pipe title */
  fprintf(fd_gnuplot, "e\n");
  fprintf(fd_gnuplot, "set title \"%s\"\n", title);
  fprintf(fd_gnuplot, "refresh\n");

  pclose(fd_gnuplot);
  return 0;
}


////////////////////////////////////////////////////////////////////////////////
// EXPERIMENTS / BENCHMARKING 
////////////////////////////////////////////////////////////////////////////////

/* data sender thread */
void* thread_send(void *p){
  threadPdata_t *pdata = (threadPdata_t*)p;
  return (void*)icom_send(pdata->icom, pdata->buf, pdata->bufSize);
}

icomStatus_t test(uint64_t *timeUs, const char *comStrings[2], uint32_t transferSize, int fdRandom){
  icom_t *icomTx, *icomRx;
  icomStatus_t ret = ICOM_SUCCESS;
  threadPdata_t threadPdata;
  uint8_t *bufTx, *bufRx;
  pthread_t pid;
  int bytes, bytes_total=0;
  int64_t retThread;

  /* initialize communicators */
  icomTx = icom_init(comStrings[0]);
  if(ICOM_IS_ERR(icomTx)){
    _E("Failed to initialize Tx communicator");
    return ICOM_PTR_ERR(icomTx);
  }
  icomRx = icom_init(comStrings[1]);
  if(ICOM_IS_ERR(icomRx)){
    _E("Failed to initialize Rx communicator");
    ret = ICOM_PTR_ERR(icomRx);
    goto cleanup_icom_init;
  }

  /* initialize data */
  bufTx = (uint8_t*)malloc(transferSize);
  if(!bufTx){
    _E("Failed to allocate Tx buffer memory: %u", transferSize);
    ret = ICOM_ENOMEM;
    goto cleanup_malloc_tx;
  }
  do{
    bytes = read(fdRandom, bufTx, transferSize-bytes_total);
    bytes_total += bytes;
  } while((bytes_total != transferSize) && (bytes == -1));

  if((bytes_total != transferSize) || (bytes == -1)){
    _SE("Failed to initialize buffer memory");
    ret = ICOM_ERROR;
    goto cleanup_read;
  }
 
  /* run experiment */
  threadPdata = (threadPdata_t){icomTx=icomTx, bufTx, transferSize};
  stimer_set();
  pthread_create(&pid, NULL, thread_send, &threadPdata);
  ret = icom_recv(icomRx, (void**)&bufRx, &bytes);
  *timeUs = stimer_get_us();
  pthread_join(pid, (void**)&retThread);
  if(ret != ICOM_SUCCESS){
    _E("Failed to receive data");
    ret = ICOM_ERROR;
    goto cleanup_icom_recv;
  }
  if(bytes != transferSize){
    _E("Reveived incorrect size (%u, expected: %u)", bytes, transferSize);
    ret = ICOM_ERROR;
    goto cleanup_icom_recv;
  }
  if(memcmp(bufTx, bufRx, transferSize) != 0){
    _E("Sent / received data mismatch");
    ret = ICOM_ERROR;
    goto cleanup_icom_recv;
  }
  if(retThread != ICOM_SUCCESS){
    _E("Transmitter error");
    ret = ICOM_ERROR;
    goto cleanup_icom_recv;
  }
  

  ret = ICOM_SUCCESS;

cleanup_icom_recv:
cleanup_read:
  free(bufTx);
cleanup_malloc_tx:
  icom_deinit(icomRx);
cleanup_icom_init:
  icom_deinit(icomTx);
  return ret;
}


int main(void){
  icomStatus_t status;
  uint64_t times[STATIC_ARRAY_SIZE(g_com_strings)][TEST_SIZE_LOG_INCREMENTS] = {0};
  uint64_t sizes[TEST_SIZE_LOG_INCREMENTS];
  uint64_t time;
  int fd;

  /* fill sizes array (used later for plotting) */
  for(int i=0, size=TEST_SIZE_MIN; size<TEST_SIZE_MAX; size=size<<1, i++){
    sizes[i] = size;
  }

  /* initialize file descriptor for generating random data */
  fd = open("/dev/urandom", O_RDONLY);
  if(fd == -1){
    _SE("Failed to open \"%s\"", "/dev/random");
    return 1;
  }

  /* run all the tests */
  for(int s=0; s<STATIC_ARRAY_SIZE(g_com_strings); s++){
  for(int i=0; i<TEST_SIZE_LOG_INCREMENTS; i++){
    _I("Performing %u tests for %lu bytes", AVERAGING_TEST_COUNT, sizes[i]);
    for(int j=0; j<AVERAGING_TEST_COUNT; j++){
      status = test(&time, g_com_strings[0], sizes[i], fd);
      if(status != ICOM_SUCCESS){
        _E("Test failed");
        continue;
      }
      times[s][i] += time;
    }
    times[s][i] /= TEST_SIZE_LOG_INCREMENTS;
  }}


  /* print scenarios and results to the terminal */
  disp_scenarios();
  disp_results(sizes, times);

  /* cleanup */
  close(fd);

  return 0;
}
