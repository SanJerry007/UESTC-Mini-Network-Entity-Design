//一键启动
#include <iostream>
#include <string>
#include <fstream>
#include <atlstr.h>
#include <vector>
#include <windows.h>
using namespace std;

struct functionMap_t {
	string LayerName;
	string funcitonName;
	int len; //层次名字字符串的长度
}aFunctionMap[7];

//倒计时关闭
void shutdown() {
	for (int i = 5; i >= 1; --i) {
		cout << "该启动器将在" << i << "s后自动关闭\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b";
		Sleep(1000);
	}
}

string getFunction(string strName) {
	int i;
	for (i = 0; i < 7; i++) {
		if (aFunctionMap[i].LayerName.empty() || strName.empty())
			continue;
		if (strName.compare(aFunctionMap[i].LayerName) == 0)//找到
			return aFunctionMap[i].funcitonName;
	}
	return "";//没找到 
}

//只保留ASCII码大于32的字符，32为空格，以下的都是控制字符
void myStrcpy(string& dst, string& src) {
	size_t i;
	const char* cpSrc;
	dst.clear();
	if (src.empty()) return dst.clear();
	cpSrc = src.c_str();
	for (i = 0; i < strlen(cpSrc); i++)
		if (cpSrc[i] > 32 || cpSrc[i] < 0)
			dst.append(1, cpSrc[i]);
}

void readMapFile(ifstream& f) {
	string strTmp;
	string csLeft;
	string csRight;
	string csLayer;
	string csFunc;
	int i = 0;
	
	while (!f.eof()) {
		getline(f, strTmp);
		if (strTmp.empty() || strTmp.find("=") < 0) continue;
		csLeft = strTmp.substr(0, strTmp.find("=") - 1);
		csRight = strTmp.substr(strTmp.find("=") + 1, strTmp.length() - strTmp.find("="));
		myStrcpy(csLayer, csLeft);
		myStrcpy(csFunc, csRight);
		aFunctionMap[i].LayerName = csLayer;
		aFunctionMap[i].funcitonName = csFunc;
		i++;
	}
}

//判断本行是否为分割行，依据是连续的----
bool isSplitLine(string* pstr) {
	int retval;
	retval = (int)pstr->find("-------");
	if (retval < 0) return 0;
	return 1;
}

//判断本行是否是参数行，依据是开头的#
bool isParmsLine(string* pstr) {
	if (pstr->at(0) == '#' || isSplitLine(pstr)) return 0;
	return 1;
}

//从描述字串中提取设备号，如果字串以空格开始，则提取不到。
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
	
	retval = (int)str->find("@");//从str中找出实体号字段
	if (retval == -1) strEnt = str->c_str();//没有IP地址，最后1到两个字符是数字
	else strEnt = str->substr(0, retval);//取@之前
	
	for (i = (int)strEnt.length() - 1; i >= 0; i--)
		if (strEnt.at(i) < '0' || strEnt.at(i) > '9')
			break;
	
	strEnt = strEnt.substr(i + 1, strEnt.length() - i - 1);
	strLayer = str->substr(0, i + 1);
	return 0;
}

int main() {
	ifstream mapFile("map.txt");//打开映射文件
	if (!mapFile.is_open()) {
		cout << "ERROR: 没有找到map.txt文件" << endl << endl;
		shutdown();
		return 0;
	}
	cout << "读取map.txt文件成功" << endl;
	readMapFile(mapFile);
	mapFile.close();

	ifstream cfgFile("ne.txt");//打开配置文件
	if (!cfgFile.is_open()) {
		cout << endl << "ERROR: 没有找到ne.txt文件" << endl << endl;
		shutdown();
		return 0;
	}
	cout << "读取ne.txt文件成功" << endl;
	
	string* pstrTmp;
	string strTmp;
	vector <string*> archSection;
	while (!cfgFile.eof()) {//遍历配置文件
		getline(cfgFile, strTmp);
		if (isSplitLine(&strTmp)) break;
	}
	if (cfgFile.eof()) {//没有读到有效内容
		cfgFile.close();
		cout << endl << "ERROR: ne.txt文件缺失有效内容" << endl << endl;
		shutdown();
		return -2;
	}
	while (!cfgFile.eof()) {//读入架构和地址区域内容
		getline(cfgFile, strTmp);
		if (isSplitLine(&strTmp)) break;
		if (!isParmsLine(&strTmp)) continue;
		pstrTmp = new string(strTmp.c_str());
		archSection.push_back(pstrTmp);
	}
	cfgFile.close();
	if (archSection.size() == 0) {//判定架构和地址区域内容
		cout << endl << "ERROR: ne.txt文件缺失构和地址区域内容" << endl << endl;
		shutdown();
		return 0;
	}
	cout << "读取配置成功" << endl;
	
	int NECount = 0;
	size_t index;
	string strDev;
	string strLayer;
	string strEntity;
	string strCmd;
	string strParm;
	int begin;
	int end;
	
	for (index = 0; index < archSection.size(); index++) {
		pstrTmp = archSection[index];//读出一个，执行一个
		begin = 0;//读设备号
		end = (int)pstrTmp->find_first_of(" ");
		if (end < 0) continue;//这一行不对
		else if (end > 0) strDev = pstrTmp->substr(0, end);//如果有设备号，就提取，否则用之前的
		begin = end;
		
		while (begin < (int)pstrTmp->length() && begin >= 0) {
			begin = (int)pstrTmp->find_first_not_of(' ', begin);//跳过空格
			if (begin == -1) break;
			end = (int)pstrTmp->find_first_of(' ', begin);//截取
			if (end == -1) end = (int)pstrTmp->length();//后面没有了

			strTmp = pstrTmp->substr(begin, end - begin);
			getLayerAndEnt(&strTmp, strLayer, strEntity);
			strCmd = getFunction(strLayer);
			strParm = strDev;
			strParm += " ";
			strParm += strLayer;
			strParm += " ";
			strParm += strEntity;

			ShellExecute(NULL, _T("open"), strCmd.c_str(), strParm.c_str(), NULL, SW_MINIMIZE);
			cout << "启动网元 " << strDev << " 的 " << strLayer << " 层实体 " << strEntity << endl;
			NECount++;
			begin = end;
		}
	}
	cout << "读出网元 " << NECount << " 个" << endl << endl << "所有文件已打开" << endl << endl;
	shutdown();
	return 0;
}
