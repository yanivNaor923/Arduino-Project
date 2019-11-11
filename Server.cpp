#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include "pacemakerServer1.h"
using namespace std;


int main() {

	PGconn* conn = nullptr;
	char* dataArray[7];
	char* quary = nullptr;
	char dataStr[1024] = "";
	//dataStr example "name=adar;id=315524868;age=23;serial_number=123;beat_per_minute=70;time=10:00;date=12/09/2019;" ;

	while (true) {
		TCPconnection(dataStr);
		printf("the message: %s\n", dataStr);
		processDataStr(dataStr, dataArray);
		creatQuary(dataArray, &quary);
		conn = ConnectDB();
		InsertPatientRec(conn, quary);
		freeAllocations(dataArray, quary);
	}
	return 0;
}

void TCPconnection(char* dataStr) {
	int server_fd, new_socket, valread;
	char opt = 1;
	struct sockaddr_in address = { 0 };
	int addrlen = sizeof(address);
	char buffer[1024] = { 0 };

	WORD wVersionRequested;
	WSADATA wsaData;
	int err;

	/* Use the MAKEWORD(lowbyte, highbyte) macro declared in Windef.h */
	wVersionRequested = MAKEWORD(2, 2);

	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0) {
		/* Tell the user that we could not find a usable */
		/* Winsock DLL.                                  */
		printf("WSAStartup failed with error: %d\n", err);
		return;
	}

	struct addrinfo* result = NULL, *ptr = NULL, hints;
	SOCKET ListenSocket = INVALID_SOCKET;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the local address and port to be used by the server
	int iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed: %d\n", iResult);
		WSACleanup();
		return;
	}

	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		printf("Error at socket(): %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return;
	}

	// Setup the TCP listening socket
	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return;
	}
	freeaddrinfo(result);

	if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR) {
		printf("Listen failed with error: %ld\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return;
	}

	SOCKET ClientSocket;

	ClientSocket = INVALID_SOCKET;

	// Accept a client socket
	ClientSocket = accept(ListenSocket, NULL, NULL);
	if (ClientSocket == INVALID_SOCKET) {
		printf("accept failed: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return;
	}

	char recvbuf[DEFAULT_BUFLEN] = { 0 };
	int iSendResult;
	int recvbuflen = DEFAULT_BUFLEN;

	// Receive until the peer shuts down the connection
	do {

		iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0) {
			printf("Bytes received: %d\n", iResult);
			printf("Data received:: %s\n", recvbuf);
			strcpy(dataStr, recvbuf);
		}
		else if (iResult == 0) {
			printf("Connection closing...\n");
			closesocket(ClientSocket);
			closesocket(ListenSocket);
		}
		else {
			printf("recv failed: %d\n", WSAGetLastError());
			closesocket(ClientSocket);
			closesocket(ListenSocket);
			WSACleanup();
			return;
		}

	} while (iResult > 0);

}

//char* name, char* id, char* age, char* serial_number, char* beat_per_minute, char* time, char* date

void processDataStr(char* dataStr, char** dataArray) {

	int i = 0;
	char* token;
	char* tempStr = (char*)malloc((strlen(dataStr) + 1) * sizeof(char));
	strcpy(tempStr, dataStr);
	char* toFree = tempStr;
	while (*dataStr) {

		while (*dataStr != '=' && *dataStr != 0) dataStr++;
		if (*dataStr == 0) break;
		dataStr++; //skip the '='
		strcpy(tempStr, dataStr);
		token = strtok(tempStr, ";");
		dataArray[i] = (char*)malloc((strlen(token) + 1) * sizeof(char));
		strcpy(dataArray[i++], token);
		strcpy(tempStr, dataStr);

	}
	free(toFree);
}

void freeAllocations(char** dataArray, char* quary) {
	for (int i = 0; i < 7; i++)
	{
		free(dataArray[i]);
	}
	free(quary);
}

void creatQuary(char** dataArray, char** quary) {

	int i = 0;
	string tempQuary;
	tempQuary.append("INSERT INTO pacemaker_patients VALUES ('");
	for (i = 0; i < 6; i++)
	{
		tempQuary.append(dataArray[i]);
		tempQuary.append("', '");
	}
	tempQuary.append(dataArray[i]);
	tempQuary.append("')");
	*quary = (char*)calloc((tempQuary.length() + 1), sizeof(char));
	strcpy(*quary, tempQuary.c_str());
}

/* Establish connection to database */
PGconn* ConnectDB()
{
	PGconn* conn = NULL;

	// Make a connection to the database
	conn = PQconnectdb("user=gisuser1 password=gisuser#1 dbname=gisuser1db hostaddr=212.179.205.15 port=5432");

	// Check to see that the backend connection was successfully made
	if (PQstatus(conn) != CONNECTION_OK)
	{
		printf("Connection to database failed");
		CloseConn(conn);
	}

	printf("Connection to database - OK\n");

	return conn;
}

/* Close connection to database */
void CloseConn(PGconn* conn)
{
	PQfinish(conn);
	exit(1);
}

/* Append SQL statement and insert record into employee table */
void InsertPatientRec(PGconn* conn, char* quary)
{
	// Execute with sql statement
	PGresult* res = PQexec(conn, quary);

	if (PQresultStatus(res) != PGRES_COMMAND_OK)
	{
		printf("Insert patient record failed");
		PQclear(res);
		CloseConn(conn);
	}

	printf("Insert patient record - OK\n");

	// Clear result
	PQclear(res);
}
