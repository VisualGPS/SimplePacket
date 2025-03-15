/*
 * SimplePacket.h
 *
 *  Created on: Jan 23, 1998
 *      Author: Monte Variakojis
 */

#ifndef SOURCES_SIMPLEPACKET_H_
#define SOURCES_SIMPLEPACKET_H_
#include <stdint.h>
#include <cstdio>

///
/// Used to identify unused function parameters.
///
#ifndef UNUSED_PARAM
#define UNUSED_PARAM(x) (void)(x)
#endif


///
/// @class CSimplePacket
/// @brief This class will packetize or depacketize data and pass the results to the child
/// class.
///
/// The parser does not require whole packets to be received. One byte at a time or
/// multiple packets may be received.
///
class CSimplePacket {
public:

	///
	/// This state machine enumeration defines all of the parsing state for the protocol
	///
	enum RX_STATE_E {
		RX_STATE_SOM_1 = 0,														///< Start of message
		RX_STATE_SOM_2,															///< Start of message
		RX_STATE_CMD_H,															///< Command
		RX_STATE_CMD_L,															///< Command
		RX_STATE_LENGTH_H1,														///< Length high byte (24..31)
		RX_STATE_LENGTH_H2,														///< Length high byte (16..23)
		RX_STATE_LENGTH_H3,														///< Length high byte (8..15 )
		RX_STATE_LENGTH_L,														///< Length low byte  (0..7  )
		RX_STATE_DATA,															///< Data payload
		RX_STATE_CS_H,															///< Checksum high
		RX_STATE_CS_L,															///< Checksum low
	};																			

	enum ERROR_E {
		ERROR_OK = 0,															///< No error, operation successful
		ERROR_FAIL,																///< Error, Operation failed
		ERROR_CHECKSUM,															///< Error, Checksum mismatch
		ERROR_OVERRUN,															///< Error, Buffer overrun
		ERROR_SOM_1,															///< Error, Bad start of message 1
		ERROR_SOM_2,															///< Error, Bad start of message 2
		ERROR_LENGTH_TOO_LARGE,													///< Error, Length received is too large
	};

	static const uint8_t	c_u8SOM_1 = 0x55;									///< Start of message (define as needed)
	static const uint8_t	c_u8SOM_2 = 0xAA;									///< Start of message

private:
	RX_STATE_E		m_nRxState;													///< Current Rx state
	uint16_t		m_uRxCommand;												///< Command received
	size_t			m_nRxLength;												///< Length of receive data
	size_t			m_nRxIndex;													///< Current index into the receive data state
	size_t			m_nMaxRxBuffSize;											///< Maximum receive buffer size
	uint8_t	*		m_pRxBuffer;												///< Pointer to receive buffer
	uint16_t		m_uRxChecksum;												///< Actual checksum received
	uint16_t		m_uChecksum;												///< Checksum of RX_STATE_CMD_H through RX_STATE_DATA
	uint16_t		m_uRxSOM;													///< Received Start of message

public:
	///
	/// @brief This constructor will need a pointer to a storage buffer as well as its size.
	///
	/// @param pDataBuff Pointer to a data buffer to store receive data to
	/// @param nDataBuffSize Size of pDataBuff
	///
	CSimplePacket(uint8_t *pDataBuff, size_t nDataBuffSize);
	virtual ~CSimplePacket();

	///
	/// @brief Receive data processor. This method will build a command and
	/// dispatch the command if the checksum is good.
	///
	/// Note: You can feed data in small amounts or large.
	///
	/// @param pData Data buffer to parse
	/// @param nSize Number of bytes to process in pData
	///
	virtual void ProcessRxBuffer(uint8_t *pData, size_t nSize);

	///
	/// @brief Resets all state machines and data
	///
	void Reset(void);

	///
	/// @brief Returns true if the packet parser is waiting for SOM.
	/// \return true = Waiting for SOM, false = parsing
	///
	bool IsWaitingForSOM(void){ return m_nRxState == RX_STATE_SOM_1?true:false; }

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
    virtual void OnError(CSimplePacket::ERROR_E nError, uint16_t uCommand, uint8_t *pData, size_t nLength, uint16_t uRxSOM){
        UNUSED_PARAM(nError);
        UNUSED_PARAM(uCommand);
        UNUSED_PARAM(pData);
        UNUSED_PARAM(nLength);
        UNUSED_PARAM(uRxSOM);
    }

	///
	/// @brief Write a single data packet.
	///
	/// NOTE: You will need to redefine CSimplePacket::Write 
	/// @param uCommand Command to write
	/// @param pData Data associated with the command
	/// @param nLength Number of bytes in pData to write
	///
	void WritePacket(uint16_t uCommand, uint8_t *pData, size_t nLength);

private:

	///
	/// @brief You must redefine this method to write data to your serial media.
	///
	/// NOTE: The transport media can be whatever your project requires.
	///
	/// @param pData Pointer to data buffer to write
	/// @param nLength Number of bytes to write
	/// \return Number of bytes written
	///
	virtual int Write(uint8_t *pData, size_t nLength) = 0;

	///
	/// @brief Redefine this method to support a mutex lock
	///
	virtual void WriteLock(void) {}

	///
	/// @brief Redefine this method to unlock the mutex
	///
	virtual void WriteUnlock(void) {}

	///
	/// @brief THis is a CRC-16-CCITT (poly 0x1021) checksum method.
	///
	/// @param u16CRC Current checksum
	/// @param pData Buffer to calculate checksum on
	/// @param bReset When true the method will reset the CRC before running the calculation. When false, the method will continue to calculate the checksum.
	/// \return uint16_t Calculated CRC
	///
	uint16_t crc16(uint16_t u16CRC, uint8_t * pData, int nLen, bool bReset = false);
};

#endif /* SOURCES_SIMPLEPACKET_H_ */
