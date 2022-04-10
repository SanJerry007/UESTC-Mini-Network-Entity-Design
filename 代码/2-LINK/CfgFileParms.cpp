// readfile.cpp : Defines the entry point for the console application.
//
//#include "stdafx.h"
//#include "pch.h"
#include <string>
#include <fstream>
#include <iostream>
#include <winsock2.h>
#include <windows.h>
#include "CfgFileParms.h"
#pragma comment(lib,"ws2_32.lib")
using namespace std;
//20200228，简化配置文件，
//目标是UDP端口号都根据设备号和实体号自动生成；端口不能工作时，报错。一键启动软件，先检测所有端口是否都能工作，
//上下层关系由一个统一的区域描述，也是自动生成；根据描述自动生成
//拓扑连接关系由一个区域简化描述，根据描述生成连接对端的信息。注意共享信道，默认第一个描述的实体是管理端口
//统一管理也归口到数据端口，接收命令由源端判断
//其他参数减少了，不再分BASIC等组，如果一组有多个，比如低层接口模式，用lowerMode0=，lowerMode1=来表示
CCfgFileParms::CCfgFileParms()
{
	//初始化

	isConfigExist = false;

}
CCfgFileParms::CCfgFileParms(string devID, string layerID, string entID)
{
	//初始化
	isConfigExist = false;

	deviceID = devID;
	layer = layerID;
	entityID = entID;
	read();
	subnet=0;
	getValueInt(subnet,(char*)"subnet");
}

void CCfgFileParms::myStrcpy(char* str, string src) //只保留ASCII码大于32的字符，32为空格，以下的都是控制字符
{
	size_t i, j;
	j = 0;
	for (i = 0; i < strlen(src.c_str()); i++) {
		if (src[i] > 32 || src[i] < 0) {
			str[j] = src.c_str()[i];
			j++;
		}
	}
	str[j] = 0;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int CCfgFileParms::getValueInt(int& val, char* name)
{
	size_t i;
	int retval;

	for (i = 0; i < cfgParms.size(); i++) {
		retval = (int)cfgParms[i].name.find(name);
		if (retval >= 0) {
			val = cfgParms[i].value;
			return 0;
		}
	}
	return -1;
}
int CCfgFileParms::getValueInt(int& val, string name)
{
	size_t i;
	int retval;

	for (i = 0; i < cfgParms.size(); i++) {
		retval = (int)cfgParms[i].name.find(name);
		if (retval >= 0) {
			val = cfgParms[i].value;
			return 0;
		}
	}
	return -1;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

string CCfgFileParms::getValueStr(char* name)
{
	size_t i;
	int retval;

	for (i = 0; i < cfgParms.size(); i++) {
		retval = (int)cfgParms[i].name.find(name);
		if (retval >= 0) {
			return cfgParms[i].strValue;
		}
	}
	return string();
}
string CCfgFileParms::getValueStr(string name)
{
	size_t i;
	int retval;

	for (i = 0; i < cfgParms.size(); i++) {
		retval = (int)cfgParms[i].name.find(name);
		if (retval >= 0) {
			return cfgParms[i].strValue;
		}
	}
	return string();
}
//成组打印
void CCfgFileParms::printArray()
{
	size_t i;
	// TODO: 在此处添加实现代码.
	for (i = 0; i < cfgParms.size(); i++) {
		cout << cfgParms[i].name << " = " << cfgParms[i].strValue << endl;
	}
	return;
}

void CCfgFileParms::print()
{
	size_t i;
	cout << "设备号: " << deviceID << " 层次: " << layer << "实体: " << entityID  << endl;
	cout << "上层实体地址: " << upperAddr << "  UDP端口号: " << upperPort << endl;
	cout << "本层实体地址: " << localAddr << "  UDP端口号: " << localPort << endl;
	if (layer.compare("PHY") == 0) {
		if ((int)lowerAddr.size() <= 1) {
			cout << "下层点到点信道" << endl;
			cout << "链路对端地址: ";
		}
		else {
			cout << "下层广播式信道" << endl;
			cout << "共享信道站点：";
		}
	}
	else {
		cout << "下层实体";
	}
	if ((int)lowerAddr.size() == 1) {
		cout << "地址：" << lowerAddr[0] << "  UDP端口号: " << lowerPort[0] << endl;
	}
	else {
		if (layer.compare("PHY") == 0) {
			cout << endl;
			for (i = 0; i < lowerAddr.size(); i++) {
				cout << "        地址：" << lowerAddr[i] << "  UDP端口号: " << lowerPort[i] << endl;
			}
		}
		else {
			cout << endl;
			for (i = 0; i < lowerAddr.size(); i++) {
				cout << "        接口: " << i << " 地址" << lowerAddr[i] << "  UDP端口号: " << lowerPort[i] << endl;
			}
		}
	}
	cout << "统一管理平台地址: " << cmdAddr << "  UDP端口号: " << cmdPort <<endl;
	for(int i=1;i<=119;++i) printf("#");printf("\n");
	printArray();
	for(int i=1;i<=119;++i) printf("-");printf("\n");
	cout << endl;
}
void CCfgFileParms::print(WORD color)
{
	WORD colorOld;
	HANDLE handle = ::GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(handle, &csbi);
	colorOld = csbi.wAttributes;
	SetConsoleTextAttribute(handle, color);
	size_t i;
	cout << "设备号: " << deviceID << " 层次: " << layer << "实体: " << entityID << "  子网号: " << subnet<< endl;
	cout << "上层实体地址: " << upperAddr << "  UDP端口号: " << upperPort  <<endl;
	cout << "本层实体地址: " << localAddr << "  UDP端口号: " << localPort <<endl;
	if (layer.compare("PHY") == 0) {
		if ((int)lowerAddr.size() <= 1) {
			cout << "下层点到点信道" << endl;
			cout << "链路对端地址: ";
		}
		else {
			cout << "下层广播式信道" << endl;
			cout << "共享信道站点：";
		}
	}
	else {
		cout << "下层实体";
	}
	if ((int)lowerAddr.size() == 1) {
		cout << "地址：" << lowerAddr[0] << "  UDP端口号; " << lowerPort[0] << endl;
	}
	else {
		if (layer.compare("PHY") == 0) {
			cout << endl;
			for (i = 0; i < lowerAddr.size(); i++) {
				cout << "        地址：" << lowerAddr[i] << "  UDP端口号; " << lowerPort[i] << endl;
			}
		}
		else {
			cout << endl;
			for (i = 0; i < lowerAddr.size(); i++) {
				cout << "        接口: " << i << " 地址" << lowerAddr[i] << "  UDP端口号; " << lowerPort[i] << endl;
			}
		}
	}
	cout << "统一管理平台地址: " << cmdAddr << "  UDP端口号; " << cmdPort << endl;
	for(int i=1;i<=119;++i) printf("#");printf("\n");
//	printArray();
//	for(int i=1;i<=119;++i) printf("-");printf("\n");
	cout << endl;
	SetConsoleTextAttribute(handle, colorOld);
}
int CCfgFileParms::readArray()
{
	string* pstrTmp;
	string strInt;
	size_t i, j;
	int pos;
	struct parmEntry sParm;
	int begin, end;
	//每组有多少个参数

	for (i = 0; i < parmsSection.size(); i++) {
		pstrTmp = parmsSection[i];
		pos = (int)pstrTmp->find("=");
		if (pos < 0) {
			//不是正确的参数行，放弃
			continue;
		}
		sParm.name = pstrTmp->substr(0, pos); //前面有tab键和空格不管了，判断变量名的时候用find，不用compare
		begin = max((int)sParm.name.find_first_not_of(' ', 0), (int)sParm.name.find_first_not_of(9, 0));
		end = min((int)sParm.name.find_last_not_of(' '), (int)sParm.name.find_last_not_of(9));
		sParm.name = sParm.name.substr(begin, end - begin + 1);
		sParm.strValue = pstrTmp->substr(pos + 1);
		sParm.value = atoi(sParm.strValue.c_str());
		//检查之前有没有相同的参数，用后面的局部参数替代之前的全局参数
		for (j = 0; j < cfgParms.size(); j++) {
			if (sParm.name.compare(cfgParms[j].name) == 0) {
				break;
			}
		}
		if (j < cfgParms.size()) {
			//之前有全局参数，替代其值
			cfgParms[j].strValue = sParm.strValue;
			cfgParms[j].value = sParm.value;
		}
		else {
			//添加进表
			cfgParms.push_back(sParm);
		}
	}
	return 0;
}
//在结构字串中找到指定层次名和实体号的字串起始位置。返回值-1，表示本行没找到实体参数
int CCfgFileParms::findAddr(string* pStr, string* pLay, string* pEnt)
{
	string strTmp;
	int begin;

	//本行应该有参数（因为有空格），没有设备号，或者设备号匹配
	strTmp = pLay->c_str();
	strTmp += pEnt->c_str();
	begin = (int)pStr->find(strTmp, 0);
	//返回实体号在参数字符串中的位置
	return begin;
}
//获得在结构字串中，指定layer的层次号，在结构字串中，每行每层只有一个实体。同层多个实体要分行表示
//求层次的编号也就是获得层次位置，层次之间以空格间隔，没有设备号，要以空格开始
//返回值1表示最低层，-1表示没找到
int CCfgFileParms::getLayerNo(string* pStr, string* lay)
{
	string strTmp;
	int begin;
	int end;
	int i;
	//判断设备号情况,-2 表示设备号不匹配，没有设备号则可继续
	begin = (int)pStr->find(' ');
	if (begin == -1) {
		//整个字符串没有空格，格式不对,跳过
		return -1;
	}
	for (i = 1; begin < (int)pStr->length() && begin >= 0; i++) {
		//从begin开始找第一个非0的字段,跳过下一阶段空格
		begin = (int)pStr->find_first_not_of(' ', begin);
		if (begin == -1) {
			//跳过空格之后没东西
			break;
		}
		//截取子参数
		strTmp = pStr->substr(begin, 3);
		if ((int)strTmp.find(lay->c_str(), 0, 3) != -1) {
			//找到我们自己的层次了
			break;
		}
		end = (int)pStr->find_last_of(strTmp.c_str(), begin, 3);
		begin = (int)pStr->find_first_of(' ', end);
	}
	if (begin < 0) {
		//给出的层次名不对找不到
		return -1;
	}
	return i;
}
//从截取的实体描述字串的@后取出IP地址，如果没有@，则默认为回环地址
string CCfgFileParms::getAddr(string* str)
{
	int retval;
	string addr;
	// TODO: 在此处添加实现代码.
	retval = (int)str->find('@');
	if (retval == -1) {
		//使用默认参数
		addr = "127.0.0.1";
	}
	else {
		//取@后的IP地址
		addr = str->substr(retval + 1);
	}
	return addr;
}
//以dev，层次号，实体号生成UDP端口号，格式为十进制，1 dev1 layer1 enty2
string CCfgFileParms::getPort(string* dev, int iLayer, string* ent)
{
	string strTmp;
	string strLayer;
	//用设备号，层次，实体号生成唯一UDP端口号
	strTmp = "1";
	strTmp += dev->c_str();
	/*
	if (layer.compare("PHY") == 0) {
		strTmp += "1"; //层次号;
	}
	*/
	strLayer = to_string(iLayer);//层次号;

	strTmp += strLayer;

	if (ent->length() == 1) {
		//只有1位，补上0
		strTmp += "0";
	}
	strTmp += ent->c_str();

	return strTmp;
}
//从截取好的实体描述字段中，提取实体号
string CCfgFileParms::getEnt(string* str)
{
	string strEnt;
	int retval;
	int i;
	//要从str中找出实体号字段
	retval = (int)str->find("@");
	if (retval == -1) {
		//没有IP地址，最后1到两个字符是数字
		strEnt = str->c_str();
	}
	else {
		//取@之前
		strEnt = str->substr(0, retval);
	}
	for (i = (int)strEnt.length() - 1; i >= 0; i--) {
		if (strEnt.at(i) < '0' || strEnt.at(i) > '9')
			break;
	}
	strEnt = strEnt.substr(i + 1, strEnt.length() - i - 1);

	return strEnt;
}
//判断本行是否是参数行，依据是开头的#
bool CCfgFileParms::isParmsLine(string* pstr)
{
	// TODO: 在此处添加实现代码.
	if (pstr->size() == 0) {
		return false;
	}
	if (pstr->at(0) == '#' || isSplitLine(pstr)) {
		return false;
	}
	return true;
}
//判断本行是否为分割行，依据是连续的----
bool CCfgFileParms::isSplitLine(string* pstr)
{
	int retval;
	// TODO: 在此处添加实现代码.
	retval = (int)pstr->find("--------");
	if (retval < 0) {
		return false;
	}
	return true;
}

//在 pstr中比较设备号。返回值0，表示匹配，-1表示pstr中没有设备号，1表示pstr有设备号但是不匹配
int CCfgFileParms::compareDevID(string* pStr, string* dev)
{
	int end;
	string strTmp;
	//判断设备号情况,-2 表示设备号不匹配，没有设备号则可继续
	end = (int)pStr->find(' ');
	if (end == -1) {
		//整个字符串没有空格，格式不对,跳过
		return -1;
	}
	else if (end == 0) {
		//没有设备号字段
		return -1;
	}
	if (end > 1) {
		//设备号过大，暂时不支持
		return 1;
	}
	//取下第一个字段判断设备号
	strTmp = pStr->substr(0, end);
	if (strTmp.compare(dev->c_str()) != 0) {
		return 1;
	}
	return 0;
}
// 从描述字串中提取设备号，如果字串以空格开始，则提取不到。
string CCfgFileParms::getDev(string* pstr)
{
	int retval;
	// TODO: 在此处添加实现代码.
	retval = (int)pstr->find_first_of(" ");
	if (retval <= 0) {
		return string();
	}
	return pstr->substr(0, retval);
}

//用指定设备，层次，实体号的描述项在section中找到实体描述子，获得其地址和UDP端口号
//返回值：小于0表示各种错误，0表示成功
//输出：addr，地址，port，UDP端口号，indexOfDev，本设备描述的首行位置，indexOfThis，本实体所在行，iLayNo本层层次编号
int CCfgFileParms::getAddrFromSection(string* dev, string* lay, string* ent, string& addr, string& port, int& indexOfDev, int& indexOfThis, int& iLayNo)
{
	int deviceFind;
	size_t index;
	string* pstrTmp;
	int retval;
	int begin;
	int end;
	string strTmp;
	// 从archSection区域将指定设备，层次和实体的地址和UDP端口号提取出来
	deviceFind = 0;
	for (index = 0; index < archSection.size(); index++) {
		pstrTmp = archSection[index];
		//检查设备号
		retval = compareDevID(pstrTmp, dev);
		if (!deviceFind) {
			if (retval != 0) {
				continue;
			}
			else {
				deviceFind = 1;
				indexOfDev = (int)index;
			}
		}
		else {
			if (retval == 1) {
				break;
			}
		}
		//设备号匹配，或者位于设备号匹配的范围内，判断实体参数是否存在
		begin = findAddr(pstrTmp, lay, ent);
		if (begin == -1) {
			//不含指定实体参数，看下一条
			continue;
		}
		//找到实体，截取参数值
		indexOfThis = (int)index;
		end = (int)pstrTmp->find(' ', begin);
		if (end == -1) {
			strTmp = pstrTmp->substr(begin);
		}
		else {
			end = end - begin;
			strTmp = pstrTmp->substr(begin, end);
		}
		//从参数中获取IP地址，否则用默认的本地回环地址
		addr = getAddr(&strTmp);
		//通过层次名计算出层次号,1是第一层
		iLayNo = getLayerNo(pstrTmp, lay);
		if (iLayNo < 1) {
			//指定的layer名在配置文件中找不到
			return -3;
		}
		//生成UDP端口号
		port = getPort(dev, iLayNo, ent);
		break;
	}
	if (index == archSection.size()) {
		return -1;
	}
	return 0;
}
//功能：在指定的描述行中，取得某层实体的上层地址信息
//      方法是从本行找上层，没有则逐条上推寻找，直到最全的设备行
//输入：两个index，indexOfDev指明改设备描述在section的起始位置，indexOfThis表示该所在的描述行，iLay本层的层次编号，便于定位上下层
//返回值：-1表示没有更高层，0表示找到
//输出：addr中输出IP地址，port输出UDP端口号
int CCfgFileParms::getUpperAddr(size_t indexOfDev, size_t indexOfThis, int iLay, string& addr, string& port)
{
	string* pstrTmp;
	size_t index;
	string strTmp;
	string strEnt;
	string strDev;
	int begin;
	int end;
	int count;
	//得到设备号
	strDev = getDev(archSection[indexOfDev]);
	for (index = indexOfThis; (int)index >= (int)indexOfDev; index--) {
		pstrTmp = archSection[index];
		//跳过第一个空格间隔。
		begin = (int)pstrTmp->find_first_of(" ", 0);
		begin = (int)pstrTmp->find_first_not_of(" ", begin);

		end = begin;
		count = 0;
		while (end > 0) {
			count++;
			if (count == iLay + 1)
				break;
			end = (int)pstrTmp->find_first_of(" ", begin);
			if (end == -1)
				break;
			begin = (int)pstrTmp->find_first_not_of(" ", end);
			end = begin;
		}
		if (count < iLay + 1) {
			//没有找到高层
			continue;
		}
		else {
			//有高层，截取高层字段
			end = (int)pstrTmp->find_first_of(" ", begin);
			if (end > 0) {
				//有高层字段，截取
				strTmp = pstrTmp->substr(begin, end - begin);
			}
			else {
				strTmp = pstrTmp->substr(begin);
			}
			//获取高层参数值
			addr = getAddr(&strTmp);
			strEnt = getEnt(&strTmp);
			port = getPort(&strDev, iLay + 1, &strEnt);
			//port = getPort(&strTmp, deviceID, iLayer + 1);
			break;
		}
	}
	if ((int)index < (int)indexOfDev) {
		//没找到
		return -1;
	}
	return 0;
}
// 从链路字串的设定范围内，根据逗号，取出设备号和实体号，start包含在范围内，end不包含在范围内！
int CCfgFileParms::getDevEntFromLink(string* pstr, int start, int end, string& dev, string& port)
{
	int com;
	// TODO: 在此处添加实现代码.
	com = (int)pstr->find_first_of(",", start);
	if (com == -1) {
		return -6;//链路格式错误，缺少 --
	}
	dev = pstr->substr(start, com - start);
	port = pstr->substr(com + 1, end - com - 1);
	return 0;
}

//功能：在指定的描述行中，取得某层实体的下层地址信息
//      方法是就在本行找低层，位于前面一个,同时还要注意，需要把本层有关的低层都读进来。
//输入：两个index，indexOfDev指明改设备描述在section的起始位置，获取设备号，indexOfThis表示该所在的描述行，iLay本层的层次编号，便于定位上下层
//返回值：-1表示没有低层，0表示找到
//输出：addr中输出IP地址，port输出UDP端口号
int CCfgFileParms::getLowerAddr(size_t indexOfDev, size_t indexOfThis, int iLay, vector<string>& addr, vector<string>& port)
{
	string* pstrTmp;
	size_t index;
	string strTmp;
	string strEnt;
	string strDev;
	int begin;
	int end;
	int count;
	int retval;

	strDev = getDev(archSection[indexOfDev]);
	//找到低层实体参数，
	if (iLay > 1) {
		//不是最低层，前面才会有实体，只取前一个实体地址外，还要依次向下找全相关联的所有低层实体
		strDev = getDev(archSection[indexOfDev]);
		for (index = indexOfThis; index < archSection.size(); index++) {
			pstrTmp = archSection[index];
			//设备号不匹配就直接跳出
			retval = compareDevID(pstrTmp, &strDev);
			if (retval == 1) {
				return 0;
			}
			//跳过第一个空格间隔。
			begin = (int)pstrTmp->find_first_of(" ", 0);
			begin = (int)pstrTmp->find_first_not_of(" ", begin);

			end = begin;
			count = 0;
			while (end > 0) {
				count++;
				if (count == iLay - 1)
					break;
				end = (int)pstrTmp->find_first_of(" ", begin);
				if (end == -1)
					break;
				begin = (int)pstrTmp->find_first_not_of(" ", end);
				end = begin;
			}
			if (count < iLay - 1) {
				//低层不够数量，继续
				continue;
			}
			else {
				//有低层，但是还要看同层是否有，如果同层且，不一样的，则结束寻找
				if (index != indexOfThis) {
					//换行了
					end = (int)pstrTmp->find_first_of(" ", begin);
					if (end > 0) {
						end = (int)pstrTmp->find_first_not_of(" ", end);
						if (end > 0) {
							//有同层，结束
							return 0;
						}
					}
				}
			}
			//有合适低层，截取字段
			end = (int)pstrTmp->find_first_of(" ", begin);
			if (end > 0) {
				//有低层字段，截取
				strTmp = pstrTmp->substr(begin, end - begin);
			}
			else {
				strTmp = pstrTmp->substr(begin);
			}
			//获取低层参数值
			addr.push_back(getAddr(&strTmp));
			strEnt = getEnt(&strTmp);
			port.push_back(getPort(&strDev, iLay - 1, &strEnt));
		}

	}
	else {
		int mybegin;
		string strAddrTmp;
		string strPortTmp;
		string strPeerDev;
		string strPeerEnt;//layer一定是物理层
		int x, y, z;
		//本层是物理层实体，从链路找到对端地址
		strTmp = deviceID;//getDev(archSection[indexOfDev]);
		strTmp += ",";
		strTmp += entityID; //直接来了哈，本来应该从Layer数到，然后取得，太麻烦


		for (index = 0; index < linksSection.size(); index++) {
			pstrTmp = linksSection[index];
			mybegin = (int)pstrTmp->find(strTmp);
			if (mybegin == -1) {
				continue;
			}
			//找到包含本实体的链路描述，有且仅有一行
			//一段一段切出来，找到对端地址，存放
			begin = 0;
			end = 0;
			end = (int)pstrTmp->find("--", begin);
			while (begin < (int)pstrTmp->length()) {
				if (begin != mybegin) {
					//取得对等实体设备号和实体号
					retval = getDevEntFromLink(pstrTmp, begin, end, strPeerDev, strPeerEnt);
					if (retval < 0) {
						return retval;
					}
					//在archSection中按设备号，层次名，实体号，取出地址
					retval = getAddrFromSection(&strPeerDev, &layer, &strPeerEnt, strAddrTmp, strPortTmp, x, y, z);
					if (retval < 0) {
						return -7;
					}
					lowerAddr.push_back(strAddrTmp);
					lowerPort.push_back(strPortTmp);
				}
				//确定下一个字段的位置
				begin = end + 2;
				end = (int)pstrTmp->find("--", begin);
				if (end == -1)
					end = (int)pstrTmp->length();
			}
			break;
		}
		if (index >= linksSection.size()) {
			//没找到，网元无法接入网络
			return -5;
		}
	}
	return 0;
}

//-1：配置文件不存在，-2：配置文件中没有有效参数行;-3 在该设备的结构描述行中没找到指定实体
//-4: 结构参数中，不应该是最低层的前面没有低层实体参数；-5：链路区域找不到本实体
//-6：链路描述行格式有错，没找到--,-7：链路层对端没有在结构区域中描述
//-8: 拓扑中少于两个节点
int CCfgFileParms::read()
{
	string strTmp;
	string strTmp2;
	string* pstrTmp; //每一行字串指针
	//string strInt;
	bool isStart = false;
	int deviceFind = 0;
	int layerFind = 0;
	int entityFind = 0;
	int retval;
	size_t index = 0;
	int iLayer;
	int indexDev;
	int indexThis;
	struct parmEntry sParmEntry;

	ifstream cfgFile("ne.txt");

	if (!cfgFile.is_open())
	{
		isConfigExist = false;
		return -1;
	}

	//做个内容清零操作
	cleanSections();
	cfgParms.clear();
	//将文件内容分区全部读出
	//先跳过第一个分区线之前的
	while (!cfgFile.eof()) {
		getline(cfgFile, strTmp);
		if (isSplitLine(&strTmp)) {
			break;
		}
	}
	if (cfgFile.eof()) {
		//没有读到有效内容
		isConfigExist = 0;
		cfgFile.close();
		return -2;
	}
	//读入架构和地址区域内容
	while (!cfgFile.eof()) {
		getline(cfgFile, strTmp);
		if (isSplitLine(&strTmp)) {
			break;
		}
		if (!isParmsLine(&strTmp)) {
			continue;
		}
		pstrTmp = new string(strTmp.c_str());
		archSection.push_back(pstrTmp);
	}
	//读入链路区域内容
	while (!cfgFile.eof()) {
		getline(cfgFile, strTmp);
		if (isSplitLine(&strTmp)) {
			break;
		}
		if (!isParmsLine(&strTmp)) {
			continue;
		}
		pstrTmp = new string(strTmp.c_str());
		linksSection.push_back(pstrTmp);
	}
	//读入统一平台区域内容
	while (!cfgFile.eof()) {
		getline(cfgFile, strTmp);
		if (isSplitLine(&strTmp)) {
			break;
		}
		if (!isParmsLine(&strTmp)) {
			continue;
		}
		retval = (int)strTmp.find("cmdIpAddr");
		if (retval >= 0) {
			cmdAddr = strTmp.substr(strTmp.find("=") + 1);
		}
		else {
			retval = (int)strTmp.find("cmdPort");
			if (retval >= 0) {
				cmdPort = strTmp.substr((int)strTmp.find("=") + 1);
			}
			else {
				//全局参数，存入参数表
				pstrTmp = new string(strTmp.c_str());
				parmsSection.push_back(pstrTmp);
			}
		}
	}
	//读入其他参数区域内容，只读本实体有关参数
	deviceFind = 0;
	layerFind = 0;
	entityFind = 0;
	while (!cfgFile.eof()) {
		getline(cfgFile, strTmp);
		if (!isParmsLine(&strTmp)) {
			continue;
		}
		if (isSplitLine(&strTmp)) {
			//分割线，看状态
			if (entityFind) {
				//该结束了
				break;
			}
		}
		//从deviceID开始
		if (!deviceFind) {
			retval = (int)strTmp.find("deviceID");
			if (retval == -1) {
				continue;
			}
			retval = (int)strTmp.find(deviceID);
			if (retval == -1) {
				continue;
			}
			//找到设备
			deviceFind = 1;
			continue;
		}
		else {
			retval = (int)strTmp.find("deviceID");
			if (retval >= 0) {
				//在已经发现过deviceID的情况下，又读到deviceID行，这是新的deviceID了，退出
				break;
			}
		}
		if (!layerFind) {
			retval = (int)strTmp.find("layer");
			if (retval == -1) {
				continue;
			}
			retval = (int)strTmp.find(layer);
			if (retval == -1) {
				continue;
			}
			//找到层次
			layerFind = 1;
			continue;
		}
		else {
			retval = (int)strTmp.find("layer");
			if (retval >= 0) {
				//在已经发现过layer的情况下，又读到layer行，这是新的layer了，退出
				break;
			}

		}
		if (!entityFind) {
			retval = (int)strTmp.find("entityID");
			if (retval == -1) {
				continue;
			}
			retval = (int)strTmp.find(entityID);
			if (retval == -1) {
				continue;
			}
			//找到实体
			entityFind = 1;
			continue;
		}
		else {
			retval = (int)strTmp.find("entityID");
			if (retval >= 0 || isSplitLine(&strTmp)) {
				//在已经发现过entieyID的情况下，又读到entieyID行或者分割行，这是新的entieyID了，退出
				break;
			}

		}

		//放入参数表，
		pstrTmp = new string(strTmp.c_str());
		parmsSection.push_back(pstrTmp);
	}

	cfgFile.close();

	//在架构区域中找到本实体的IP地址和UDP端口号字串
	retval = getAddrFromSection(&deviceID, &layer, &entityID, localAddr, localPort, indexDev, indexThis, iLayer);
	if (retval < 0) {
		isConfigExist = false;
		return retval;
	}
	//获得低层地址
	retval = getLowerAddr(indexDev, indexThis, iLayer, lowerAddr, lowerPort);
	if (retval < 0) {
		//不能没有低层，PHY的低层就是链路对端
		return retval;
	}
	//获得高层地址
	retval = getUpperAddr(indexDev, indexThis, iLayer, upperAddr, upperPort);

	//判断有多少个网元，以获得布局layout,构造用参数的形式放入到参数表中
	strTmp = "z"; //比较设备号时不匹配

	sParmEntry.value = 0;
	for (index = 0; index < archSection.size(); index++) {
		pstrTmp = archSection[index];
		retval = compareDevID(pstrTmp, &strTmp);
		if (retval == 1) {
			sParmEntry.value++;
		}
	}
	if (sParmEntry.value >= 2) {
		sParmEntry.name = "layOut";
		sParmEntry.strValue = std::to_string(sParmEntry.value);
	}
	else {
		return -8;
	}

	cfgParms.push_back(sParmEntry);

	//整理其他参数集合，拆分成变量名和变量值结构
	readArray();

	isConfigExist = true;

	return 0;
}

int CCfgFileParms::cleanSections()
{
	string* pstrTmp;
	// TODO: 在此处添加实现代码.
	while (!archSection.empty()) {
		pstrTmp = archSection[archSection.size() - 1];
		archSection.pop_back();  //最尾巴弹出
		delete pstrTmp;
	}
	while (!linksSection.empty()) {
		pstrTmp = linksSection[linksSection.size() - 1];
		linksSection.pop_back();
		delete pstrTmp;
	}
	while (!parmsSection.empty()) {
		pstrTmp = parmsSection[parmsSection.size() - 1];
		parmsSection.pop_back();
		delete pstrTmp;
	}

	return 0;
}

CCfgFileParms::~CCfgFileParms()
{
	//释放文件内容
	cleanSections();

}
//根据设备号，层次号，实体号，按16进制格式生成UDP端口号,作为返回值，
//规则:最高位为0，次高位为1，最左为bit0，b0~b3——类型，b4~b7——设备号，b8~b11——层次号，b12~b15——实体号
unsigned short CCfgFileParms::createHexPort(int inID, int inLayer, int inEntity, int inType)
{
	// TODO: 在此处添加实现代码.
	unsigned short port;
	port = 0;
	//组装实体号
	port = (inEntity & 0xf) | port;
	//组装层次号
	port = ((inLayer & 0xf) << 4) | port;
	//组装设备号
	port = ((inID & 0xf) << 8) | port;
	//组装类型号，type只有两种，普通为0，命令为1
	port = ((inType & 0xf) << 12) | port;
	//最高位设位0，次高位设为1
	port = (port & 0x3fff) | 0x4000;

	return port;
}
//获得指定type的套接字地址
//输入：type， -1表示低层，0表示本层，1表示高层
sockaddr_in CCfgFileParms::getUDPAddr(AddrType type, int index)
{
	sockaddr_in addr;
	addr.sin_family = 0;
	addr.sin_addr.S_un.S_addr = 0;
	addr.sin_port = 0;
	// TODO: 在此处添加实现代码.
	switch (type) {
	case LOWER:
		if (lowerAddr.size() == 0 || index >= (int)lowerAddr.size()) {
			break;
		}
		addr.sin_family = AF_INET;
		addr.sin_addr.S_un.S_addr = myInetAddr2n(&(lowerAddr[index]));
		addr.sin_port = htons(atoi(lowerPort[index].c_str()));
		break;
	case LOCAL:
		addr.sin_family = AF_INET;
		addr.sin_addr.S_un.S_addr = myInetAddr2n(&localAddr);
		addr.sin_port = htons(atoi(localPort.c_str()));

		break;
	case UPPER:
		addr.sin_family = AF_INET;
		addr.sin_addr.S_un.S_addr = myInetAddr2n(&upperAddr);
		addr.sin_port = htons(atoi(upperPort.c_str()));
		break;
	case CMDER:
		addr.sin_family = AF_INET;
		addr.sin_addr.S_un.S_addr = myInetAddr2n(&cmdAddr);
		addr.sin_port = htons(atoi(cmdPort.c_str()));
		break;
	}
	return addr;
}
string CCfgFileParms::getUDPAddrString(AddrType type, int index)
{
	string strTmp;
	// TODO: 在此处添加实现代码.
	switch (type) {
	case LOWER:
		if (lowerAddr.size() == 0 || index >= (int)lowerAddr.size()) {
			break;
		}
		strTmp = lowerAddr[index].c_str();
		strTmp += ":";
		strTmp += lowerPort[index].c_str();
		break;
	case LOCAL:
		strTmp = localAddr.c_str();
		strTmp += ":";
		strTmp += localPort.c_str();
		break;
	case UPPER:
		strTmp = upperAddr.c_str();
		strTmp += ":";
		strTmp += upperPort.c_str();
		break;
	}
	return strTmp;
}
//获得一组地址的数量，主要是低层接口的数量，
//输入：type， -1表示低层，0表示本层，1表示高层
int CCfgFileParms::getUDPAddrNumber(AddrType type)
{
	// TODO: 在此处添加实现代码.
	switch (type) {
	case LOWER:
		return (int)lowerAddr.size();
	}
	return 1;
}
//自编的IP地址字符串转换为网络字节序整数形式
unsigned long CCfgFileParms::myInetAddr2n(string* pstr)
{
	int begin;
	int end;
	unsigned long retval;
	unsigned long temp;
	string strTmp;
	// TODO: 在此处添加实现代码.
	retval = 0;
	begin = (int)pstr->find_first_of(".");
	if (begin < 0)
		return 0;
	strTmp = pstr->substr(0, begin);
	temp = atoi(strTmp.c_str());
	retval = (temp << 24) & 0xff000000;

	begin = begin + 1;
	end = (int)pstr->find_first_of(".", begin);
	if (end < 0) {
		return 0;
	}
	strTmp = pstr->substr(begin, end - begin);
	temp = atoi(strTmp.c_str());
	retval += (temp << 16) & 0x00ff0000;

	begin = end + 1;
	end = (int)pstr->find_first_of(".", begin);
	if (end < 0) {
		return 0;
	}
	strTmp = pstr->substr(begin, end - begin);
	temp = atoi(strTmp.c_str());
	retval += (temp << 8) & 0x0000ff00;

	begin = end + 1;
	strTmp = pstr->substr(begin);
	temp = atoi(strTmp.c_str());
	retval += temp & 0x000000ff;

	return htonl(retval);
}

//获得链路在拓扑文件中的编号，用来统一调整颜色，相同链路上的界面都是一个颜色
//仅物理层使用
int CCfgFileParms::getLinkIndex()
{
	string strTmp;
	string* pstrTmp;
	size_t index;
	int retval;
	// TODO: 在此处添加实现代码.
	if (layer.compare("PHY") != 0) {
		return -1;
	}
	//本层是物理层实体，从链路找到对端地址
	strTmp = deviceID;//getDev(archSection[indexOfDev]);
	strTmp += ",";
	strTmp += entityID; //直接来了哈，本来应该从Layer数到，然后取得，太麻烦


	for (index = 0; index < linksSection.size(); index++) {
		pstrTmp = linksSection[index];
		retval = (int)pstrTmp->find(strTmp);
		if (retval >= 0) {
			break;
		}
	}
	if (index >= linksSection.size()) {
		return -1;
	}
	else
		return (int)index;
}

string CCfgFileParms::getDeviceID()
{
	// TODO: 在此处添加实现代码.
	return deviceID;
}


string CCfgFileParms::getLayer()
{
	// TODO: 在此处添加实现代码.
	return layer;
}


string CCfgFileParms::getEntity()
{
	// TODO: 在此处添加实现代码.
	return entityID;
}
