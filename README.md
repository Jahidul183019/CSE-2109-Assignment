# CRC-16 Error Detection with TCP Sockets and Manchester Encoding

## 1. Project Question
Implement CRC-16 based error detection in a sender–receiver communication system using TCP sockets in C/C++.  
The system uses Manchester encoding and detects the following bit-level errors:

- Single-bit error
- Two isolated single-bit errors
- Odd number of errors
- Burst errors of length 8, 17, 22

> **Note:** Only C/C++ implementations are allowed.

---

## 2. Objectives
- Implement **CRC-16 (Polynomial: 0x1021, Init: 0x0000)**
- Build a **TCP-based sender and receiver**
- Encode the transmitted frame using **Manchester encoding**
- Introduce configurable **bit-level errors** from the sender side
- Detect errors at the receiver using CRC verification

---

## 3. System Overview

### 3.1 Sender
1. Takes **data bits** as input (0/1 string)
2. Computes **CRC-16** over the data bits
3. Builds a **frame**: `frame = data_bits + crc_bits(16)`
4. Injects selected **bit errors**
5. Applies **Manchester encoding**
6. Sends encoded bitstream to the receiver via **TCP socket**

### 3.2 Receiver
1. Receives encoded bitstream via **TCP**
2. Converts received payload into a bit vector
3. Performs **Manchester decoding**
4. Splits decoded frame into: `data_bits` and `received_crc`
5. Recomputes CRC-16 on received data
6. Compares CRC values and outputs:
   - ✅ No error detected
   - ❌ Error detected

---

## 4. Manchester Encoding
Manchester encoding rule used:

| Data Bit | Manchester Code |
|---------:|------------------|
| 0        | 10               |
| 1        | 01               |

---

## 5. CRC-16 Details
- **Polynomial:** `0x1021`
- **Initial CRC:** `0x0000`
- CRC computed **bit-by-bit**
- CRC (16 bits) appended to the frame before transmission

---

## 6. Error Injection Test Cases
Errors are applied at the sender side (bit-level):

| Choice | Test Case                | Description |
|------:|--------------------------|------------|
| 0     | No Error                 | Clean transmission |
| 1     | Single-bit error         | 1 bit flipped |
| 2     | Two isolated errors      | 2 bits flipped at different positions |
| 3     | Odd number of errors     | User input: 3/5/7/... bits flipped |
| 4     | Burst error (8)          | 8 consecutive bits flipped |
| 5     | Burst error (17)         | 17 consecutive bits flipped |
| 6     | Burst error (22)         | 22 consecutive bits flipped |

> **Note:** Errors are injected **before Manchester encoding** so the encoded signal remains valid.

---

## 7. Expected Output
Receiver prints CRC comparison result:

- If `received_crc == computed_crc` → ✅ No error detected  
- Else → ❌ Error detected

Manchester decoding failure (invalid pair) is also treated as an error.

---

## 8. Compilation & Execution

### 8.1 Compile
```bash
g++ sender.cpp -o sender
g++ receiver.cpp -o receiver
```

### 8.2 Run (Port: 9000)
1.Run the receiver first:
```bash
./receiver
```

2.Run the sender:
```bash
./sender
```

## 9. Files

sender.cpp → Sender implementation

receiver.cpp → Receiver implementation

README.md → Project documentation
