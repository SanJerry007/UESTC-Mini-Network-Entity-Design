// readfile.cpp : Defines the entry point for the console application.
//
//#include "pch.h"
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <winsock2.h>
#include <windows.h>
//#pragma comment(lib,"ws2_32.lib")
//#include <iostream>
using namespace std;


class CCfgFileParms{
public:
	enum AddrType { LOWER, LOCAL, UPPER, CMDER };
	vector<string*> archSection; //存放该区域的内容的原始数据
	vector<string*> linksSection;
	vector<string*> parmsSection;
	struct parmEntry {
		string name;
		string strValue;
		int value;
	};
	vector <struct parmEntry> cfgParms; //分解后的参数

	bool isConfigExist;
	int subnet;

	int getValueInt(int& val, char* name); //根据参数的名字，取得参数的数值，name 参数名字，0表示参数找到，-1表示没找到
	int getValueInt(int& val, string name);
	string getValueStr(char* name);//根据参数名字，取得参数的字面值
	string getValueStr(string name);//根据参数名字，取得参数的字面值

	void print();
	void print(WORD color);
	int read();
	void setDeviceID(string ID)
	{
		deviceID = ID;
	}
/*
	void setDeviceID(int ID){
		deviceID = itoa(ID,10);
	}
*/
	void setLayer(string lay)
	{
		layer = lay;
	}

	void setEntityID(string ID)
	{
		entityID = ID;
	}
/*
	void setEntityID(int ID){
		entityID = itoa(ID,10);
	}
*/
	CCfgFileParms();
	CCfgFileParms(string devID, string layerID, string entID);

	~CCfgFileParms();
protected:
	string deviceID;
	string layer;
	string entityID;

	string localAddr; //读文件，只需要获得字符串类型地址，在执行体中，再根据需要转换
	string localPort;
	vector<string> lowerAddr;
	vector<string> lowerPort;
	string upperAddr;
	string upperPort;
	string cmdAddr;
	string cmdPort;

	void myStrcpy(char* str, string src);

	void printArray();
	int readArray();

	int findAddr(string* pStr, string* pLay, string* pEnt);//在结构字串中找到指定层次名和实体号的字串位置

private:
	unsigned short createHexPort(int inID, int inLayer, int inEntity, int inType);
	int getLayerNo(string* pStr, string* lay); //根据结构字串中指定层次名所处的位置，获得层次的编号，以生成端口号
	string getAddr(string* str);
	string getPort(string* dev, int iLayer, string* ent);//以dev，层次号，实体号生成端口号
	string getEnt(string* str);//从截取好的实体描述中提取去掉层次名的实体号
	bool isParmsLine(string* pstr);
	bool isSplitLine(string* pstr);
	int cleanSections();
	int compareDevID(string* pstr, string* dev);
	int getAddrFromSection(string* dev, string* lay, string* ent, string& addr, string& port, int& indexOfDev, int& indexOfThis, int& iLayNo);
	int getUpperAddr(size_t indexOfDev, size_t indexOfThis, int iLay, string& addr, string& port);
	int getLowerAddr(size_t indexOfDev, size_t indexOfThis, int iLay, vector<string>& addr, vector<string>& port);
	// 从描述字串中提取设备号，如果以空格开始，则错误。
	string getDev(string* pstr);
	// 从链路字串的设定范围内，根据逗号，取出设备号和实体号，start包含在范围内，end不包含！
	int getDevEntFromLink(string* pstr, int start, int end, string& dev, string& port);
public:
	string getUDPAddrString(AddrType type, int index);
	sockaddr_in getUDPAddr(AddrType type, int index);
	int getUDPAddrNumber(AddrType type);
	unsigned long myInetAddr2n(string* pstr);
	int getLinkIndex();
	string getDeviceID();
	string getLayer();
	string getEntity();
};
