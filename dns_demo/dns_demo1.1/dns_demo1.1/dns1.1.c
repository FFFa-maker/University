#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <WinSock2.h>
#include <Windows.h>
#include <sys/timeb.h>
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

VOID DomainName(CHAR* buf)
{
	CHAR* p = buf;
	printf("[DomainName = ");
	while (*p != 0) {
		if (*p >= 33)
			printf("%c", *p);
		else {
			printf(".");
		}
		p++;
	}
	printf("]\n");
}

INT LookUp(CHAR* domainName, CHAR* ip) {
	INT flag = 0;
	FILE* f;
//	if ((f = fopen("dnsrelay.txt", "r")) == NULL) {
	if ((f = fopen("C:\\Users\\1\\Desktop\\dnsrelay.txt", "r")) == NULL){
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
			else if (fl == 1) {
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
					ip[i] = (CHAR)atoi(t2);
					i++;
					t2 = t1 + 1;
				}
				t1++;
			}
			ip[i] = (CHAR)atoi(t2);
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

VOID PrintAnswer(const CHAR* buf) {
	UINT ipAnswer = 0;
	CHAR* ip = (CHAR*)malloc(4);
	CHAR* p = buf + 6;
	USHORT n = *(USHORT*)p;
	n = ntohs(n);
	p = buf + 12;
	printf("********************************************************************\n");
	printf("[ID = 0x%x]\n", ntohs((*((USHORT*)buf)) & 0xFFFF));
	DomainName(p + 1);
	printf("GET FROM SERVER\n");
	p = p + strlen(p) + 1 + 4;
	for (USHORT i = 0; i < n; i++) {
		p = p + 2;
		USHORT type = ntohs(*(USHORT*)p);
		p = p + 8;
		USHORT length = ntohs(*(USHORT*)p);
		if (type == 1){
			ip = p + 2;
			if (ip[0] == (CHAR)204 && ip[1] == (CHAR)204 && ip[2] == (CHAR)204 && ip[3] == (CHAR)204) {
				printf("TimeOut\n");
				continue;
			}
			printf("[IP[%u] = %u.%u.%u.%u]\n", ipAnswer,*(ip) & 0xff, *(ip + 1) & 0xff, *(ip + 2) & 0xff, *(ip + 3) & 0xff);
			ipAnswer++;
		}
		p = p + 2 + length;
	}
	if (ipAnswer == 0) {
		printf("No IPV4 answer\n");
	}
	printf("********************************************************************\n");
}

VOID dns() {
	CHAR buf[BUFFER_SIZE];

	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		printf("error\n");
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
	server.sin_addr.S_un.S_addr = inet_addr(LOCAL_IP);

	SOCKADDR_IN client;
	client.sin_family = AF_INET;
	client.sin_port = htons(PORT);
	client.sin_addr.S_un.S_addr = inet_addr(LOCAL_IP);

	INT z = bind(socketFd, (struct sockaddr*)&server, sizeof(server));
	if (z != 0) {
		printf("bind error\n");
		exit(1);
	}

	SOCKET DnsFd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	INT timeout = 2000;
//	setsockopt(DnsFd, SOL_SOCKET, SO_SNDTIMEO, (CHAR*)&timeout, sizeof(INT));
	setsockopt(DnsFd, SOL_SOCKET, SO_RCVTIMEO, (CHAR*)&timeout, sizeof(INT));
	if (DnsFd == SOCKET_ERROR) {
		printf("Creat Socket Error\n");
		exit(1);
	}
	SOCKADDR_IN Dns;
	Dns.sin_family = AF_INET;
	Dns.sin_port = htons(PORT);
	Dns.sin_addr.S_un.S_addr = inet_addr(SERVER_IP);

	CHAR* temp = (CHAR*)malloc(BUFFER_SIZE);
	CHAR* ip = (CHAR*)malloc(4);
	UINT len = sizeof(client);
	while (1) {
		memset(buf, 0, BUFFER_SIZE);
		z = recvfrom(socketFd, buf, BUFFER_SIZE, 0, (struct sockaddr*)&client, &len);
		if (z < 0) {
//			printf("recvfrom Client error\n");
			continue;
		}
		strcpy(temp, buf + sizeof(DNSHEADER) + 1);

		INT ifFind = LookUp(temp, ip);

		if (ifFind == 1) {
			printf("********************************************************************\n");
			printf("[ID = 0x%x]\n", ntohs((*((USHORT*)buf)) & 0xFFFF));
			DomainName(temp);
			printf("Find From Host\n[IP = %u.%u.%u.%u]\n", *ip & 0xff, *(ip + 1) & 0xff, *(ip + 2) & 0xff, *(ip + 3) & 0xff);
			Respond(buf, ip);
			printf("********************************************************************\n");
		}
		else if (ifFind == 0) {
			sendto(DnsFd, buf, BUFFER_SIZE, 0, (struct sockaddr*)&Dns, sizeof(Dns));
			UINT i = sizeof(Dns);
			UINT j;
			j = recvfrom(DnsFd, buf, BUFFER_SIZE, 0, (struct sockaddr*)&Dns, &i);

			if (j < 0) {
				printf("recevfrom DnsServer error\n");
				exit(1);
			}
			PrintAnswer(buf);
/*			CHAR* type = buf + 12;
			type = type + strlen(type) + 1 + 4 + 2;
			USHORT typeTemp = ntohs(*(USHORT*)type);
			if (typeTemp == 1) {
				DomainName(temp);
				printf("Get From Server\n");
				ip = buf + sizeof(CHAR) * (j - 4);
				if (ip[0] == (CHAR)204 && ip[1] == (CHAR)204 && ip[2] == (CHAR)204 && ip[3] == (CHAR)204) {
					printf("TimeOut\n");
					continue;
				}
				printf("[ID = 0x%x]\n", ntohs((*((USHORT*)buf)) & 0xFFFF));
				printf("[RCODE = %u]\n", *(buf + 3) & 0xf);
				printf("[IP = %u.%u.%u.%u]\n", *(ip) & 0xff, *(ip + 1) & 0xff, *(ip + 2) & 0xff, *(ip + 3) & 0xff);
			}
*/
		}
		else if (ifFind == -1) {
			continue;
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