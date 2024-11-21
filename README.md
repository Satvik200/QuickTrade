
# QuickTrade

This project was created as an exercise to test and implement a variety of techniques in an ultra-low-latency environment.
## Goals

- Write human readable view of in-memory book every 10 messages

- Write machine-readable series of mid-quotes

- On each trade message, write total qty traded at most recent trade price

- Gracefully handle various garbage messages
## Development and Testing Environment

- Processor: Apple M2

- OS: macOS v15.1.1

- Kernel: v24.1.0

- Clang Version: Apple clang version 16.0.0

- Boost Version: v1.86.0

- Perl Version: v5.34.1



   
   
   
## Creating a sample Input File

To create a sample input for this project, execute the following command: 

```bash
  perl OrderGenerator.pl > orders.txt
```


## Running Tests

To run tests, execute the following commands:

```bash
  chmod +x runTest.sh
  ./runTest.sh
```


## Running Build

To run the build of this project, execute the following commands: 

```bash
  chmod +x runBuild.sh
  ./runBuild.sh
```

