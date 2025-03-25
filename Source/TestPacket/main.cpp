#include <stdio.h>
#include <string.h>
#include <SimplePacket.h>

void PrintDump(uint8_t * pData, int nLen, int nWidth);

enum CMD_ID {
	CMD_ID_TEST_STRING = 0,
	CMD_ID_TEST2_UINT32
};

// This class demonstrates how to use CSimplePacket as a base class to process packets.
class CCommDevice : public CSimplePacket {

private:
	static const int	c_nBuffSize = 1024;
	uint8_t				m_pRxBuffer[c_nBuffSize];

public:
	CCommDevice()
	: CSimplePacket(m_pRxBuffer, c_nBuffSize) 
	{
		// Redundant -- only for test
		CSimplePacket::SetRxBuffer(m_pRxBuffer, c_nBuffSize);
	}

	virtual void OnProcessCommand(uint16_t uCommand, uint8_t *pData, size_t nLength) {
		uint32_t u32Val;
		printf("Command #%d received, data = \n", uCommand);
		PrintDump(pData, nLength, 16);

		switch (uCommand) {
		case CMD_ID_TEST_STRING :
			break;

		case CMD_ID_TEST2_UINT32 :
			u32Val = 0;
			u32Val |= (uint32_t)((uint8_t)pData[0]) << 24;
			u32Val |= (uint32_t)((uint8_t)pData[1]) << 16;
			u32Val |= (uint32_t)((uint8_t)pData[2]) << 8;
			u32Val |= (uint32_t)((uint8_t)pData[3]) << 0;
			printf("CMD_ID_TEST2_UINT32 received, value = 0x%08X\n", u32Val);

			break;
		}
	}

	// For demonstration purposes, we are only printing the buffer. In a real-world
	// application, you would send this data to a socket or serial port.
	virtual int Write(uint8_t *pData, size_t nLength) {
		// Here we are simply printing the data. What you would do, is write to your transport media. For example, a serial port.
		PrintDump(pData, nLength, 16);
		return nLength;
	}
};

int main(int argc, char *argv[], char *envp[]) {

	CCommDevice commDevice;

	// Send a string of characters using command: CMD_ID_TEST_STRING
	printf("Sending CMD_ID_TEST_STRING command\n");
	const char lpszTestData[] = "Four score and seven years ago our fathers brought forth on this continent, a new nation, conceived in Liberty, and dedicated to the proposition that all men are created equal.";
	commDevice.WritePacket(CMD_ID_TEST_STRING, (uint8_t *)lpszTestData, sizeof(lpszTestData));

	// Example of sending an unsigned 32 integer
	printf("Sending CMD_ID_TEST2_UINT32 command\n");
	uint8_t pTestBuff[16];
	int i = 0;
	uint32_t u32Val = 0xBEEFCAFE;
	pTestBuff[i++] = (uint8_t)(u32Val >> 24);
	pTestBuff[i++] = (uint8_t)(u32Val >> 16);
	pTestBuff[i++] = (uint8_t)(u32Val >> 8);
	pTestBuff[i++] = (uint8_t)(u32Val >> 0);
	commDevice.WritePacket(CMD_ID_TEST2_UINT32, pTestBuff, i);

	// Here is an example of receiving data. pRxData is a simulated buffer that has received some data
	printf("Receiving some data and processing it\n");
	uint8_t pRxData[] = { 0x55, 0xAA, 0x00, 0x01, 0x00, 0x00, 0x00, 0x04, 0xBE, 0xEF, 0xCA, 0xFE, 0xAB, 0xD9 };
	commDevice.ProcessRxBuffer(pRxData, sizeof(pRxData));

	return 0;
}

///
/// \brief Print a buffer
/// \param pData pointer to buffer to print
/// \param nLen number of byts in pData to print
/// \param nWidth Number of bytes for each row
///
void PrintDump(uint8_t * pData, int nLen, int nWidth) {

	static char szStrHex[16384];
	static char szStrASCII[16384];
	szStrHex[0] = '\0';
	memset(szStrASCII, ' ', nWidth);
	szStrASCII[nWidth] = '\0';
	int nASCIIIndex = 0;
	char szAddr[16];
	for (int i = 0; i < nLen; i++) {

		if ((i != 0) && ((i % nWidth) == 0)) {
			szStrASCII[nWidth] = 0;
			printf("%s:  %s    %s\n", szAddr, szStrHex, szStrASCII);

			memset(szStrASCII, ' ', nWidth);
			szStrASCII[nWidth] = '\0';
			nASCIIIndex = 0;
			szStrHex[0] = '\0';
		}

		// Show address
		if ((i % nWidth) == 0) {
			sprintf(szAddr, "%04X", i);
		}

		char szStr2[32];
		char cVal = pData[i];
		sprintf(szStr2, "%02X ", (unsigned int)cVal & 0xFF);
		strncat(szStrHex, szStr2, 32);
		if (cVal >= ' ' && cVal <= '~' && cVal != '%') {
			szStrASCII[nASCIIIndex++] = cVal;
			if (cVal == '%') {
				szStrASCII[nASCIIIndex++] = cVal;
			}
		}
		else {
			szStrASCII[nASCIIIndex++] = '.';
		}
	}
	szStrASCII[nWidth] = 0;

	//
	// Fill in the extra spaces
	//
	int nStrHexLen = strlen(szStrHex);
	for (int i = nStrHexLen; i < nWidth * 3; i++) {
		szStrHex[i] = ' ';
	}
	szStrHex[nWidth * 3] = 0;

	printf("%s:  %s    %s\n", szAddr, szStrHex, szStrASCII);

}
