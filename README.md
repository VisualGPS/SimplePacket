# SimplePacket

Monte Variakojis @VisualGPS

The SimplePacket project demonstrates a simple robust packet protocol for use with various media such as serial ports or even sockets. SimplePacket is intended to be very simple and has been used in many projects and platforms including Linux, Windows, and embedded projects. It's only two files, SimplePacket.cpp and SimplePacket.h.

## Packet Format

The packet is composed of a Start of Message (SOM), command, data length, data payload, and a checksum field. All protocol data is represented in Big Endian encoding. Although it's up to you on how the data payload is used. It's recommended that any formatted payload data follows Big Endian encoding.

### Packet

| SOM | COMMAND | DATA_LENGTH | DATA | CRC

#### Field Description

| Field       | Size          | Description                                             |
| ----------- | ------------- | ------------------------------------------------------- |
| SOM         | uint16_t (2)  | Start of message and is set to 0x55AA                   |
| COMMAND     | uint16_t (2)  | Command - 0 to 65535                                    |
| DATA_LENGTH | uint32_t (4)  | Data length, 0 to 2^32 - 1                              |
| DATA        | variable      | Payload data (omitted if DATA_LENGTH == 0)              |
| CRC         | uint16_t (2)  | CRC 16 of COMMAND, DATA_LENGTH, and DATA                |

## Software Design

The idea of this class is to define packet formatting allowing you to concentrate on the payload data. The class CSimplePacket is OS, platform independent, and must be used as a base class. 

The receive parser is driven by a state machine where the individual states are defined the the enumeration CSimplePacket::RX_STATE_E. You simply feed the CSimplePacket::ProcessRxBuffer() method data. Since it's a state machine, you can feed the method a single byte at a time or large buffers. When the parser completes parsing and verifies a packet, the state machine will call OnProcessCommad() method where you can process the payload data.

There are some mandatory virtual methods that the derived class will need to redefine. These methods are:

```cpp
    ///
    /// @brief You must redefine this method to write data to your serial media.
    ///
    /// NOTE: The transport media can be whatever your project requires.
    ///
    /// @param pData Pointer to data buffer to write
    /// @param nLength Number of bytes to write
    /// @return Number of bytes written
    ///
    virtual int Write(uint8_t *pData, size_t nLength) = 0;

    ///
    /// @brief This method is called when a successful command is processed
    ///
    /// Redefine this class in your child class to parse commands that you have defined.
    ///
    /// @param uCommand Two byte command
    /// @param pData Pointer to data buffer of received data for this command
    /// @param nLength Number of data bytes received
    ///
    virtual void OnProcessCommand(uint16_t uCommand, uint8_t *pData, size_t nLength) = 0;
```

There are some optional virtual methods that can be redefined to help with data synchronization and debugging. These methods are described below.

```cpp
    ///
    /// @brief This method is called when an error occurs. Please note that
    /// uCommand, pData and nLength are not guaranteed and provided for debugging reference
    /// only.
    ///
    /// NOTE: Redefine this method to capture any errors.
    ///
    /// @param nError ERROR_T error number being reported
    /// @param uCommand Two byte command
    /// @param pData Pointer to data buffer of received data for this command
    /// @param nLength Number of data bytes received
    /// @param uRxSOM Received SOM - This is what was received
    ///
    virtual void OnError(CSimplePacket::ERROR_E nError, uint16_t uCommand, uint8_t *pData, size_t nLength, uint16_t uRxSOM){}

    ///
    /// @brief Redefine this method to support a mutex lock
    ///
    virtual void WriteLock(void) {}

    ///
    /// @brief Redefine this method to unlock the mutex
    ///
    virtual void WriteUnlock(void) {}
```

## Requirements

The class library is two files, SimplePacket.cpp and SimplePacket.h and includes no third party libraries.

- C++ (C11)
- cmake - CMake is an open-source, cross-platform family of tools designed to build, test and package software. 
- doxygen (optional) Builds html documentation

## Build

This project is managed using cmake and can be built using your favorite operating system.

### Linux/Unix based

```bash
mkdir ./BuildSimplePacket
cd ./BuildSimplePacket
cmake ../Source
make
```

### Windows

```bash
mkdir ./BuildSimplePacket
cd ./BuildSimplePacket
cmake ../Source
Run VisualStudio and load the project
```

## Packet State Machine

```plantuml

(RX_STATE_SOM_1)
(RX_STATE_SOM_2)
(RX_STATE_CMD_H)
(RX_STATE_CMD_L)
(RX_STATE_LENGTH_H1)
(RX_STATE_LENGTH_H2)
(RX_STATE_LENGTH_H3)
(RX_STATE_LENGTH_L)
(RX_STATE_DATA)
(RX_STATE_CS_H)
(RX_STATE_CS_L)


RX_STATE_SOM_1 --> RX_STATE_SOM_1
RX_STATE_SOM_1 --> RX_STATE_SOM_2
RX_STATE_SOM_2 --> RX_STATE_SOM_1
RX_STATE_SOM_2 --> RX_STATE_CMD_H
RX_STATE_CMD_H --> RX_STATE_CMD_L
RX_STATE_CMD_L --> RX_STATE_LENGTH_H1
RX_STATE_LENGTH_H1 --> RX_STATE_LENGTH_H2
RX_STATE_LENGTH_H2 --> RX_STATE_LENGTH_H3
RX_STATE_LENGTH_H3 --> RX_STATE_LENGTH_L
RX_STATE_LENGTH_L --> RX_STATE_SOM_1
RX_STATE_LENGTH_L --> RX_STATE_DATA
RX_STATE_LENGTH_L --> RX_STATE_CS_H
RX_STATE_CS_H --> RX_STATE_CS_L
RX_STATE_CS_L --> RX_STATE_SOM_1
RX_STATE_DATA --> RX_STATE_CS_H
RX_STATE_DATA --> RX_STATE_SOM_1
RX_STATE_DATA --> RX_STATE_DATA
```
