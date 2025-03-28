/*
 * SimplePacket.cpp
 *
 *  Created on: Jan 23, 1998
 *      Author: Monte Variakojis
 */
#include "SimplePacket.h"
#include <stdio.h>
#include <memory.h>

CSimplePacket::CSimplePacket(uint8_t *pDataBuff, size_t nDataBuffSize) {

	m_nMaxRxBuffSize = nDataBuffSize;
	m_pRxBuffer = pDataBuff;

	Reset();
}

CSimplePacket::~CSimplePacket() {
	// TODO Auto-generated destructor stub
}

void CSimplePacket::SetRxBuffer(uint8_t *pDataBuff, size_t nDataBuffSize)
{
	m_nMaxRxBuffSize = nDataBuffSize;
	m_pRxBuffer = pDataBuff;
}

void CSimplePacket::ProcessRxBuffer(uint8_t *pData, size_t nSize) {

	if(m_pRxBuffer == NULL) {
		OnError(ERROR_RX_BUFFER_SET_NULL, m_uRxCommand, m_pRxBuffer, m_nRxLength, m_uRxSOM);
		return;
	}

	for(size_t i = 0; i < nSize; i++) {

		uint8_t uData = pData[i];

		switch(m_nRxState) {

		case RX_STATE_SOM_1 :
			//
			// Check for first start of message. If it passes, then look for SOM 2
			//
			m_uRxSOM = (uint16_t)uData << 8;
			if(uData == c_u8SOM_1) {
				m_nRxState = RX_STATE_SOM_2;
			}
			else {
				OnError(ERROR_SOM_1, m_uRxCommand, m_pRxBuffer, m_nRxLength, m_uRxSOM);
			}
			break;

		case RX_STATE_SOM_2 :
			//
			// Check for start of message 2. If it looks good, continue with packet parsing
			//
			m_uRxSOM |= (uint16_t)uData;
			if(uData == c_u8SOM_2) {
				Reset();
				m_nRxState = RX_STATE_CMD_H;
			}
			else {
				// SOM did not pass, go back and look for SOM 1
				m_nRxState = RX_STATE_SOM_1;
				OnError(ERROR_SOM_2, m_uRxCommand, m_pRxBuffer, m_nRxLength, m_uRxSOM);
			}
			break;

		case RX_STATE_CMD_H :
			m_uRxCommand = (uint16_t)( (uint8_t)uData ) << 8;
			m_uChecksum = crc16(m_uChecksum, &uData, 1, false);
			m_nRxState = RX_STATE_CMD_L;
			break;

		case RX_STATE_CMD_L :
			m_uRxCommand |= (uint16_t)( (uint8_t)uData ) << 0;
			m_uChecksum = crc16(m_uChecksum, &uData, 1, false);
			m_nRxState = RX_STATE_LENGTH_H1;
			break;

		case RX_STATE_LENGTH_H1:
			m_nRxLength = 0;
			m_nRxLength |= (size_t)((uint8_t)uData) << 24;
			m_uChecksum = crc16(m_uChecksum, &uData, 1, false);
			m_nRxState = RX_STATE_LENGTH_H2;
			break;

		case RX_STATE_LENGTH_H2:
			m_nRxLength |= (size_t)((uint8_t)uData) << 16;
			m_uChecksum = crc16(m_uChecksum, &uData, 1, false);
			m_nRxState = RX_STATE_LENGTH_H3;
			break;

		case RX_STATE_LENGTH_H3 :
			m_nRxLength |= (size_t)((uint8_t)uData) << 8;
			m_uChecksum = crc16(m_uChecksum, &uData, 1, false);
			m_nRxState = RX_STATE_LENGTH_L;
			break;

		case RX_STATE_LENGTH_L :
			m_nRxLength |= (size_t)( (uint8_t)uData ) << 0;
			m_uChecksum = crc16(m_uChecksum, &uData, 1, false);
			m_nRxIndex = 0;

			// Check if the packet data length is too large
			if(m_nRxLength > m_nMaxRxBuffSize) {
				OnError(ERROR_LENGTH_TOO_LARGE, m_uRxCommand, m_pRxBuffer, m_nRxLength, m_uRxSOM);
				m_nRxState = RX_STATE_SOM_1;
			}

			// If we don't have data, then just go check the checksum
			else if(m_nRxLength == 0) {
				m_nRxState = RX_STATE_CS_H;
			}
			else {
				// We can receive data byte by byte or as a block. Here we'll determine that
				// based on the length of the data. For now we'll check for exact fit
				size_t nBlockSize = nSize - i - m_nRxLength + 1;
				// Check if we have enough data to fill the data buffer.
				if(nBlockSize == m_nRxLength) {
					memcpy(m_pRxBuffer, &pData[i+1], m_nRxLength);
					m_uChecksum = crc16(m_uChecksum, &pData[i+1], m_nRxLength, false);
					m_nRxIndex += m_nRxLength;
					i += m_nRxLength;
					m_nRxState = RX_STATE_CS_H;
				}
				else {
					m_nRxState = RX_STATE_DATA;
				}
			}
			break;

		case RX_STATE_DATA :
			if(m_nRxIndex >= m_nMaxRxBuffSize) {
				OnError(ERROR_OVERRUN, m_uRxCommand, m_pRxBuffer, m_nRxLength, m_uRxSOM);
				m_nRxState = RX_STATE_SOM_1;
			}
			else {
				m_pRxBuffer[m_nRxIndex++] = uData;
				m_uChecksum = crc16(m_uChecksum, &uData, 1, false);
				if(m_nRxIndex >= m_nRxLength) {
					m_nRxState = RX_STATE_CS_H;
				}
			}
			break;

		case RX_STATE_CS_H :
			m_uRxChecksum = (uint16_t)( (uint8_t)uData ) << 8;
			m_nRxState = RX_STATE_CS_L;
			break;

		case RX_STATE_CS_L :
			m_uRxChecksum |= (uint16_t)( (uint8_t)uData ) << 0;

			if(m_uRxChecksum == m_uChecksum) {
				OnProcessCommand(m_uRxCommand, m_pRxBuffer, m_nRxLength);
			}
			else {
				OnError(ERROR_CHECKSUM, m_uRxCommand, m_pRxBuffer, m_nRxLength, m_uRxSOM);
			}

			Reset();

			break;

		default :
			Reset();
		}
	}
}

void CSimplePacket::Reset(void){
	m_nRxState = RX_STATE_SOM_1;
	m_uRxCommand = 0xFFFF;
	m_uChecksum = crc16(m_uChecksum, NULL, 0, true);
}

void CSimplePacket::WritePacket(uint16_t uCommand, uint8_t * pData, size_t nLength) {
	uint8_t pBuff[64];
	uint16_t crc = 0;
	int i = 0;

	//
	// Build header (SOM CMD DATALEN)
	//
	pBuff[i++] = c_u8SOM_1;
	pBuff[i++] = c_u8SOM_2;

	pBuff[i++] = (uint8_t)((uint16_t)uCommand >> 8);
	pBuff[i++] = (uint8_t)((uint16_t)uCommand >> 0);
	crc = crc16(crc, &pBuff[i - 2], 2, true);

	pBuff[i++] = (uint8_t)((size_t)nLength >> 24);
	pBuff[i++] = (uint8_t)((size_t)nLength >> 16);
	pBuff[i++] = (uint8_t)((size_t)nLength >> 8);
	pBuff[i++] = (uint8_t)((size_t)nLength >> 0);
	crc = crc16(crc, &pBuff[i - 4], 4, false); // reset crc and calculate

	Write(pBuff, i);

	//
	// Write data
	//
	crc = crc16(crc, pData, nLength, false);
	if (nLength > 0) {
		Write(pData, nLength);
	}

	//
	// Write checksum
	//
	i = 0;
	pBuff[i++] = (uint8_t)((int)crc >> 8);
	pBuff[i++] = (uint8_t)((int)crc >> 0);
	Write(pBuff, i);
}

uint16_t CSimplePacket::crc16(uint16_t u16CRC, uint8_t * pData, int nLen, bool bReset){
    uint8_t x;

    if(bReset == true) {
    	u16CRC = 0xFFFF;
    }

    for(int i = 0; i < nLen; i++) {

        x = u16CRC >> 8 ^ pData[i];
        x ^= x >> 4;
        u16CRC = (u16CRC << 8) ^ ((unsigned short)(x << 12)) ^ ((unsigned short)(x <<5)) ^ ((unsigned short)x);
    }

    return u16CRC;
}
