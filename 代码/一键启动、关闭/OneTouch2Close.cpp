// OneTouch2Close.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。

#include <iostream>
#include <string>
#include <fstream>
#include <atlstr.h>
#include <vector>
#include <windows.h>
#include <Ws2tcpip.h>  //InetNtop 和InetPton 声明的文件

#pragma comment(lib,"Ws2_32.lib")

using namespace std;

//倒计时关闭
void shutdown() {
	for (int i = 5; i >= 1; --i) {
		cout << "该启动器将在" << i << "s后自动关闭\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b";
		Sleep(1000);
	}
}

//只保留ASCII码大于32的字符，32为空格，以下的都是控制字符
void myStrcpy(string& dst, string& src) {
	size_t i;
	const char* cpSrc;
	dst.clear();
	if (src.empty())return dst.clear();
	cpSrc = src.c_str();
	for (i = 0; i < strlen(cpSrc); i++)
		if (cpSrc[i] > 32 || cpSrc[i] < 0)
			dst.append(1, cpSrc[i]);
}

//从截取的实体描述字串的@后取出IP地址，如果没有@，则默认为回环地址
string getAddr(string* str) {
	string addr;
	int retval = (int)str->find('@');
	if (retval == -1) addr = "127.0.0.1";//使用默认参数
	else addr = str->substr(retval + 1);//取@后的IP地址
	return addr;
}

//以dev，层次号，实体号生成UDP端口号，格式为十进制，1 dev1 layer1 enty2
string getPort(string* dev, int iLayer, string* ent) {
	string strTmp;
	string strLayer;
	
	//用设备号，层次，实体号生成唯一UDP端口号
	strTmp = "1";
	strTmp += dev->c_str();
	strLayer = std::to_string(iLayer);//层次号;
	strTmp += strLayer;
	if (ent->length() == 1)strTmp += "0";//只有1位，补上0
	strTmp += ent->c_str();
	return strTmp;
}

//判断本行是否为分割行，依据是连续的----
bool isSplitLine(string* pstr) {
	int retval;
	retval = (int)pstr->find("--------");
	if (retval < 0) return 0;
	return 1;
}

//判断本行是否是参数行，依据是开头的#
bool isParmsLine(string* pstr) {
	if (pstr->at(0) == '#' || isSplitLine(pstr)) return 0;
	return 1;
}

// 从描述字串中提取设备号，如果字串以空格开始，则提取不到。
string getDev(string* pstr) {
	int retval;
	retval = (int)pstr->find_first_of(" ");
	if (retval <= 0) return string();
	return pstr->substr(0, retval);
}

//从截取好的实体描述字段中，提取层次号和实体号
int getLayerAndEnt(string* str, string& strLayer, string& strEnt) {
	int retval;
	int i;
	
	retval = (int)str->find("@");//要从str中找出实体号字段
	if (retval == -1) strEnt = str->c_str();//没有IP地址，最后1到两个字符是数字
	else strEnt = str->substr(0, retval);//取@之前

	for (i = (int)strEnt.length() - 1; i >= 0; i--)
		if (strEnt.at(i) < '0' || strEnt.at(i) > '9')
			break;

	strEnt = strEnt.substr(i + 1, strEnt.length() - i - 1);
	strLayer = str->substr(0, i + 1);

	return 0;
}

//获得在结构字串中，指定layer的层次号，在结构字串中，每行每层只有一个实体。同层多个实体要分行表示
//求层次的编号也就是获得层次位置，层次之间以空格间隔，没有设备号，要以空格开始
//返回值1表示最低层，-1表示没找到
int getLayerNo(string* pStr, string* lay) {
	string strTmp;
	int begin;
	int end;
	int i;

	//判断设备号情况,-2 表示设备号不匹配，没有设备号则可继续
	begin = (int)pStr->find(' ');

	//整个字符串没有空格，格式不对,跳过
	if (begin == -1)return -1;

	for (i = 1; begin < (int)pStr->length() && begin >= 0; i++) {
		//从begin开始找第一个非0的字段,跳过下一阶段空格
		begin = (int)pStr->find_first_not_of(' ', begin);
		//跳过空格之后没东西
		if (begin == -1)break;
		//截取子参数
		strTmp = pStr->substr(begin, 3);
		//找到我们自己的层次了
		if (strTmp.find(lay->c_str(), 0, 3) != -1) break;

		end = (int)pStr->find_last_of(strTmp.c_str(), begin, 3);
		begin = (int)pStr->find_first_of(' ', end);
	}

	//给出的层次名不对找不到
	if (begin < 0) return -1;

	return i;
}

//自编的IP地址字符串转换为网络字节序整数形式
unsigned long myInetAddr2n(string* pstr) {
	int begin;
	int end;
	unsigned long retval;
	unsigned long temp;
	string strTmp;

	retval = 0;
	begin = (int)pstr->find_first_of(".");
	if (begin < 0)return 0;

	strTmp = pstr->substr(0, begin);
	temp = atoi(strTmp.c_str());
	retval = (temp << 24) & 0xff000000;

	begin = begin + 1;
	end = (int)pstr->find_first_of(".", begin);
	if (end < 0) return 0;

	strTmp = pstr->substr(begin, end - begin);
	temp = atoi(strTmp.c_str());
	retval += (temp << 16) & 0x00ff0000;

	begin = end + 1;
	end = (int)pstr->find_first_of(".", begin);
	if (end < 0) return 0;

	strTmp = pstr->substr(begin, end - begin);
	temp = atoi(strTmp.c_str());
	retval += (temp << 8) & 0x0000ff00;

	begin = end + 1;
	strTmp = pstr->substr(begin);
	temp = atoi(strTmp.c_str());
	retval += temp & 0x000000ff;

	return htonl(retval);
}

int main() {
	WSAData wsa;
	WSAStartup(0x201, &wsa);

	//打开配置文件
	ifstream cfgFile("ne.txt");
	if (!cfgFile.is_open()) {
		cout << "ERROR: 没有找到ne.txt文件" << endl << endl;
		shutdown();
		return 0;
	}
	cout << "读取ne.txt文件成功" << endl;

	//遍历配置文件
	//通过设备号，层次名，和实体号，得到四个参数组:basic, lower , upper，peer
	string* pstrTmp;
	string strTmp;
	vector <string*> archSection;
	while (!cfgFile.eof()) {
		getline(cfgFile, strTmp);
		if (isSplitLine(&strTmp)) break;
	}
	if (cfgFile.eof()) {//没有读到有效内容
		cfgFile.close();
		cout << endl << "ERROR: ne.txt文件缺失有效内容" << endl << endl;
		shutdown();
		return -2;
	}
	//读入架构和地址区域内容
	while (!cfgFile.eof()) {
		getline(cfgFile, strTmp);
		if (isSplitLine(&strTmp)) break;
		if (!isParmsLine(&strTmp)) continue;
		pstrTmp = new string(strTmp.c_str());
		archSection.push_back(pstrTmp);
	}
	cfgFile.close();
	//判定架构和地址区域内容
	if (archSection.size() == 0) {
		cout << endl << "ERROR: ne.txt文件缺失构和地址区域内容" << endl << endl;
		shutdown();
		return 0;
	}
	cout << "读取配置成功" << endl;

	//读完文件，开始分析
	int NECount = 0;
	size_t index;
	string strDev;
	string strLayer;
	string strEntity;
	string strCmd;
	string strParm;
	string strAddr;
	string strPort;
	int begin;
	int end;
	SOCKET sock;
	int iLayer;

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock == SOCKET_ERROR)return 0;

	int retval;
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(20000);
	retval = bind(sock, (sockaddr*)&addr, sizeof(sockaddr_in));

	for (index = 0; index < archSection.size(); index++) {
		//读出一个，执行一个
		pstrTmp = archSection[index];
		//读设备号
		begin = 0;
		end = (int)pstrTmp->find_first_of(" ");
		//这一行不对
		if (end < 0)continue;
		//如果有设备号，就提取，否则就用之前的
		else if (end > 0) strDev = pstrTmp->substr(0, end);

		begin = end;
		iLayer = 1;
		while (begin < (int)pstrTmp->length() && begin >= 0) {
			//跳过空格
			begin = (int)pstrTmp->find_first_not_of(' ', begin);
			if (begin == -1)break;
			//截取
			end = (int)pstrTmp->find_first_of(' ', begin);
			//后面没有了
			if (end == -1)end = (int)pstrTmp->length();

			strTmp = pstrTmp->substr(begin, end - begin);
			getLayerAndEnt(&strTmp, strLayer, strEntity);

			strAddr = getAddr(&strTmp);
			strPort = getPort(&strDev, iLayer, &strEntity);

			addr.sin_addr.S_un.S_addr = myInetAddr2n(&strAddr);
			addr.sin_port = htons(atoi(strPort.c_str()));

			//逐一发送关闭命令
			retval = sendto(sock, "exit", 5, 0, (sockaddr*)&(addr), sizeof(sockaddr_in));
			if (retval <= 0) retval = WSAGetLastError();

			cout << "关闭网元 " << strDev << " 的 " << strLayer << " 层实体 " << strEntity << " 地址：" << addr.sin_addr.S_un.S_addr << " port:" << addr.sin_port << endl;
			NECount++;
			begin = end;
			iLayer++;
		}
	}

	cout << "读出网元 " << NECount << " 个" << endl << endl << "所有文件已关闭" << endl << endl;
	closesocket(sock);
	WSACleanup();
	shutdown();
	return 0;
}