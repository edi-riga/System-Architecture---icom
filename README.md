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
icom_t *icom = icom_init("socket_tx|zero|127.0.0.1:8889,8890,8891");

// Initialize tcp socket to any network interface and bind 8889,8890,8891 ports
// while using (pointer) zero-copy communication with timeout detection
icom_t *icom = icom_init("socket_rx|zero,timeout|*:[8889-8891]");
```

### Deinitialization


### Communication


## Repository
