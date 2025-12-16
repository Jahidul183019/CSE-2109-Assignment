# Implementation of CRC-16 Error Detection Using Socket Programming with Manchester Encoding
##  Project Question
Implement CRC-16 based error detection in a sender–receiver communication system using TCP sockets in C/C++. The system should use Manchester encoding and be able to detect the following errors introduced at the bit level:

Single-bit error detection

Two isolated single-bit errors

Odd number of errors

Burst errors of different sizes (8, 17, 22 bits)

Note: Only C/C++ implementations are allowed. Use of Linux environment is recommended for socket programming.

## Objective

The objective of this project is to:

Implement CRC-16 (Polynomial 0x1021) based error detection.

Simulate sender-receiver communication using TCP sockets in C++.

Apply Manchester encoding for bit-level transmission.

Detect different types of bit-level transmission errors.

## Tools & Environment

Language: C/C++

Platform: Linux/MacOS

Communication Protocol: TCP Socket Programming

Error Detection: CRC-16 (Polynomial 0x1021)

Encoding Scheme: Manchester Encoding

## System Overview

The system has two programs:

### 1. Sender

Converts data into binary bits

Computes CRC-16 on data bits

Appends CRC bits to form a frame

Introduces bit-level errors (configurable)

Applies Manchester encoding

Sends encoded bits to the receiver via TCP socket

### 2. Receiver

Receives encoded bitstream

Performs Manchester decoding

Separates data bits and CRC bits

Recomputes CRC-16 on received data

Compares CRC values to detect errors

## Manchester Encoding

| Data Bit | Manchester Code |
|----------|----------------|
| 0        | 10             |
| 1        | 01             |


Polynomial: 0x1021

Initial CRC Value: 0x0000

Calculated bit-by-bit on data bits

CRC bits are appended to the frame before transmission

## Error Injection Test Cases

| Test Case              | Description                 |
|------------------------|----------------------------|
| No Error               | Clean transmission         |
| Single Bit Error       | One bit flipped            |
| Two Isolated Errors    | Two separate bit flips     |
| Odd Number of Errors   | Three flipped bits         |
| Burst Error (8 bits)   | 8 consecutive bit flips    |
| Burst Error (17 bits)  | 17 consecutive bit flips   |
| Burst Error (22 bits)  | 22 consecutive bit flips   |

> Errors are injected **before Manchester encoding**, ensuring valid encoded symbols.

## Result

The receiver successfully detects all injected errors using CRC-16.

No errors → CRC verification passes

Errors → CRC mismatch indicates transmission error

## Conclusion

CRC-16 combined with Manchester encoding and socket communication provides reliable error detection.

The system can detect single-bit, multi-bit, odd-bit, and burst errors, demonstrating the effectiveness of CRC-16 in digital data transmission.

## 🖥 Compilation & Execution Instructions (Linux/MacOS)

1. **Navigate** to the project directory:
```bash
cd /path/to/project
```

2.Compile sender and receiver using g++:
```bash
g++ sender.cpp -o sender
g++ receiver.cpp -o receiver
```

3.Run the receiver first (so it listens on a port):
```bash
./receiver
```

4.Run the sender to send data:
```bash
./sender
```

> The sender will compute CRC-16, encode data with Manchester encoding, optionally inject errors, and send it to the receiver.
The receiver decodes, recomputes CRC, and reports error detection results.
