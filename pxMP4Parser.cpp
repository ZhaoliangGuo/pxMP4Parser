// pxMP4Parser.cpp : 定义控制台应用程序的入口点。
/*
Function : A demo for analyze the mp4 video format.
Author   : Zhaoliang Guo
Date     : 20170622
*
*/

#include "stdafx.h"

// 为了使用字节序转换函数
#include <winsock2.h>
#pragma comment(lib, "Ws2_32.lib")

typedef unsigned __int64 UINT64;
typedef unsigned __int32 UINT;

struct SPxBoxHeader
{
	// 整个Box的大小 包括Box的Header
	// 大端存储
	UINT   ui32DataSize;

	// 大端存储
	UINT   ui32BoxType;  
};

struct SPxBoxHeaderEx
{
	UINT   ui32DataSize; // 大端存储
	UINT   ui32BoxType;  // 大端存储
	UINT64 ui64LargeSize;
};

void TestPlayground();

int _tmain(int argc, _TCHAR* argv[])
{
	WSADATA wsaData;
	WORD    wVersionRequested;

	wVersionRequested = MAKEWORD(2, 2);  
	int ret = WSAStartup(wVersionRequested, &wsaData);  //加载套接字库  
	if(ret!=0)  
	{  
		printf("WSAStartup() failed!\n");  

		return -1;  
	}  

	// 确认WinSock DLL支持版本2.2  
	if(LOBYTE(wsaData.wVersion)!=2 || HIBYTE(wsaData.wVersion)!=2)  
	{  
		WSACleanup();  
		printf("Invalid WinSock version!\n");

		return -1;  
	}  

	FILE *fMP4File = fopen("RARBG.COM.mp4", "r");

	SPxBoxHeader sPxBoxHeader;
	
	UINT64 ui64BoxSize = 0;

	UINT64 ui64LargerSize = 0;

	UINT64 ui64CurPos = 0;

	UINT ui32BoxTpye  = 0;

	bool bLargeBox = false;

	while (true)
	{
		bLargeBox = false;

		fseek(fMP4File, ui64CurPos, SEEK_SET);

		memset(&sPxBoxHeader, 0, sizeof(SPxBoxHeader));
		fread((char *)&sPxBoxHeader, 1, sizeof(SPxBoxHeader), fMP4File);
		ui64CurPos += sizeof(SPxBoxHeader);

		// 将网络字节序转换为本机字节序
		unsigned long ulDataSize = ntohl(sPxBoxHeader.ui32DataSize);
		unsigned long ulBoxType  = ntohl(sPxBoxHeader.ui32BoxType);

		if (1 == sPxBoxHeader.ui32BoxType)
		{
			bLargeBox = true;

			fseek(fMP4File, ui64CurPos, SEEK_SET);
			fread((char *)&ui64LargerSize, 1, 8, fMP4File);
			ui64CurPos += 8;

			/*UINT ui32MostSignificantWord  = ntohl(ui64LargerSize      & 0xFFFFFFFF);
			UINT ui32LeastSignificantWord = ntohl(ui64LargerSize >> 32 & 0xFFFFFFFF);*/
		}
		else if (0 == sPxBoxHeader.ui32BoxType) // Last Box
		{

			break;
		}
		else
		{
			ui64BoxSize = ulDataSize;
		}

		if (bLargeBox)
		{
			ui64CurPos -= 16;
		}
		else
		{
			ui64CurPos -= 8;
		}

		printf("BoxType:%c%c%c%c; Box Size:%I64u; Offset:%I64u\n",    
			   (sPxBoxHeader.ui32BoxType       & 0xFF),
			   (sPxBoxHeader.ui32BoxType >>  8 & 0xFF),
			   (sPxBoxHeader.ui32BoxType >> 16 & 0xFF),
			   (sPxBoxHeader.ui32BoxType >> 24 & 0xFF),
			   ui64BoxSize,
			   ui64CurPos);

		ui64CurPos += ui64BoxSize; // Box Body
	}

	if (fMP4File)
	{
		fclose(fMP4File);
		fMP4File = NULL;
	}

	WSACleanup();

	return 0;
}

// test and check 
void TestPlayground()
{

}

