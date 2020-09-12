#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <WinSock2.h>
#include <Windows.h>
#pragma comment(lib, "WS2_32.lib")

#define PORT 53
#define BUFFER_SIZE 1024
const char* LOCAL_IP = "127.0.0.1";
const char* SERVER_IP = "192.168.124.1";

typedef struct DNSHEADER {
	USHORT ID;
	USHORT Flag;
	USHORT Qdcount;
	USHORT Ancount;
	USHORT Nscount;
	USHORT Arcount;
}DNSHEADER;

typedef struct DNSQUESTION {
	USHORT Qtype;
	USHORT Qclass;
}DNSQUESTION;

//#pragma pack(push, 1)
typedef struct DNSRESOURCE {
	USHORT Type;
	USHORT Class;
	UINT TTL;
	USHORT Length;
}DNSRESOURCE;
//#pragma pack(pop)

void DomainName(char* buf)
{
	char* p = buf;
	while (*p != 0) {
		if (*p >= 33)
			p++;
		else {
			*p = '.';
			p++;
		}
	}
	printf("[DomainName = %s]\n", buf);
}

INT LookUp(CHAR* domainName, CHAR* ip) {
	INT flag = 0;
	FILE* f;
	if ((f = fopen("dnsrelay.txt", "r")) == NULL) {
		printf("Can't open the file\n");
		return -1;
//		exit(1);
	}
	CHAR domainNameTemp[BUFFER_SIZE];
	CHAR ipTemp[BUFFER_SIZE];
	CHAR temp[BUFFER_SIZE];
	while (!feof(f)) {
		fgets(temp, BUFFER_SIZE, f);
		for (INT i = 0, fl = 0, iplength = 0; i < BUFFER_SIZE; i++) {
			if (fl == 0) {
				if (temp[i] != ' ') {
					ipTemp[i] = temp[i];
				}
				else {
					iplength = i;
					fl = 1;
					ipTemp[i] = '\0';
				}
			}
			else if(fl == 1){
				if (temp[i] != '\n') {
					domainNameTemp[i - strlen(ipTemp) - 1] = temp[i];
				}
				else {
					domainNameTemp[i - strlen(ipTemp) - 1] = '\0';
					break;
				}
			}
		}
		if (strcmp(domainName, domainNameTemp) == 0) {
			flag = 1;
			CHAR* t1 = ipTemp;
			CHAR* t2 = ipTemp;
			INT i = 0;
			while (*t1 != '\0') {
				if (*t1 == '.') {
					*t1 = '\0';
					ip[i] = (char)atoi(t2);
					i++;
					t2 = t1 + 1;
				}
				t1++;
			}
			ip[i] = (char)atoi(t2);
			return flag;
		}
	}
	return flag;
}

VOID Respond(CHAR* buf, CHAR* ip) {
	DNSHEADER* header = (DNSHEADER*)buf;
	DNSRESOURCE* resouce;
	if (ip[0] == (char)0 && ip[1] == (char)0 && ip[2] == (char)0 && ip[3] == (char)0) {
		printf("IPaddr is 0.0.0.0, the domainname is unsafe.\n");
		header->Flag = htons(0x8183);
	}
	else {
		header->Flag = htons(0x8180);
	}
	header->Ancount = htons(1);
	CHAR* dn = buf + sizeof(DNSHEADER);
	CHAR* name = dn + strlen(dn) + sizeof(DNSQUESTION) + 1;
	USHORT* nameTemp = (unsigned short*)name;
	*nameTemp = htons((unsigned)0xC00C);
	resouce = (DNSRESOURCE*)(name + 2);
	resouce->Type = htons(1);
	resouce->Class = htons(1);
	resouce->TTL = htons(0x0FFF);
	resouce->Length = htons(4);
	CHAR* data = (char*)resouce + 10;
	*data = *ip;
	*(data + 1) = *(ip + 1);
	*(data + 2) = *(ip + 2);
	*(data + 3) = *(ip + 3);
}

VOID dns() {
	CHAR buf[BUFFER_SIZE];

	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		printf("error");

		exit(1);
	}

	SOCKET socketFd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (socketFd == SOCKET_ERROR) {
		printf("Creat Socket Error\n");
		exit(1);
	}
	SOCKADDR_IN server;
	server.sin_family = AF_INET;
	server.sin_port = htons(PORT);
	server.sin_addr.S_un.S_addr = htons(INADDR_ANY);

	SOCKADDR_IN client;
	client.sin_family = AF_INET;
	client.sin_port = htons(PORT);
	client.sin_addr.S_un.S_addr = htons(INADDR_ANY);

	INT z = bind(socketFd, (struct sockaddr *)&server, sizeof(server));
	if (z != 0) {
		printf("bind error with ErroNum %d\n", WSAGetLastError());
		exit(1);
	}

	CHAR* temp = (char*)malloc(BUFFER_SIZE);
	CHAR* ip = (char*)malloc(4);
	UINT len = sizeof(client);
	while(1){
		printf("\n");
		memset(buf, 0, BUFFER_SIZE);
		z = recvfrom(socketFd, buf, BUFFER_SIZE, 0, (struct sockaddr*)&client, &len);
		if (z < 0) {
//			printf("recvfrom Client error\n");
			continue;
		}

		strcpy(temp, buf + sizeof(DNSHEADER) + 1);
		
		DomainName(temp);
		INT ifFind = LookUp(temp, ip);

		if (ifFind == 1) {
			printf("Find From Host\n[IP = %u.%u.%u.%u]\n", *ip & 0x000000ff, *(ip + 1) & 0x000000ff, *(ip + 2) & 0x000000ff, *(ip + 3) & 0x000000ff);
			Respond(buf, ip);
		}
		else if (ifFind == 0){
			
			INT DnsFd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
			if (DnsFd == SOCKET_ERROR) {
				printf("Creat Socket Error\n");
				exit(1);
			}
			SOCKADDR_IN Dns;
			Dns.sin_family = AF_INET;
			Dns.sin_port = htons(PORT);
			Dns.sin_addr.S_un.S_addr = inet_addr(SERVER_IP);
			
			sendto(DnsFd, buf, BUFFER_SIZE, 0, (struct sockaddr*)&Dns, sizeof(Dns));
			UINT i = sizeof(Dns);
			UINT j;
			j = recvfrom(DnsFd, buf, BUFFER_SIZE, 0, (struct sockaddr*)&Dns, &i);
			if (j < 0) {
				printf("recevfrom DnsServer error\n");
				exit(1);
			}
			CHAR* p = buf + 3;
			printf("%u", *p & 0x0F);
			ip = buf + j - 4;
			printf("Get From Server\n[IP = %u.%u.%u.%u]\n", *(ip) & 0x000000ff, *(ip + 1) & 0x000000ff, *(ip + 2) & 0x000000ff, *(ip + 3) & 0x000000ff);
			closesocket(DnsFd);
		}
		else if (ifFind == -1) {
			printf("lose\n");
			exit(1);
		}

		z = sendto(socketFd, buf, sizeof(buf), 0, (struct sockaddr*)&client, sizeof(client));
		if (z < 0) {
			printf("sendto Client error\n");
			exit(1);
		}
	}
}

int main() {
	dns();
	return 0;
}