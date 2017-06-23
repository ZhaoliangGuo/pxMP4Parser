// pxMP4Parser.cpp : 定义控制台应用程序的入口点。
/*
Function : A demo for analyze the mp4 video format.
Author   : Zhaoliang Guo
Date     : 20170622
*
*/

#include "stdafx.h"
#include "pxMP4CommonDef.h"

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

int GetBoxHeader(FILE         *in_fpMP4File, 
	             UINT64       in_ui64BoxStartPos, 
				 SPxBoxHeader *out_psBoxHeader)
{
	if (NULL == in_fpMP4File)
	{
		return -1;
	}

	if (NULL == out_psBoxHeader)
	{
		return -1;
	}

	UINT ui32DataSize = 0;
	UINT ui32BoxType  = 0;

	// Data Size
	fseek(in_fpMP4File, in_ui64BoxStartPos, SEEK_SET);
	fread(&ui32DataSize, 1, 4, in_fpMP4File);
	//in_ui64BoxStartPos += 4;

	// Box Type (Atom Type)
	fseek(in_fpMP4File, in_ui64BoxStartPos + 4, SEEK_SET);
	fread(&ui32BoxType, 1, 4, in_fpMP4File);
	//in_ui64BoxStartPos += 4;

	// 将网络字节序转换为本机字节序
	out_psBoxHeader->ui32DataSize = (UINT)ntohl(ui32DataSize);
	out_psBoxHeader->ui32BoxType  = (UINT)ntohl(ui32BoxType);

	return 0;
}

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

	char *pszMP4FileName = "RARBG.COM.mp4";

	FILE *fMP4File = fopen(pszMP4FileName, "r");
	if (NULL == fMP4File)
	{
		printf("open fail: %s", pszMP4FileName);

		return -1;
	}

	SPxBoxHeader   sPxBoxHeader;
	SPxBoxHeaderEx sPxBoxHeaderEx;
	
	UINT64 ui64BoxSize = 0;

	UINT64 ui64LargerSize = 0;

	UINT64 ui64CurPos = 0;

	UINT ui32BoxTpye  = 0;

	bool bLargeBox = false;

	while (true)
	{
		bLargeBox = false;

		memset(&sPxBoxHeader, 0, sizeof(sPxBoxHeader));
		//memset(&sPxBoxHeaderEx, 0, sizeof(SPxBoxHeaderEx));

		GetBoxHeader(fMP4File, ui64CurPos, &sPxBoxHeader);
		ui64CurPos += sizeof(SPxBoxHeader);
	
		if (1 == sPxBoxHeader.ui32DataSize) // Large Box
		{
			bLargeBox = true;

			fseek(fMP4File, ui64CurPos, SEEK_SET);
			fread((char *)&ui64LargerSize, 1, 8, fMP4File);
			ui64CurPos += 8;

		    // TODO:
			// S1: 字节序转换
			// S2: 赋值给ui64BoxSize

			/*UINT ui32MostSignificantWord  = ntohl(ui64LargerSize      & 0xFFFFFFFF);
			UINT ui32LeastSignificantWord = ntohl(ui64LargerSize >> 32 & 0xFFFFFFFF);*/

			//ui64BoxSize = sPxBoxHeaderEx.ui32DataSize;
		}
		else if (0 == sPxBoxHeader.ui32DataSize) // Last Box
		{

			break;
		}
		else
		{
			ui64BoxSize = sPxBoxHeader.ui32DataSize;
		}

		printf("\n - BoxType:%c%c%c%c; Box Size:%I64u; Offset:%I64u\n",    
			   (sPxBoxHeader.ui32BoxType >> 24 & 0xFF),
			   (sPxBoxHeader.ui32BoxType >> 16 & 0xFF),
			   (sPxBoxHeader.ui32BoxType >> 8  & 0xFF),
			   (sPxBoxHeader.ui32BoxType       & 0xFF),
			   ui64BoxSize,
			   bLargeBox ? ui64CurPos - 16 : ui64CurPos - 8);

		SPxBoxHeader sMoovBoxHeader;
		memset(&sMoovBoxHeader, 0, sizeof(SPxBoxHeader));

		// moov
		if (NS_MMG_ATOM_MOOV == sPxBoxHeader.ui32BoxType)
		{
			int nMoovPos = 0;

			while (nMoovPos < ui64BoxSize - 8)
			{
				GetBoxHeader(fMP4File, ui64CurPos + nMoovPos, &sMoovBoxHeader);

				printf("	-- BoxType:%c%c%c%c; Box Size:%u; Offset:%I64u\n",    
					(sMoovBoxHeader.ui32BoxType >> 24 & 0xFF),
					(sMoovBoxHeader.ui32BoxType >> 16 & 0xFF),
					(sMoovBoxHeader.ui32BoxType >> 8  & 0xFF),
					(sMoovBoxHeader.ui32BoxType       & 0xFF),
					sMoovBoxHeader.ui32DataSize,
					ui64CurPos + nMoovPos);

				nMoovPos += sMoovBoxHeader.ui32DataSize;
				//nMoovPos -= 8;
			}
		}

		if (bLargeBox)
		{
			ui64CurPos -= 16;
		}
		else
		{
			ui64CurPos -= 8;
		}

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

