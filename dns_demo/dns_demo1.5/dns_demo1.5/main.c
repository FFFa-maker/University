#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <WinSock2.h>
#include <Windows.h>
#include <time.h>
#pragma comment(lib, "WS2_32.lib")

#define PORT 53
#define BUFFER_SIZE 1024
#define LRUSIZE 10
const char* LOCAL_IP = "127.0.0.1";
const char* SERVER_IP = "10.3.9.4";
const char* LOCAL_FILE = "dnsrelay.txt";

typedef struct DNSHEADER {
	unsigned short ID;
	unsigned short Flag;
	unsigned short Qdcount;
	unsigned short Ancount;
	unsigned short Nscount;
	unsigned short Arcount;
}DNSHEADER;			//dnsͷ

typedef struct DNSQUESTION {
	unsigned short Qtype;
	unsigned short Qclass;
}DNSQUESTION;		//dns����

typedef struct DNSRESOURCE {
	unsigned short Type;
	unsigned short Class;
	unsigned int TTL;
	unsigned short Length;
}DNSRESOURCE;		//dns�𰸣����������ṹ���õ��Ĳ��࣬д��������鿴�ṹ��

typedef struct Node {
	char IP[20];
	char DN[80];
	struct Node* next;
}Node;					//�������ļ���IP��DomainName

Node* HEAD;						//ȫ�ֱ������ļ���Ϣ������ͷ

typedef struct NNode {
	char IP[20];
	char DN[80];
	struct NNode* pre;
	struct NNode* next;
}NNode;

typedef struct LRUList {
	int size;
	NNode* head;
	NNode* tail;
}LRUList;

LRUList LIST;		//�洢��˫����������ִ��LRU����

/*
��ȫ�ֱ���list��ʼ��,����˫�������ͷ��β�Լ�size
*/
void initList() {
	LIST.size = 0;
	LIST.head = (NNode*)malloc(sizeof(NNode));
	LIST.tail = (NNode*)malloc(sizeof(NNode));
	LIST.head->pre = NULL;
	LIST.head->next = LIST.tail;
	LIST.tail->pre = LIST.head;
	LIST.tail->next = NULL;
}

/*
*���ڵ�p���뵽head�ĺ���
*/
void putFirst(NNode* p) {
	p->next = LIST.head->next;
	p->pre = LIST.head;
	LIST.head->next = p;
	p->next->pre = p;
}

/*
*����LRU�㷨�������ڵĽڵ����˫��������
*/
void setNNode(NNode* p) {
/*	ԭ����Ҫ������һ�εģ������뵽��lookup2�����оͿ��Խ����ҵ�������ֱ����ǰ������ע�͵���
	NNode* q = LIST.head;
	while (q->next != LIST.tail) {
		//�ҵ��˾ͷŵ�head����
		if (!strcmp(q->next->DN, p->DN)) {
			putFirst(p);
			p = q->next;
			q->next = p->next;
			p->next->pre = q;
			free(p);
			return;
		}
		q = q->next;
	}*/
	//û�ҵ��һ���û����ʱ��ŵ�ͷ����size++
	if (LIST.size < LRUSIZE) {
		putFirst(p);
		LIST.size++;
	}
	else {
		putFirst(p);
		p = LIST.tail->pre;
		LIST.tail->pre = p->pre;
		p->pre->next = LIST.tail;
		free(p);
	}
}

/*
*��ȡLIST�����ز�ѯ�����Ľ�������������򷵻�1�����򷵻�0
*domainNameΪ��Ҫ��ȡ��������ipΪ������Ӧ��IP��ַ�����ҵ��Ļ��ͽ�������뵽ip����
*/
int LookUp2(char* domainName, char* ip) {
	NNode* p = LIST.head;
	int flag = 0;
	while (p->next!=LIST.tail) {
		if (strcmp(domainName, p->next->DN) == 0) {
			p = p->next;
			p->pre->next = p->next;
			p->next->pre = p->pre;
			p->pre = LIST.head;
			p->next = LIST.head->next;
			LIST.head->next = p;
			p->next->pre = p;
			flag = 1;
			char* t1 = p->IP;
			char* t2 = p->IP;
			int i = 0;
			/*ͨ��t1��t2����ָ����ж�ȡ��������'.'ʱ��˵����ǰ���ֽ���*/
			while (*t1 != '\0') {
				if (*t1 == '.') {
					*t1 = '\0';
					ip[i] = (char)atoi(t2);
					i++;
					t2 = t1 + 1;
					*t1 = '.';
				}
				t1++;
			}
			/*�ַ���ת����*/
			ip[i] = (char)atoi(t2);
//			printf("�ӻ������ҵ�\n");
			return flag;
		}
		p = p->next;
	}
	return flag;
}

/*
*��ȡ�ļ�������������ͷ�Ľڵ�,ͬʱ����ļ��б������Ϣ
*����fileΪ��Ҫ��ȡ���ļ���·����level��ʾ���Եȼ�
*/
Node* OpenFileT(char* file, int level) {
	initList();
	int flag = 0;						//���ڱ�ǵ�����IP��������
	FILE* f;
	if ((f = fopen(file, "r")) == NULL) {
		perror("Can't open the file");
		exit(1);
	}
	char temp[BUFFER_SIZE];				//����ÿһ�е��ַ���
	Node* head = (Node*)malloc(sizeof(Node));
	head->next = NULL;
	Node* p = head;
	int n = 0;
	while (!feof(f)) {
		p->next = (Node*)malloc(sizeof(Node));
		p = p->next;
		fgets(temp, BUFFER_SIZE, f);
		for (int i = 0, fl = 0, iplength = 0; i < BUFFER_SIZE; i++) {
			/*IP*/
			if (fl == 0) {
				if (temp[i] != ' ') {
					p->IP[i] = temp[i];
				}
				/*Ϊ�ո�ʱ��IP��Χ����*/
				else {
					iplength = i;
					fl = 1;
					p->IP[i] = '\0';
				}
			}
			/*����*/
			else if (fl == 1) {
				if (temp[i] != '\n') {
					p->DN[i - iplength - 1] = temp[i];
				}
				/*Ϊ����ʱ��������Χ����*/
				else {
					p->DN[i - iplength - 1] = '\0';
					break;
				}
			}
		}
		p->next = NULL;
		n++;
	}
	/*���Եȼ�Ϊ2ʱ������ļ���Ϣ*/
	if (level == 2) {
		printf("\n��ȡ�ļ���Ϣ���£�\n");
		for (Node* p = head->next; p; p = p->next) {
			printf("\t%s  ", p->DN);
			printf("%s\n ", p->IP);
		}
		printf("�ļ���ȡ���,��%d����Ϣ\n", n);
	}
	return head;
}

/*
*��ȡ�������ز�ѯ�����Ľ�������������򷵻�1�����򷵻�0
*domainNameΪ��Ҫ��ȡ��������ipΪ������Ӧ��IP��ַ�����ҵ��Ļ��ͽ�������뵽ip�����У�headΪ����ͷ
*/
int LookUp1(char* domainName, char* ip, Node* head) {
	Node* p = head;
	int flag = 0;
	while (p->next) {
		if (strcmp(domainName, p->next->DN) == 0) {
			flag = 1;
			char* t1 = p->next->IP;
			char* t2 = p->next->IP;
			int i = 0;
			/*ͨ��t1��t2����ָ����ж�ȡ��������'.'ʱ��˵����ǰ���ֽ���*/
			while (*t1 != '\0') {
				if (*t1 == '.') {
					*t1 = '\0';
					ip[i] = (char)atoi(t2);
					i++;
					t2 = t1 + 1;
					*t1 = '.';
				}
				t1++;
			}
			/*�ַ���ת����*/
			ip[i] = (char)atoi(t2);
			return flag;
		}
		p = p->next;
	}
	return flag;
}

/*
*��ȡ˫��������������ز�ͬ��ֵ
*/
int LookUp(char* domainname, char* ip, Node* head) {
	if (LookUp2(domainname, ip))
		return 2;
	if (LookUp1(domainname, ip, head))
		return 1;
	return 0;
}

/*
*�������ʱʹ��
*/
void PrintName(char* buf) {
	char* p = buf;
	while (*p != 0) {
		if (*p >= 33) {
			printf("%c", *p);
		}
		else {
			printf(".");
		}
		p++;
	}
}

/*
*�����bufΪͷ����Ӧ���У���from��ʼ��ʾ��CNAME
*/
void PrintNameTemp(char* buf, char* from) {
	char* p = from;
	while (*p != 0) {
		/*��ǰλΪ0xC0ʱ����ʾ��һλΪָ�룬ָ��buf+��һλ���ֵ�λ��*/
		if (*p == (char)192) {
			PrintNameTemp(buf, buf + *((unsigned char*)p + 1));
			return;
		}
		else if (*p >= 33)
			printf("%c", *p);
		/*С��33����'.'�������*/
		else {
			printf(".");
		}
		p++;
	}
	return;
}

/*
*�����bufΪͷ������
*/
void DomainName(char* buf)
{
	printf("[DomainName = ");
	PrintName(buf);
	printf("]\t");
}

/*
*�����bufΪͷ������
*ͬʱ�Ὣ��ֵС��33���ַ��ĳ�'.'��������LookUp()�����н��бȽ�
*Ҳ�Ὣ����ת����Сд
*/
void ToDomainName(char* buf) {
	char* p = buf;
	while (*p != 0) {
		if (*p >= 33) {
			if (*p >= 'A' && *p <= 'Z')
				*p = *p + 32;
		}
		else {
			*p = '.';
		}
		p++;
	}
}

/*
*���ݱ������û��棬ֻ��dns0��dns1��ʹ��
*/
void setBuf(char* buf) {
	char* ip = (char*)malloc(4);
	char* p = buf + 6;						//��ʱָ��ancount����ʾ������
	unsigned short n = *(unsigned short*)p;	//ǿ��ת����2B��short����
	n = ntohs(n);							//ע���С�˵ĵ���
	p = buf + 12;
	p = p + strlen(p) + 1 + 4;
	int first = 0;
	//pָ���һ��answer����
	for (unsigned short i = 0; i < n; i++) {
		p = p + 2;
		unsigned short type = ntohs(*(unsigned short*)p);	//answer����
		p = p + 8;
		unsigned short length = ntohs(*(unsigned short*)p);	//answer�е�data����
		/*IPV4��ַ*/
		if (type == 1) {
			ip = p + 2;
			if (!first) {
				NNode* node = (NNode*)malloc(sizeof(NNode));
				strcpy(node->DN, buf + 13);
				ToDomainName(node->DN);
				sprintf(node->IP, "%u.%u.%u.%u", *(ip) & 0xff, *(ip + 1) & 0xff, *(ip + 2) & 0xff, *(ip + 3) & 0xff);
				setNNode(node);
				return;
			}
		}
		p = p + 2 + length;
	}
}

/*
*������Ӧ���ģ��������в�ѯ��ʱʹ�ã���Ҫ��ͨ�����Ĳ�ѯ����ʵ�ֵ�
*bufΪѯ�ʱ��ģ�ipΪ��ѯ����IP��ַ��levelΪ���Եȼ���ֻ�е��Եȼ�Ϊ2ʱ�����������ȫ��Ϣ
*/
void Respond(char* buf, char* ip, int level) {
	DNSHEADER* header = (DNSHEADER*)buf;
	DNSRESOURCE* resouce;
	/*IPΪ0.0.0.0��Ӧ����flag�е�RCODEΪ0x3����ʾ����������*/
	if (ip[0] == (char)0 && ip[1] == (char)0 && ip[2] == (char)0 && ip[3] == (char)0) {
		/*���Եȼ�Ϊ2ʱ�Ż����*/
		if (level == 2)
			printf("IPaddr is 0.0.0.0, the domainname is unsafe.\n");
		header->Flag = htons(0x8183);
	}
	/*�����������Ӧ����flagΪ0x8180*/
	else {
		header->Flag = htons(0x8180);
	}
	/*�ش���Ϊ1*/
	header->Ancount = htons(1);

	char* dn = buf + 12;							//ָ�����е�questionͷ
	char* name = dn + strlen(dn) + 1 + 4;			//ָ�����е�answerͷ
	unsigned short* nameTemp = (unsigned short*)name;
	*nameTemp = htons(0xC00C);						//��answer��ǰ�����ֽ�д��0xC0
	/*��answer���ֽ�����д*/
	resouce = (DNSRESOURCE*)(name + 2);
	resouce->Type = htons(1);
	resouce->Class = htons(1);
	resouce->TTL = htons(0x0FFF);
	resouce->Length = htons(4);
	/*����IP��*/
	char* data = (char*)resouce + 10;
	*data = *ip;
	*(data + 1) = *(ip + 1);
	*(data + 2) = *(ip + 2);
	*(data + 3) = *(ip + 3);
}

/*
*���Եȼ�Ϊ1ʱʹ�ã�һ��ֻ����Ӧ�����������ʱ��Ϳͻ���IP��ַ���Լ�����
*/
void PrintAnswerLess(SOCKADDR_IN client, const char* buf) {
	time_t NowTime = time(NULL);
	struct tm* t = localtime(&NowTime);
	printf("\n%d/%02d/%02d,%02d:%02d:%02d  ", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
	printf("Client %d.%d.%d.%d    ", client.sin_addr.S_un.S_un_b.s_b1, client.sin_addr.S_un.S_un_b.s_b2, client.sin_addr.S_un.S_un_b.s_b3, client.sin_addr.S_un.S_un_b.s_b4);

	char* ip = (char*)malloc(4);
	char* p = buf + 12;
	DomainName(p + 1);
}

/*
*���Եȼ�Ϊ2ʱʹ�ã����ڶ�ȡ��ѯ������Ӧ���������ݲ�ͬ���ͽ������
*��ѯ�����ʱ�䡢��ѯ���͡��Լ��ͻ��˵�ַ
*��Ӧ�������ʱ�䡢�𰸣�������������IPV4��ַ��CNAME��PTRָ�룩��������IP�Ϳͻ���IP
*/
void PrintAnswerMore(SOCKADDR_IN client, const char* buf, char* server_ip) {
	printf("********************************************************************\n");

	time_t NowTime = time(NULL);
	struct tm* t = localtime(&NowTime);
	printf("%d/%02d/%02d,%02d:%02d:%02d\n", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
	char* ip = (char*)malloc(4);
	char* p = buf + 6;						//��ʱָ��ancount����ʾ������
	unsigned short n = *(unsigned short*)p;	//ǿ��ת����2B��short����
	n = ntohs(n);							//ע���С�˵ĵ���
	p = buf + 12;
	printf("[ID = 0x%x]\n", ntohs((*((unsigned short*)buf)) & 0xFFFF));
	DomainName(p + 1);

	/*ttָ��header�е�flag�������жϵ�һλ��QR��Ϊ0��ʾ��ѯ��1��ʾ��Ӧ*/
	unsigned short* tt = buf + 2;
	/*��ѯ���е���Ϣ���*/
	if (*tt >> 15 == 0) {
		char* temp = buf + 12;
		temp = temp + strlen(temp) + 1;
		printf("\n[TYPE = %u]\n", ntohs((*(unsigned short*)temp)) & 0xFFFF);
		printf("\nASK FROM CLIENT %d.%d.%d.%d\n", client.sin_addr.S_un.S_un_b.s_b1, client.sin_addr.S_un.S_un_b.s_b2, client.sin_addr.S_un.S_un_b.s_b3, client.sin_addr.S_un.S_un_b.s_b4);
		return;
	}
	/*��Ӧ���е���Ϣ���*/
	printf("\n\n");
	p = p + strlen(p) + 1 + 4;	
	int first = 0;
	//pָ���һ��answer����
	for (unsigned short i = 0; i < n; i++) {
		p = p + 2;
		unsigned short type = ntohs(*(unsigned short*)p);	//answer����
		p = p + 8;
		unsigned short length = ntohs(*(unsigned short*)p);	//answer�е�data����
		/*IPV4��ַ*/
		if (type == 1) {
			ip = p + 2;
			printf("[IP = %u.%u.%u.%u]\n", *(ip) & 0xff, *(ip + 1) & 0xff, *(ip + 2) & 0xff, *(ip + 3) & 0xff);
			if (!first) {
				NNode* node = (NNode*)malloc(sizeof(NNode));
				strcpy(node->DN, buf + 13);
				ToDomainName(node->DN);
				sprintf(node->IP, "%u.%u.%u.%u", *(ip) & 0xff, *(ip + 1) & 0xff, *(ip + 2) & 0xff, *(ip + 3) & 0xff);
				//printf("++++%s\n++++%s", node->DN, node->IP);
				setNNode(node);
				first = 1;
			}
		}
		/*CNAME*/
		else if (type == 5) {
			printf("[CNAME = ");
			PrintNameTemp(buf, p + 3);
			printf("]\n");
		}
		/*PTRָ��*/
		else if (type == 12) {
			printf("[PTR = ");
			PrintName(p + 3);
			printf("]\n");
		}
		/*IPV6��ַ���ҵĵ��Դ���û�յ���IPV6ָ�룬����д��û��Ҫ*/
		else if (type == 28) {
		}
		p = p + 2 + length;
	}
	/*����Ϊ0*/
	if (n == 0) {
		printf("No answer in this package\n");
	}
	/*����������Ϳͻ�����Ϣ*/
	printf("\nGET FROM SERVER %s\n", server_ip);
	printf("SEND TO CLIENT %d.%d.%d.%d\n", client.sin_addr.S_un.S_un_b.s_b1, client.sin_addr.S_un.S_un_b.s_b2, client.sin_addr.S_un.S_un_b.s_b3, client.sin_addr.S_un.S_un_b.s_b4);

	printf("********************************************************************\n");
}

/*
*�޵��Եȼ�ʱʹ��
*/
void dns0() {
	char buf[BUFFER_SIZE];		//���������Ϣ

	/*׼��UDPͨ��*/
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		printf("error\n");
		exit(1);
	}
	/*������ͻ��˹�ͨ���׽���*/
	SOCKET socketFd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (socketFd == SOCKET_ERROR) {
		printf("Creat Socket Error\n");
		exit(1);
	}
	/*��������Ϊ����������IP�Ͷ˿�*/
	SOCKADDR_IN server;
	server.sin_family = AF_INET;
	server.sin_port = htons(PORT);
	server.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	/*�ͻ��ˣ�����Ҫ������Ϣ*/
	SOCKADDR_IN client;

	int z = bind(socketFd, (struct sockaddr*)&server, sizeof(server));
	if (z != 0) {
		printf("bind error\n");
		exit(1);
	}

	/*������dns��������ͨ���׽���*/
	SOCKET DnsFd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (DnsFd == SOCKET_ERROR) {
		printf("Creat Socket Error\n");
		exit(1);
	}
	/*���ý��շ���������ֹ����ʧ����ѭ���п�ס*/
	int timeout = 2000;
	setsockopt(DnsFd, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(int));

	/*dns����������IP�Ͷ˿�*/
	SOCKADDR_IN Dns;
	Dns.sin_family = AF_INET;
	Dns.sin_port = htons(PORT);
	Dns.sin_addr.S_un.S_addr = inet_addr(SERVER_IP);

	char* temp = (char*)malloc(BUFFER_SIZE);	//����������
	char* ip = (char*)malloc(4);				//��IP
	unsigned int len = sizeof(client);
	while (1) {
		/*��buf��ȫ����0*/
		memset(buf, 0, BUFFER_SIZE);

		/*�ӿͻ��˽�����Ϣ������ʧ���������һ��ѭ��*/
		z = recvfrom(socketFd, buf, BUFFER_SIZE, 0, (struct sockaddr*)&client, &len);
		if (z < 0) {
			continue;
		}
		/*��������ֵ��temp������*/
		strcpy(temp, buf + sizeof(DNSHEADER) + 1);
		ToDomainName(temp);
		

		/*ȷ��ѯ������*/
		char* tempp = buf + 12;
		char* typeptr = tempp + strlen(tempp) + 1;
		unsigned short type = ntohs((*(unsigned short*)typeptr)) & 0xFFFF;
		
		int ifFind = 0;
		/*ѯ������ΪIPV4���Ҳ�ѯ�����������Ӧ����*/
		if (type == 1) {
			ifFind = LookUp(temp, ip, HEAD);
			if (ifFind) {
				int ifFind = LookUp(temp, ip, HEAD);
				Respond(buf, ip, 1);
			}
		}
		/*ѯ�����Ͳ���IPV4����δ��ѯ������Ҫ���ϲ��dns���������в�ѯ*/
		if(!ifFind){
			/*����ѯ��ԭ�ⲻ���ķ��͸�dns������*/
			sendto(DnsFd, buf, BUFFER_SIZE, 0, (struct sockaddr*)&Dns, sizeof(Dns));

			unsigned short id = *(unsigned short*)buf;
			unsigned short idtemp;
			unsigned int i = sizeof(Dns);
			unsigned int j;
			/*ֻ����Ӧ�����ѯ����id��ͬʱ��ֹͣ���գ�
			*���ڽ��յķ�ʽ�����˷����������������õĳ�ʱʱ��ʱ�����Զ�����һ��û�д𰸵���Ӧ��
			*���dns������֮ǰ���͵���Ӧ������Ƚ�����ô�ͻ���������ѯ�ʵĲ����ϣ��ͻ������������ʵ������������Ҫ����һ��ɸѡ
			*/
			do
			{
				j = recvfrom(DnsFd, buf, BUFFER_SIZE, 0, (struct sockaddr*)&Dns, &i);
				idtemp = *(unsigned short*)buf;
			} while (idtemp != id);
			setBuf(buf);
		}
		/*����Ӧ��ԭ�ⲻ�����͸��ͻ��ˣ�����ʧ���򲻶��ط������ͳɹ�*/
		do
		{
			z = sendto(socketFd, buf, sizeof(buf), 0, (struct sockaddr*)&client, sizeof(client));
		} while (z < 0);
	}
}

/*
*���Եȼ�Ϊ1ʱʹ�ã����ں�dns0()�������𲻴�����ֻ�ڲ���������ĵط����н���
*/
void dns1(char* IP, char* file) {
	char buf[BUFFER_SIZE];

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
	server.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	SOCKADDR_IN client;

	int z = bind(socketFd, (struct sockaddr*)&server, sizeof(server));
	if (z != 0) {
		printf("bind error\n");
		exit(1);
	}

	SOCKET DnsFd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	int timeout = 2000;
	setsockopt(DnsFd, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(int));
	if (DnsFd == SOCKET_ERROR) {
		printf("Creat Socket Error\n");
		exit(1);
	}
	SOCKADDR_IN Dns;
	Dns.sin_family = AF_INET;
	Dns.sin_port = htons(PORT);
	Dns.sin_addr.S_un.S_addr = inet_addr(IP);

	char* temp = (char*)malloc(BUFFER_SIZE);
	char* ip = (char*)malloc(4);
	unsigned int len = sizeof(client);
	while (1) {
		memset(buf, 0, BUFFER_SIZE);
		z = recvfrom(socketFd, buf, BUFFER_SIZE, 0, (struct sockaddr*)&client, &len);
		if (z < 0) {
			continue;
		}
		strcpy(temp, buf + sizeof(DNSHEADER) + 1);
		ToDomainName(temp);

		/*ȷ��ѯ������*/
		char* tempp = buf + 12;
		char* typeptr = tempp + strlen(tempp) + 1;
		unsigned short type = ntohs((*(unsigned short*)typeptr)) & 0xFFFF;

		int ifFind = 0;

		/*���Եȼ�Ϊ1ʱ����Ҫ�����Ӧ����Ϣ*/
		if (type == 1) {
			ifFind = LookUp(temp, ip, HEAD);
			if (ifFind) {
				time_t NowTime = time(NULL);
				struct tm* t = localtime(&NowTime);
				printf("\n%d/%02d/%02d,%02d:%02d:%02d  ", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
				printf("Client %d.%d.%d.%d    ", client.sin_addr.S_un.S_un_b.s_b1, client.sin_addr.S_un.S_un_b.s_b2, client.sin_addr.S_un.S_un_b.s_b3, client.sin_addr.S_un.S_un_b.s_b4);
				DomainName(temp);
				Respond(buf, ip, 1);
			}
		}
		if(!ifFind){
			sendto(DnsFd, buf, BUFFER_SIZE, 0, (struct sockaddr*)&Dns, sizeof(Dns));

			unsigned short id = *(unsigned short*)buf;
			unsigned short idtemp;
			unsigned int i = sizeof(Dns);
			unsigned int j;
			do
			{
				j = recvfrom(DnsFd, buf, BUFFER_SIZE, 0, (struct sockaddr*)&Dns, &i);
				idtemp = *(unsigned short*)buf;
			} while (idtemp != id);
			/*�����Ӧ������Ϣ*/
			PrintAnswerLess(client, buf);
			setBuf(buf);
		}
		do
		{
			z = sendto(socketFd, buf, sizeof(buf), 0, (struct sockaddr*)&client, sizeof(client));
		} while (z < 0);
	}
}

/*
*ͬdns1�����Ͳ��ֲ�ͬ����
*/
void dns2(char* IP) {
	char buf[BUFFER_SIZE];

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
	server.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	SOCKADDR_IN client;

	int z = bind(socketFd, (struct sockaddr*)&server, sizeof(server));
	if (z != 0) {
		printf("bind error\n");
		exit(1);
	}

	SOCKET DnsFd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	int timeout = 2000;
	setsockopt(DnsFd, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(int));
	if (DnsFd == SOCKET_ERROR) {
		printf("Creat Socket Error\n");
		exit(1);
	}

	SOCKADDR_IN Dns;
	Dns.sin_family = AF_INET;
	Dns.sin_port = htons(PORT);
	Dns.sin_addr.S_un.S_addr = inet_addr(IP);

	char* temp = (char*)malloc(BUFFER_SIZE);
	char* ip = (char*)malloc(4);
	unsigned int len = sizeof(client);
	while (1) {
		memset(buf, 0, BUFFER_SIZE);
		z = recvfrom(socketFd, buf, BUFFER_SIZE, 0, (struct sockaddr*)&client, &len);
		printf("\n\n");
		if (z < 0) {
			continue;
		}
		PrintAnswerMore(client, buf, IP);
		strcpy(temp, buf + sizeof(DNSHEADER) + 1);
		ToDomainName(temp);

		/*ȷ��ѯ������*/
		char* tempp = buf + 12;
		char* typeptr = tempp + strlen(tempp) + 1;
		unsigned short type = ntohs((*(unsigned short*)typeptr)) & 0xFFFF;

		int ifFind = 0;
		/*�������Ϣ����*/
		if (type == 1) {
			ifFind = LookUp(temp, ip, HEAD);
			if (ifFind) {
				printf("********************************************************************\n");
				time_t NowTime = time(NULL);
				struct tm* t = localtime(&NowTime);
				printf("%d/%02d/%02d,%02d:%02d:%02d\n", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
				printf("ASK FROM CLIENT %d.%d.%d.%d\n\n", client.sin_addr.S_un.S_un_b.s_b1, client.sin_addr.S_un.S_un_b.s_b2, client.sin_addr.S_un.S_un_b.s_b3, client.sin_addr.S_un.S_un_b.s_b4);
				printf("[ID = 0x%x]\n", ntohs((*((unsigned short*)buf)) & 0xFFFF));

				DomainName(temp);
				printf("\n[IP = %u.%u.%u.%u]\n", *ip & 0xff, *(ip + 1) & 0xff, *(ip + 2) & 0xff, *(ip + 3) & 0xff);
				Respond(buf, ip, 2);
				if (ifFind == 1)
					printf("\nFind From Host %s\n", LOCAL_IP);
				else if (ifFind == 2)
					printf("\nFind From Host Cache\n");
				printf("SEND TO CLIENT %d.%d.%d.%d\n", client.sin_addr.S_un.S_un_b.s_b1, client.sin_addr.S_un.S_un_b.s_b2, client.sin_addr.S_un.S_un_b.s_b3, client.sin_addr.S_un.S_un_b.s_b4);
				printf("********************************************************************\n");
			}
		}
		if(!ifFind){
			sendto(DnsFd, buf, BUFFER_SIZE, 0, (struct sockaddr*)&Dns, sizeof(Dns));
			unsigned short id = *(unsigned short*)buf;
			unsigned short idtemp;
			unsigned int i = sizeof(Dns);
			unsigned int j;
			do
			{
				j = recvfrom(DnsFd, buf, BUFFER_SIZE, 0, (struct sockaddr*)&Dns, &i);
				idtemp = *(unsigned short*)buf;
			} while (idtemp != id);
			/*�����Ӧ����Ϣ*/
			PrintAnswerMore(client, buf, IP);
		}

		do
		{
			z = sendto(socketFd, buf, sizeof(buf), 0, (struct sockaddr*)&client, sizeof(client));
		} while (z < 0);
	}
}

/*
*main()��������������в����Ĳ�ͬ������ִ�в�ͬ��dns����
*/
int main(int argc, char* argv[], char* envp[]) {
	printf("\nDNSRELAY, Version 1.5 Build: 2020/09/05 16:23\n");
	printf("Usage: dnsrelay [-d|-dd] [<dns-server>] [<db-file>]\n\n");

	if (argc == 1) {
		printf("������Ϣ�Ӽ���0 �޵�����Ϣ���\n");
		printf("ָ�����ַ�����Ϊ %s:53\n", SERVER_IP);
		printf("ʹ��Ĭ�������ļ� %s\n", LOCAL_FILE);
		HEAD = OpenFileT(LOCAL_FILE, 0);
		dns0();
	}
	else if (argc == 4 && !strcmp(argv[1], "-d")) {
		printf("������Ϣ�Ӽ���1 �򵥵�����Ϣ���\n");
		printf("ָ�����ַ�����Ϊ %s:53\n", argv[2]);
		printf("ʹ��ָ�������ļ� %s\n", argv[3]);
		HEAD = OpenFileT(argv[3], 1);
		dns1(argv[2], argv[3]);
	}
	else if (argc == 3 && !strcmp(argv[1], "-dd")) {
		printf("������Ϣ�Ӽ���2 ���ӵ�����Ϣ���\n");
		printf("ָ�����ַ�����Ϊ %s:53\n", argv[2]);
		printf("ʹ��Ĭ�������ļ� %s\n", LOCAL_FILE);
		HEAD = OpenFileT(LOCAL_FILE, 2);
		dns2(argv[2]);
	}
	else {
		printf("����������������������\n");
	}
	return 0;
}
