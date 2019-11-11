#pragma once
#pragma comment(lib, "Ws2_32.lib")

#include <io.h> 
#include <stdio.h> 
#include <winsock2.h> 
#include <stdlib.h> 
#include <winsock.h> 
#include <string.h> 
#include <Ws2tcpip.h>
#include "libpq-fe.h"


#define PORT 4444 
#define DEFAULT_PORT "4444"
#define DEFAULT_BUFLEN 512

void TCPconnection(char* dataStr);
void processDataStr(char* dataStr, char** dataArray);
void freeAllocations(char** dataArray, char* quary);
void creatQuary(char** dataArray, char** quary);
void CloseConn(PGconn* conn);
void InsertPatientRec(PGconn* conn, char* quary);
PGconn* ConnectDB();