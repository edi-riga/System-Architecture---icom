[[_TOC_]]

## Introduction
_**icom**_ _(Intercommunication)_ is one of EDI's system architecture tools for Linux-based system development and exploration. _icom_ provides routines for implementing simplified _Inter-Process Communications (IPC)_. However, note that many envisioned features are still not present in the library.

## Build


## Basic usage
The framework usage can be split into three parts:
- initialization
- deinitialization
- usage

### Initialization
```c
icom_t *icom = icom_init(const char* config);
```
Here `icom` is a framework's communication object, but `config` encodes communication's configuration and has the following format:
```c
"communicator|flgas|communicator_specific_configuration"
```

The following communicators are supported:
```c
"socket_tx"  // tcp (connect) socket
"socket_rx"  // tcp (bind) socket
```

The following flags are supported:
```c
"default"  // default deep-copy communication
"zero"     // zero-copy communication
"timeout"  // enable timeout detection
```

Communicator setup examples:
```c
// Initialize tcp socket connecting to (home) 127.0.0.1 IP address and 8889 port
// using (typical) deep-copy communication
icom_t *icom = icom_init("socket_tx|default|127.0.0.1:8889");

// Initialize tcp socket connecting to (home) 127.0.0.1 IP address and 8889,8890,8891
// ports using (pointer) zero-copy communication
icom_t *icom = icom_init("socket_tx|zero|127.0.0.1:[8889,8890,8891]");

// Initialize tcp socket to any network interface and bind 8889,8890,8891 ports
// while using (pointer) zero-copy communication with timeout detection
icom_t *icom = icom_init("socket_rx|zero,timeout|*:[8889-8891]");
```

### Deinitialization
```c
icom_deinit(icom);
```

### Communication

#### Sending
```c
// Send data to a previously initialized connection
// If a connection has multiple destinations, the same data gets sent to all of them
icom_send(icom, buf, bufSize);
```

#### Receiving
The `icom_recv` function accepts variable number of arguments depending on particular use case.
```c
void *buf;         // Buffer to store received data
unsigned bufSize;  // Variable that holds buffer size

// Receive data and buffer size
icom_recv(icom, buf, &bufSize);

// Receive data, don't care about the size
// Can be useful if the buffer size is fixed or included in the data stream itself
icom_recv(icom, buf);
```

Receiving from a connection with multiple sources:
```c
void *buf;
unsigned bufSize;

// A special pointer for buffer traversal
// Has to be a null pointer to get the first buffer
void *data = nullptr;

// Receive data from all of the sources
icom_recv(p->icomIn);

// Traverse the individual buffers
while (true) {
    buf = icom_nextBuffer(p->icomIn, &data, &bytes);

    if(data == NULL){
        // No more buffers
        break;
    }

    // Data processing goes here
}
```

#### Zero-copy and buffer overwrites
If zero-copy communication reuses the same buffer for all transactions, there may be situations where the sender could overwrite the buffer contents with new data before the receiver has finished processing them, leading to data corruption. Thus there must be some way for the receiver to notify the sender when it is safe to overwrite. Here this is done with the `notify` keyword.
```c
// This sender may expect a notification from the receiver
icom_t *icom_tx = icom_init("socket_tx|zero,notify|127.0.0.1:3210");

// This receiver may send a notification to the sender
icom_t *icom_rx = icom_init("socket_rx|zero,notify|127.0.0.1:3210");
```

The receiver then operates as follows:
```c
// Receive the data
icom_recv(icom_rx, buf, &bufSize);

// Process the data
...

//Notify the sender that the buffer can be overwritten
icom_notify_send(icom_rx);
```

The sending side:
```c
// Acquire the data
...

// Send the data
icom_recv(icom_tx, buf, &bufSize);

// Block until notified by the receiver
icom_notify_recv(icom_tx);
```

To allow configuring communication parameters without code changes, the `icom_notify_send` and `icom_notify_recv` won't do anything unless the respective interface was initialized with the `notify` flag.

The notification functions can be integrated into the send/receive functions by using the `autonotify` flag instead. This is most useful with sender interfaces, where using `autonotify` makes `icom_send` automatically call `icom_notify_recv` after sending the data, which is a common usage scenario. Note that on the receiving interface a similar configuration option would make `icom_receive` call `icom_notify_send` *before* attempting to receive.


## Repository
