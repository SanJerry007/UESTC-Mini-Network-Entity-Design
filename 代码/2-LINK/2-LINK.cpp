//2 Data Link Layer(链路层)
#include<iostream>
#include<cstdio>
#include<cstdlib>
#include<cmath>
#include<queue>
#include<winsock2.h>
#include<windows.h>
#include"CfgFileParms.h"
#include"BasicFunction.h"
//#pragma comment(lib,"ws2_32.lib")
using namespace std;

CCfgFileParms Link_file;               //网元信息
const int MAX_BUFFER_SIZE=65000;       //缓存空间(bit)
const int HEAD_SIZE=sizeof(HEAD);      //头部长度(字节)
int SOCKADDR_SIZE=sizeof(SOCKADDR_IN); //套接字地址长度(字节)

WSADATA     Local_data;    //本层初始化信息
SOCKET      Local_sock;    //本层套接字
SOCKADDR_IN Local_addr;    //本层套接字地址
SOCKADDR_IN Upper_addr;    //上层套接字地址(网络层)
SOCKADDR_IN Lower_addr[8]; //下层套接字地址(物理层)
SOCKADDR_IN Cmd_addr;      //管理层套接字地址

int Top=0;                  //是否为顶层，即是否为交换机
int Sub_net=0;              //本层虚拟子网号
int Local_ip;               //本层真实IP
int Local_port;             //本层真实端口
int Lower_mac_ip[8][128];   //下层来源MAC的真实IP             (地址表)
int Lower_mac_port[8][128]; //下层来源MAC的真实端口           (地址表)
int Lower_mac_num[8];       //已学习的下层MAC地址数 (最大128) (地址表相关)

int Print_mode=0;   //本层收发信息打印模式，0为不打印收发的内容，1为打印bit流，2为打印8进制 (默认1)
int F_Print_mode=0; //本层帧处理信息打印模式，0为不打印帧的处理信息，1为打印 (默认0)
int T_Print_mode=0; //本层提示信息打印模式，0为只打印发生错误时的提示信息，1增加打印收发处理信息，2增加打印流量控制信息 (默认0)
int A_Print_mode=0; //本层地址相关信息打印模式，0为不打印交换信息，1为打印 (默认0)
int Upper_mode=1;   //上层接口模式，0为bit数组，1为字节数组 (默认1)
int Lower_mode[8];  //下层接口模式，0为bit数组，1为字节数组 (默认0)
int Lower_number=1; //下层实体数量 (默认1)(最大为8)

int Upper_full=25;         //上层拥堵指数 (默认25)
int Lower_full=25;         //同层拥堵指数 (默认25)
int Recv_timeout=20;       //接收超时(ms) (默认20)(作为整个程序运行的时间单位)(重要)
int R_Send_timeout=200;    //重传超时(ms) (默认200)
int Upper_Send_timeout=20; //上层发送超时(ms) (默认20)
int Lower_Send_timeout=20; //下层发送超时(ms) (默认20)

int Recv_buf;           //已用接收缓存(bit)(从下层接收)
int Send_buf;           //已用发送缓存(bit)(向下层发送)
char *Tmp_buf;          //临时缓存(bit/字节)
queue<char*>Recv_queue; //接收序列
queue<char*>Send_queue; //发送序列
queue<INFO>Recv_info;   //接收帧的信息
queue<INFO>Send_info;   //发送帧的信息
DATA Recv_amount;       //接收数据量
DATA Recv_time;         //接收次数
DATA Send_amount;       //发送数据量
DATA Send_time;         //发送次数

int Divide_len=100;        //分帧的帧长度 (bit)(默认100)(最大为1000)
int Divide_up_thershold=5; //使分帧长度增加的，未发生错误的周期数量阈值 

int Send_seq[128];            //发送的帧序号
int Send_window_size=10;      //发送窗口大小 (默认10)(最大为100)
bool Send_window_use[100];    //发送窗口是否被占用
int Send_window_wait[100];    //发送窗口的等待时间 
int Send_window_busy_check=3; //发送窗口的繁忙判定 (默认重传次数为3判定为繁忙)
bool Send_window_busy[100];   //发送窗口是否繁忙 
int Send_window_busy_num;     //发送窗口的繁忙数量 
int Send_window_stop_check=5; //发送窗口的暂停判定 (默认重传次数为5暂停重传)
bool Send_window_stop[100];   //发送窗口是否暂停
int Send_window_stop_num;     //发送窗口的暂停数量 

char *Send_window_frame[100]; //发送窗口的帧
INFO Send_window_info[100];   //发送窗口的帧信息
char *Special_window_frame;   //特殊发送窗口的帧 
INFO Special_window_info;     //特殊发送窗口的帧信息 
char *Recv_tmp_data;          //待向上发送的数据
INFO Recv_tmp_info;           //待向上发送的数据信息

int Recv_buf_high_thershold=0.4*MAX_BUFFER_SIZE; //接收缓存较满判定阈值
int Recv_buf_low_thershold=0.25*MAX_BUFFER_SIZE; //接收缓存较空判定阈值
int Send_buf_high_thershold=0.4*MAX_BUFFER_SIZE; //发送缓存较满判定阈值
int Send_buf_low_thershold=0.25*MAX_BUFFER_SIZE; //发送缓存较空判定阈值
int Upper_full_thershold=25; //上层拥堵指数较大判定阈值
int Lower_full_thershold=25; //同层拥堵指数较大判定阈值

int Upper_send_stop=0;    //向上发送暂停周期数
int Lower_send_stop=0;    //向下发送暂停周期数
bool Upper_send_data=0;   //是否向上层发送了数据信息
bool Lower_send_data=0;   //是否向下层发送了数据信息
bool Upper_recv_data=0;   //是否从上层接收了数据信息
bool Lower_recv_data=0;   //是否从下层接收了数据信息
bool Recv_buf_no_empty=0; //上个周期接收缓存是否不空
bool Send_buf_no_empty=0; //上个周期发送缓存是否不空
bool ACK_add_clear=0;     //是否在ACK中添加清零控制信息

bool Recv_data_time[128][4194304]; //接收到的数据帧序号情况统计 (最大序号4194304-1)

void shutdown(){//退出
	WSACleanup();
	if(Tmp_buf) delete [] Tmp_buf;
	if(Special_window_frame) delete [] Special_window_frame;
	if(Recv_tmp_data) delete [] Recv_tmp_data;
	for(int i=0;i<=99;++i){
		if(Send_window_frame[i])
		delete [] Send_window_frame[i];
	}
}
void tip(int type,int num){//提示
	if(type==0&&num==0){
		cprintf("#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#",62);
		cprintf("        链        路        层        ",62);
		cprintf("#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#\n",62);
		return;
	}
	switch(type){
		case 0:{//成功
			switch(num){
				case 1:{cprintf("*层次信息配置成功\n",2);break;}
				case 2:{cprintf("*缓存空间分配成功\n",2);break;}
				case 3:{cprintf("*套接字环境初始化成功\n",2);break;}
				case 4:{cprintf("*套接字配置读取成功\n",2);break;}
				case 5:{cprintf("*本层套接字初始化并绑定成功\n",2);break;}
				case 6:{cprintf("*其他层套接字初始化成功\n",2);break;}
				case 7:{cprintf("*套接字设置超时成功\n",2);break;}
				case 8:{cprintf("*套接字设置非阻塞成功\n",2);break;}
			}
			break;
		}
		case 1:{//失败
			switch(num){
				case 1:{cprintf("*层次信息配置失败\n",4);break;}
				case 2:{cprintf("*缓存空间分配失败\n",4);break;}
				case 3:{cprintf("*套接字环境初始化失败\n",4);break;}
				case 4:{cprintf("*套接字配置读取失败\n",4);break;}
				case 5:{cprintf("*本层套接字绑定失败\n",4);break;}
				case 6:{cprintf("*下层套接字参数错误\n",4);break;}
				case 7:{cprintf("*套接字设置超时失败\n",4);break;}
				case 8:{cprintf("*套接字设置非阻塞失败\n",4);break;}
			}
			shutdown();
			system("@ pause");
			exit(0);
		}
		default:{//其他情况
			switch(num){
				case 1:{cprintf("*对方套接字已关闭",3);
						print_void(102);printf("\n");break;}
				case 2:{cprintf("*套接字发生异常",3);
						print_void(104);printf("\n");break;}
				case 3:{cprintf("*发送：帧长度超过缓存上限，成帧失败",3);
						print_void(84);printf("\n");break;}
				case 4:{cprintf("*发送：收到数据，已保存至发送缓存",3);
						print_void(86);printf("\n");break;}//T_Print_mode
				case 5:{cprintf("*发送：发送缓存达到上限，已丢弃即将发送的帧",3);
						print_void(76);printf("\n");break;}
				case 6:{cprintf("*发送：发送队列与缓存空间不匹配，已重置发送队列与缓存",3);
						print_void(66);printf("\n");break;}
				case 7:{cprintf("*接收：帧长度超过缓存上限，恢复失败",3);
						print_void(84);printf("\n");break;}
				case 8:{cprintf("*接收：帧误且无法纠正，且帧长度过短，判定为ACK或NAK，已丢弃",3);
						print_void(60);printf("\n");break;}//T_Print_mode
				case 9:{cprintf("*接收：帧误且无法纠正，但帧长度较长，判定为数据帧，请求重传",3);
						print_void(60);printf("\n");break;}//T_Print_mode
				case 10:{cprintf("*接收：收到ACK",3);
						print_void(105);printf("\n");break;}//T_Print_mode
				case 11:{cprintf("*接收：收到NAK",3);
						print_void(105);printf("\n");break;}//T_Print_mode
				case 12:{cprintf("*接收：收到数据帧，帧序号重复，已丢弃数据帧，并添加ACK至发送缓存",3);
						print_void(55);printf("\n");break;}//T_Print_mode
				case 13:{cprintf("*接收：收到数据帧，已保存至接收缓存，并添加ACK至发送缓存",3);
						print_void(63);printf("\n");break;}//T_Print_mode
				case 14:{cprintf("*接收：接收缓存达到上限，已丢弃接收的帧",3);
						print_void(80);printf("\n");break;}
				case 15:{cprintf("*接收：接收队列与缓存空间不匹配，已重置接收队列与缓存",3);
						print_void(66);printf("\n");break;}
				case 16:{cprintf("*内存分配：失败！",12);
						print_void(102);printf("\n");break;}
			}
		}
	}
}
void initial(int argc,char *argv[]){//初始化 
	//层次信息
	string s1,s2,s3;
	switch(argc){
		case 4:{
			s1=argv[1];
			s2=argv[2];
			s3=argv[3];
			break;
		}
		case 3:{
			s1=argv[1];
			s2="LINK";
			s3=argv[3];
			break;
		}
		default:tip(1,1);
	}
	CCfgFileParms File(s1,s2,s3);
	Link_file=File;
	Link_file.getValueInt(Top,(char*)"Top"); //是否为顶层
	Link_file.getValueInt(Sub_net,(char*)"Sub_net");//子网号
	Link_file.getValueInt(Print_mode,(char*)"Print_mode");    //收发信息打印模式
	Link_file.getValueInt(F_Print_mode,(char*)"F_Print_mode");//帧处理信息打印模式
	Link_file.getValueInt(T_Print_mode,(char*)"T_Print_mode");//提示信息打印模式
	Link_file.getValueInt(A_Print_mode,(char*)"A_Print_mode");//地址信息打印模式
	Link_file.getValueInt(Recv_timeout,(char*)"Recv_timeout");    //接收超时
	Link_file.getValueInt(R_Send_timeout,(char*)"R_Send_timeout");//重传超时
	Link_file.getValueInt(Upper_Send_timeout,(char*)"Upper_Send_timeout");//上层发送超时
	Link_file.getValueInt(Lower_Send_timeout,(char*)"Lower_Send_timeout");//下层发送超时
	Link_file.getValueInt(Send_window_size,(char*)"Send_window_size");//发送窗口数量
	Link_file.getValueInt(Divide_len,(char*)"Divide_len");            //分帧长度
	//下层实体数量
	Lower_number=Link_file.getUDPAddrNumber(CCfgFileParms::LOWER);
	if(Lower_number<0) tip(1,1);
	//读取本层IP与端口信息
	HEAD tmp_h=NULL_HEAD;
	string info=Link_file.getUDPAddrString(CCfgFileParms::LOCAL,0);//本层
	String2Num(&Local_ip,&Local_port,info);
/*	//读取下层IP与端口信息
	for(int i=0;i<=Lower_number-1;++i){
		info=Link_file.getUDPAddrString(CCfgFileParms::LOWER,i);//下层
		String2Num(&Lower_mac_ip[i],&Lower_mac_port[i],info);
	}*/
	tip(0,1);
	
	//缓存
	Tmp_buf=new char[MAX_BUFFER_SIZE];//临时缓存
	tip(!Tmp_buf,2);
	
	//套接字环境
	if(WSAStartup(MAKEWORD(2,2),&Local_data)==0) tip(0,3);//初始化
	else tip(1,3);
	
	//套接字
	if(Link_file.isConfigExist){
		tip(0,4);
		//本层套接字
		Local_sock=socket(AF_INET,SOCK_DGRAM,0);//面向无连接的传输，UDP
		memset(&Local_addr,0,SOCKADDR_SIZE);//初始化
		Local_addr=Link_file.getUDPAddr(CCfgFileParms::LOCAL,0);//自动配置
		Local_addr.sin_addr.S_un.S_addr=htonl(INADDR_ANY);//任意IP地址
		if(bind(Local_sock,(SOCKADDR*)&Local_addr,SOCKADDR_SIZE)!=SOCKET_ERROR) tip(0,5);//绑定
		else tip(1,5);
		//上层套接字
		if(!Top){
			Upper_addr=Link_file.getUDPAddr(CCfgFileParms::UPPER,0);//自动配置
			Link_file.getValueInt(Upper_mode,(char*)"Upper_mode");//接口模式
		}
		//下层套接字
		for(int i=0;i<=Lower_number-1;++i){
			Lower_addr[i]=Link_file.getUDPAddr(CCfgFileParms::LOWER,i);//自动配置
			Link_file.getValueInt(Lower_mode[i],(char*)"Lower_mode");//接口模式
		}
		//管理平台套接字
		Cmd_addr=Link_file.getUDPAddr(CCfgFileParms::CMDER,0);
		tip(0,6);
	}
	else tip(1,4);
	
	//套接字超时
	if(setsockopt(Local_sock,SOL_SOCKET,SO_RCVTIMEO,(char*)&Recv_timeout,sizeof(Recv_timeout))==0) tip(0,7);
	else tip(1,7);
	
/*	//套接字非阻塞
	unsigned long argp=1;
	if(ioctlsocket(Local_sock,FIONBIO,&argp)==0) tip(0,8);
	else tip(1,8);*/
	
	printf("\n");
	Link_file.print(11);
}
void Print_addr(){
	cprintf("MAC地址表：",62);print_void(108);
	cprintf("\n 接口 |        IP        |  端口  ",11);print_void(80);printf("\n");
	for(int i=0;i<=Lower_number-1;++i){
		for(int j=0;j<=Lower_mac_num[i]-1;++j){
			print_void(2);cprintf(i,11);print_void(4-num_len(i));cprintf("|",11);
			print_void(2);print_ip(Lower_mac_ip[i][j],1,11);print_void(1);cprintf("|",11);
			print_void(2);print_port(Lower_mac_port[i][j],1,11);print_void(86);printf("\n");
		}
	}
}
void learn_lower_number(HEAD *h,int Low_num){//反向地址学习(只学源地址)
	bool have=0;
	int mac_ip=h->get_source_ip();
	int mac_port=h->get_source_port();
	if(mac_port==65535) return;//表示来源为广播，不合逻辑，直接返回
	for(int i=0;i<=Lower_mac_num[Low_num]-1;++i){
		if(Lower_mac_port[Low_num][i]==mac_port){
			have=1;
			break;
		}
	}
	if(have==0){
		Lower_mac_ip[Low_num][Lower_mac_num[Low_num]]=mac_ip;
		Lower_mac_port[Low_num][Lower_mac_num[Low_num]++]=mac_port;
		if(A_Print_mode){
			cprintf("学习了新地址！  其ip：",11);
			print_ip(mac_ip,1,11);
			cprintf("其端口：",11);
			print_port(mac_port,1,11);
			print_void(69);printf("\n");
			Print_addr();
		}
	}
}
int learn_lower_number(int mac_ip,int mac_port,int Low_num){//反向 或 正向地址学习(查找)，并返回学习后的mac所在的序号
	if(mac_port==65535) return -1;//表示来源为广播(从上层传来)，不学习
	bool have=0;
	for(int i=0;i<=Lower_mac_num[Low_num]-1;++i){
		if(Lower_mac_port[Low_num][i]==mac_port){
			have=1;
			return i;
		}
	}
	if(have==0){
		Lower_mac_ip[Low_num][Lower_mac_num[Low_num]]=mac_ip;
		Lower_mac_port[Low_num][Lower_mac_num[Low_num]++]=mac_port;
		if(A_Print_mode){
			cprintf("学习了新地址：  其ip：",11);
			print_ip(mac_ip,1,11);
			cprintf("其端口：",11);
			print_port(mac_port,1,11);
			print_void(69);printf("\n");
			Print_addr();
		}
		return Lower_mac_num[Low_num]-1;
	}
}
int get_lower_number(HEAD *h){//查找地址表，得到下层接口号
	//成功返回接口号，失败或广播返回-1
	int Low_num=-1;
	int mac_ip=h->get_destination_ip();
	int mac_port=h->get_destination_port();
	if(mac_port==65535) return -1;
	for(int i=0;i<=Lower_number-1;++i){
		for(int j=0;j<=Lower_mac_num[i]-1;++j){
			if((Lower_mac_port[i][j]==mac_port)&&(Lower_mac_ip[i][j]==mac_ip)){
				Low_num=i;
				break;
			}
		}
		if(Low_num!=-1) break;
	}
	return Low_num;
}
int get_mac_number(int mac_ip,int mac_port,int Low_num){//查找对应接口的地址表，得到mac号在地址表中的位置
	//成功返回位置号，失败返回-1
	int Mac_num=-1;
	for(int j=0;j<=Lower_mac_num[Low_num]-1;++j){
		if((Lower_mac_port[Low_num][j]==mac_port)&&(Lower_mac_ip[Low_num][j]==mac_ip)){
			Mac_num=j;
			break;
		}
	}
	return Mac_num;
}
int get_send_window(){//自动分配一个发送窗口
	//成功则返回窗口号，失败返回-1
	for(int i=0;i<=Send_window_size-1;++i)
		if(Send_window_use[i]==0){
			if(Print_mode){//输出消息
				cprintf("分配了：",62);cprintf(i,62);cprintf("号窗口",62);
				print_void(103);printf("\n");
			}
			return i;
		}
	if(Print_mode){
		cprintf("窗口已满，分配失败",62);
		print_void(101);printf("\n");
	}
	return -1;
}
void close_send_window(int window,int mode){//关闭并重置一个发送窗口
	//mode为0则不释放帧对应的内存空间，为1则释放
	//注意，窗口号为-1时代表使用的为特殊窗口(专用于收发ACK/NAK) 
	if(window>=0){//普通窗口 
		Send_window_use[window]=0;
		Send_window_wait[window]=0;
		if(Send_window_busy[window]){
			Send_window_busy[window]=0;
			Send_window_busy_num--;
		}
		if(Send_window_stop[window]){
			Send_window_stop[window]=0;
			Send_window_stop_num--;
		}
		Send_window_info[window]=NULL_INFO;
		if(mode&&Send_window_frame[window]) delete[] Send_window_frame[window];
		Send_window_frame[window]=NULL;
		if(Print_mode){//输出消息
			cprintf("回收了：",62);cprintf(window,62);cprintf("号窗口",62);
			print_void(105-num_len(window));printf("\n");
		}
	}
	else{//特殊窗口 
		Special_window_info=NULL_INFO;
		if(mode&&Special_window_frame) delete[] Special_window_frame;
		Special_window_frame=NULL;
		if(Print_mode){//输出消息
			cprintf("回收了：特殊窗口",62);
			print_void(103);printf("\n");
		}
	}
}
void Send_Upper(int i){//向上层发送数据，输入为窗口类型
	//注意：窗口号为-1时代表使用特殊窗口(专用于收发控制信息)
	char *Recv_data;
	int buf_len,Low_num,Mac_num,seq;
	
	if(i!=-1){//普通数据
		Recv_data=Recv_tmp_data;
		  buf_len=Recv_tmp_info.buf_len;
		  Low_num=Recv_tmp_info.Lower_number;
		  Mac_num=Recv_tmp_info.Mac_number;
		      seq=Recv_tmp_info.sequence;
	}
	else{//控制信息
		Recv_data=Special_window_frame;
		  buf_len=16;
	}
//	printf(" #%d @%d ",Recv_tmp_info.Mac_number,Mac_num);
	
	//信息预处理
	int len=buf_len/8+(buf_len>0&&buf_len%8!=0);
	char byte[len+9];
	
	if(i!=-1){//普通数据
		Bit2Byte(Recv_data,buf_len,byte+9,len);//将发送帧转为字节型
		//添加ip、端口、序号信息
		if(Mac_num!=-1) set_source_mac_info(Lower_mac_ip[Low_num][Mac_num],Lower_mac_port[Low_num][Mac_num],seq,byte);
		else set_source_mac_info(-1,-1,seq,byte);
		len+=9;
	}
	else//控制信息
		Bit2Byte(Recv_data,buf_len,byte,len);//将发送帧转为字节型
	
	//发送
	int Recv_len=sendto(Local_sock,byte,len,0,(SOCKADDR*)&Upper_addr,SOCKADDR_SIZE);
	if(Recv_len<=0){//发送失败，则准备重传
		Send_amount.Error+=buf_len; Send_time.Error++;
		if(i!=-1){//只重传数据帧，不重传控制帧
			Recv_buf+=buf_len;             //增加缓存 
			Recv_queue.push(Recv_tmp_data);//添加帧至队列末
			Recv_info.push(Recv_tmp_info); //添加帧信息至队列末
		}
		
		if(Print_mode){//输出消息
			if(i!=-1){
				if(Print_mode==1) print_bit(Recv_data,buf_len,"向上层发送数据失败...数据bit流：",0,11);
				else print_byte(byte,len,"向上层发送数据失败...数据字节流：",1,11);
			}
			else{
				if(Print_mode==1) print_bit(Recv_data,buf_len,"向上层发送控制信息失败...其bit流：",0,11);
				else print_byte(byte,len,"向上层发送控制信息失败...其字节流：",1,11);
			}
		}
		if(i==-1) delete[] Special_window_frame;
	}
	else{//发送成功
		Send_amount.Total+=buf_len; Send_time.Total++;
		Send_amount.Upper+=buf_len; Send_time.Upper++;
		if(i!=-1) {Send_amount.UData+=buf_len; Send_time.UData++; Upper_send_data=1;}
		if(i==-1) {Send_amount.UCtrl+=buf_len; Send_time.UCtrl++;}
		
		if(Print_mode){//输出消息
			if(i!=-1){
				if(Print_mode==1) print_bit(Recv_data,buf_len,"向上层发送数据成功！数据bit流：",0,11);
				else print_byte(byte,len,"向上层发送数据成功！数据字节流：",1,11);
			}
			else{
				if(Print_mode==1) print_bit(Recv_data,buf_len,"向上层发送控制信息成功！其bit流：",0,11);
				else print_byte(byte,len,"向上层发送控制信息成功！其字节流：",1,11);
			}
		}
		if(i!=-1) delete[] Recv_tmp_data;
		else delete[] Special_window_frame;
	}
}
void Send_Lower(int i){//向下层发送，输入为窗口编号
	//注意，窗口号为-1时代表使用的为特殊窗口(专用于收发ACK/NAK) 
	char *Send_frame;
	int len,buf_len,Low_num,lable,resend;
	
	if(i>=0){//普通窗口 
		Send_frame=Send_window_frame[i];
		       len=Send_window_info[i].len;
		   buf_len=Send_window_info[i].buf_len;
		   Low_num=Send_window_info[i].Lower_number;
		     lable=Send_window_info[i].lable;
		    resend=Send_window_info[i].resend;
	}
	else{//特殊窗口 
		Send_frame=Special_window_frame;
		       len=Special_window_info.len;
		   buf_len=Special_window_info.buf_len;
		   Low_num=Special_window_info.Lower_number;
		     lable=Special_window_info.lable;
		    resend=Special_window_info.resend;
	}
	
	if(Lower_mode[Low_num]==0){//下层接口为bit类型
		int Send_len=sendto(Local_sock,Send_frame,len,0,(SOCKADDR*)&Lower_addr[Low_num],SOCKADDR_SIZE);
		if(Send_len<=0){//发送失败，则准备重传
			Send_amount.Error+=len; Send_time.Error++;
			Send_buf+=buf_len;                        //增加缓存 
			if(lable==0){
				Send_queue.push(Send_window_frame[i]);//添加帧至队列末
				Send_info.push(Send_window_info[i]);  //添加帧信息至队列末
			}
			else{
				Send_queue.push(Special_window_frame);//添加帧至队列末
				Send_info.push(Special_window_info);  //添加帧信息至队列末
			}
			
			if(Print_mode){//输出消息
				if(Print_mode==1){
					if(lable==0) print_bit(Send_frame,len,"向下层发送数据失败...数据bit流：",0,11);
					else print_bit(Send_frame,len,"向下层发送ACK/NAK失败...其bit流：",0,11);
				}
				else{
					if(lable==0) print_byte(Send_frame,len,"向下层发送数据失败...数据字节流：",0,11);
					else print_byte(Send_frame,len,"向下层发送ACK/NAK失败...其字节流：",0,11);
				}
			}
			close_send_window(i,0);//关闭窗口
		}
		else{//发送成功
			Send_amount.Total+=len; Send_time.Total++;
			Send_amount.Lower+=len; Send_time.Lower++;
			if(lable==0&&resend==0) {Send_amount.LData+=len; Send_time.LData++; Lower_send_data=1;}
			if(lable==0&&resend!=0) {Send_amount.Re+=len;    Send_time.Re++;    Lower_send_data=1;}
			if(lable==1) {Send_amount.ACK+=len; Send_time.ACK++;}
			if(lable==2) {Send_amount.NAK+=len; Send_time.NAK++;}
			
			if(Print_mode){//输出消息
				if(Print_mode==1){
					if(lable==0) print_bit(Send_frame,len,"向下层发送数据成功！数据bit流：",0,11);
					else print_bit(Send_frame,len,"向下层发送ACK/NAK成功！其bit流：",0,11);
				}
				else{
					if(lable==0) print_byte(Send_frame,len,"向下层发送数据成功！数据字节流：",0,11);
					else print_byte(Send_frame,len,"向下层发送ACK/NAK成功！其字节流：",0,11);
				}
			}
			if(lable||Top) close_send_window(i,1);//关闭窗口
		}
	}
	else{//下层接口为字节类型
		//先将发送帧转为字节型
		len=buf_len/8+(buf_len%8!=0);
		char byte[len];
		Bit2Byte(Send_frame,buf_len,byte,len);
		//发送
		int Send_len=sendto(Local_sock,byte,len,0,(SOCKADDR*)&Lower_addr[Low_num],SOCKADDR_SIZE);
		if(Send_len<=0){//发送失败，则准备重传
			Send_amount.Error+=buf_len; Send_time.Error++;
			Send_buf+=buf_len;                        //增加缓存 
			if(lable==0){
				Send_queue.push(Send_window_frame[i]);//添加帧至队列末
				Send_info.push(Send_window_info[i]);  //添加帧信息至队列末
			}
			else{
				Send_queue.push(Special_window_frame);//添加帧至队列末
				Send_info.push(Special_window_info);  //添加帧信息至队列末
			}
			
			if(Print_mode){//输出消息
				if(Print_mode==1){
					if(lable==0) print_bit(Send_frame,buf_len,"向下层发送数据失败...数据bit流：",0,11);
					else print_bit(Send_frame,buf_len,"向下层发送ACK/NAK失败...其bit流：",0,11);
				}
				else{
					if(lable==0) print_byte(byte,len,"向下层发送数据失败...数据字节流：",1,11);
					else print_byte(byte,len,"向下层发送ACK/NAK失败...其字节流：",1,11);
				}
			}
			close_send_window(i,0);//关闭窗口
		}
		else{//发送成功
			Send_amount.Total+=buf_len; Send_time.Total++;
			Send_amount.Lower+=buf_len; Send_time.Lower++;
			if(lable==0&&resend==0) {Send_amount.LData+=buf_len; Send_time.LData++; Lower_send_data=1;}
			if(lable==0&&resend!=0) {Send_amount.Re+=buf_len;    Send_time.Re++;    Lower_send_data=1;}
			if(lable==1) {Send_amount.ACK+=buf_len; Send_time.ACK++;}
			if(lable==2) {Send_amount.NAK+=buf_len; Send_time.NAK++;}
			
			if(Print_mode){//输出消息
				if(Print_mode==1){
					if(lable==0) print_bit(Send_frame,buf_len,"向下层发送数据成功！数据bit流：",0,11);
					else print_bit(Send_frame,buf_len,"向下层发送ACK/NAK成功！其bit流：",0,11);
				}
				else{
					if(lable==0) print_byte(byte,len,"向下层发送数据成功！数据字节流：",1,11);
					else print_byte(byte,len,"向下层发送ACK/NAK成功！其字节流：",1,11);
				}
			}
			if(lable||Top) close_send_window(i,1);//关闭窗口
		}
	}
}
void broadcast(int i){//向下广播，输入为窗口号
	char *Send_frame=Send_window_frame[i];
	int          len=Send_window_info[i].len;
	int      buf_len=Send_window_info[i].buf_len;
	int   source_num=Send_window_info[i].Lower_number;
	int        lable=Send_window_info[i].lable;
	int       resend=Send_window_info[i].resend;
	
	for(int Low_num=0;Low_num<=Lower_number-1;++Low_num){
		if(Low_num==source_num) continue;//下发实体号与来源相同，跳过
//		printf("%d ",Low_num);
		if(Lower_mode[Low_num]==0){//下层接口为bit类型
			int Send_len=sendto(Local_sock,Send_frame,len,0,(SOCKADDR*)&Lower_addr[Low_num],SOCKADDR_SIZE);
			if(Send_len<=0){//发送失败，直接放弃
				Send_amount.Error+=len; Send_time.Error++;
							
				if(Print_mode){//输出消息
					if(Print_mode==1)print_bit(Send_frame,len,"向下层广播数据失败...数据bit流：",0,11);
					else print_byte(Send_frame,len,"向下层广播数据失败...数据字节流：",0,11);
				}
			}
			else{//发送成功
				Send_amount.Total+=len; Send_time.Total++;
				Send_amount.Lower+=len; Send_time.Lower++;
				Send_amount.LData+=len; Send_time.LData++; Lower_send_data=1;
				
				if(Print_mode){//输出消息
					if(Print_mode==1)print_bit(Send_frame,len,"向下层广播数据成功！数据bit流：",0,11);
					else print_byte(Send_frame,len,"向下层广播数据成功！数据字节流：",0,11);
				}
			}
		}
		else{//下层接口为字节类型
			//先将发送帧转为字节型
			len=buf_len/8+(buf_len%8!=0);
			char byte[len];
			Bit2Byte(Send_frame,buf_len,byte,len);
			//发送
			int Send_len=sendto(Local_sock,byte,len,0,(SOCKADDR*)&Lower_addr[Low_num],SOCKADDR_SIZE);
			if(Send_len<=0){//发送失败，直接放弃
				Send_amount.Error+=buf_len; Send_time.Error++;
				
				if(Print_mode){//输出消息
					if(Print_mode==1)print_bit(Send_frame,buf_len,"向下层广播数据失败...数据bit流：",0,11);
					else print_byte(byte,len,"向下层广播数据失败...数据字节流：",1,11);
				}
			}
			else{//发送成功
				Send_amount.Total+=buf_len; Send_time.Total++;
				Send_amount.Lower+=buf_len; Send_time.Lower++;
				Send_amount.LData+=buf_len; Send_time.LData++; Lower_send_data=1;
				
				if(Print_mode){//输出消息
					if(Print_mode==1) print_bit(Send_frame,buf_len,"向下层广播数据成功！数据bit流：",0,11);
					else print_byte(byte,len,"向下层广播数据成功！数据字节流：",1,11);
				}
			}
		}
	}
	close_send_window(i,1);//关闭窗口
}
int Send_Cmd(char *buf,int len){//向管理层发送数据，返回发送长度
	return sendto(Local_sock,buf,len,0,(SOCKADDR*)&Cmd_addr,SOCKADDR_SIZE);
}
void Recv_Upper(int len){//从上层收到
	if(len==2&&(unsigned char)*Tmp_buf==0xff){//如果是控制信息，则调整同层拥堵指数
		Recv_amount.Total+=16; Recv_time.Total++;
		Recv_amount.Upper+=16; Recv_time.Upper++;
		Recv_amount.UCtrl+=16; Recv_time.UCtrl++;
		char n=*(Tmp_buf+1);
		if(n==0) Upper_full=0;
		else if(n>0) Upper_full+=n;
		return;
	}//否则认为是数据信息，正常传输
	
	//获取下发的IP、端口号、目标实体号(只能是0)
	int mac_ip=0,mac_port=0;
	get_destination_mac_info(&mac_ip,&mac_port,Tmp_buf,len);
	
	int Low_num=0;
	int Mac_num=learn_lower_number(mac_ip,mac_port,Low_num);//正向地址学习(查找)
	
	//分帧并保存至发送缓存
	int pos=6,len2;
	while(pos<=len-1){
		if(pos+Divide_len/8-1<=len-1) len2=Divide_len/8;
		else len2=len-pos;
		
		//创建头部
		HEAD h;
		if(mac_port!=65535) change_head(&h,0,1,Send_seq[Mac_num],Local_ip,Local_port,Lower_mac_ip[Low_num][Mac_num],Lower_mac_port[Low_num][Mac_num]);
		else change_head(&h,0,1,0,Local_ip,Local_port,-1,65535);
		
		//成帧，并将帧加至发送缓存
		int frame_len=frame(Tmp_buf+pos,len2,&h,1,Lower_mode[Low_num]);
		if(frame_len==-1){
			Recv_amount.Error+=8*len2; Recv_time.Error++;
			tip(2,3);
		}
		else{
			if(Lower_mode[Low_num]==0){//帧为bit型，直接添加
				if(Send_buf+frame_len>MAX_BUFFER_SIZE){
					Recv_amount.Error+=8*len2; Recv_time.Error++;
					tip(2,5);
				}
				else{
					Upper_recv_data=1;
					Recv_amount.Total+=8*len2; Recv_time.Total++;
					Recv_amount.Upper+=8*len2; Recv_time.Upper++;
					Recv_amount.UData+=8*len2; Recv_time.UData++;
					if(T_Print_mode) tip(2,4);
					
					char *tmp_frame;
					try{tmp_frame=new char[frame_len];}catch(bad_alloc){tip(2,16);return;}
					change_frame(tmp_frame,Tmp_buf+pos,frame_len);//复制
					
					Send_buf+=frame_len;       //缓冲区占用大小
					Send_queue.push(tmp_frame);//添加帧至发送队列
					
					if(mac_port!=65535){//非广播
						INFO tmp_info(Send_seq[Mac_num],frame_len,frame_len,Low_num,Mac_num,0);
						Send_info.push(tmp_info);  //添加发送信息至队列
						Send_seq[Mac_num]++;       //发送序号+1，为下一次发送作准备
					}
					else{//广播
						INFO tmp_info(0,frame_len,frame_len,-1,65535,0);
						Send_info.push(tmp_info);  //添加发送信息至队列
					}
				}
			}
			else{//帧为字节型，先转换为bit型，再添加
				if(Send_buf+8*frame_len>MAX_BUFFER_SIZE){
					Recv_amount.Error+=8*len2; Recv_time.Error++;
					tip(2,5);
				}
				else{
					Upper_recv_data=1;
					Recv_amount.Total+=8*len2; Recv_time.Total++;
					Recv_amount.Upper+=8*len2; Recv_time.Upper++;
					Recv_amount.UData+=8*len2; Recv_time.UData++;
					if(T_Print_mode) tip(2,4);
					
					char *tmp_frame;
					try{tmp_frame=new char[8*frame_len];}catch(bad_alloc){tip(2,16);return;}
					Byte2Bit(tmp_frame,8*frame_len,Tmp_buf+pos,frame_len);//转换
					
					Send_buf+=8*frame_len;     //缓冲区占用大小
					Send_queue.push(tmp_frame);//添加至发送队列
					
					if(mac_port!=65535){//非广播
						INFO tmp_info(Send_seq[Mac_num],frame_len,8*frame_len,Low_num,Mac_num,0);
						Send_info.push(tmp_info);  //添加发送信息至队列
						Send_seq[Mac_num]++;       //发送序号+1，为下一次发送作准备
					}
					else{//广播
						INFO tmp_info(0,frame_len,8*frame_len,-1,65535,0);
						Send_info.push(tmp_info);  //添加发送信息至队列
					}
				}
			}
		}
		pos+=Divide_len/8;
	}
	
	//判断缓存空间，并发送控制信息
	if(Send_buf>Send_buf_high_thershold){
		char *control;
		try{control=new char[16];}catch(bad_alloc){tip(2,16);return;}
		set_control(control,Send_buf,Send_buf_high_thershold);
		Special_window_frame=control;
		Send_Upper(-1);
	}
}
void Recv_Lower(int len,int Low_num){//从下层收到
	int bit_len=(1+7*(Lower_mode[Low_num]==1))*len;
	
	char backup_frame[bit_len];//备份帧，用于交换机
	if(Top) change_frame(backup_frame,Tmp_buf,len);
	
	HEAD h;
	int frame_len=deframe(Tmp_buf,len,&h,Lower_mode[Low_num],0);//恢复数据，并将其转化为bit型
	
	if(frame_len==-1){//收到的帧超长
		Recv_amount.Error+=8*bit_len; Recv_time.Error++;
		tip(2,7);
	}
	else if(frame_len==-2){//收到的帧有误且无法纠正
		Recv_amount.Total+=8*bit_len; Recv_time.Total++;
		Recv_amount.Lower+=8*bit_len; Recv_time.Lower++;
		Recv_amount.Wrong+=8*bit_len; Recv_time.Wrong++;
		
		if(!Top){//如果不是顶层
			if(bit_len<=8*HEAD_SIZE*1.5){//帧长较短，直接丢弃
				if(T_Print_mode) tip(2,8);
			}
			else{//帧长较长，尝试提取序号，添加NAK至发送队列
				int seq=h.get_sequence();
				if(seq<=0||seq>=4194304) return;//序号不正常或过大，直接返回
				
				int d_ip=h.get_destination_ip();
				int d_port=h.get_destination_port();
				if((d_ip!=Local_ip)||(d_port!=Local_port)) return;//目的地不是本地，直接返回
				
				int Mac_num=get_mac_number(h.get_source_ip(),h.get_source_port(),Low_num);
				if(Mac_num==-1) return;//无法确定来源地址，直接返回
				
				if(T_Print_mode) tip(2,9);
				change_head(&h,2,1,seq,Local_ip,Local_port,Lower_mac_ip[Low_num][Mac_num],Lower_mac_port[Low_num][Mac_num]);
				frame_len=frame(Tmp_buf,0,&h,0,Lower_mode[Low_num]);
				
				if(Lower_mode[Low_num]==0){//帧为bit型，直接向下发送
					char *tmp_frame;
					try{tmp_frame=new char[frame_len];}catch(bad_alloc){tip(2,16);return;}
					change_frame(tmp_frame,Tmp_buf,frame_len);//复制
					
					INFO tmp_info(seq,frame_len,frame_len,Low_num,Mac_num,2);
					
					Special_window_frame=tmp_frame;
					Special_window_info=tmp_info;
					Send_Lower(-1);//发送
				}
				else{//帧为字节型，先转换为bit型(避免发送失败后添加到缓存中，产生错误)，再向下发送
					char *tmp_frame;
					try{tmp_frame=new char[8*frame_len];}catch(bad_alloc){tip(2,16);return;}
					Byte2Bit(tmp_frame,8*frame_len,Tmp_buf,frame_len);//转换
					
					INFO tmp_info(seq,frame_len,8*frame_len,Low_num,Mac_num,2);
					
					Special_window_frame=tmp_frame;
					Special_window_info=tmp_info;
					Send_Lower(-1);//发送
				}
			}
		}
	}
	else{//收到的帧无误
		if(Top){//是顶层，转存后直接返回
			learn_lower_number(&h,Low_num);//反向地址学习
			int next_lower_num=get_lower_number(&h);
			int Mac_num=-1;
			if(next_lower_num==-1){//没有找到目标地址 或 本身就是广播帧
				next_lower_num=Low_num;//下一个接口用来表示来源接口
				Mac_num=65535;//广播的mac地址
			}
			else//找到了目标地址
				Mac_num=get_mac_number(h.get_destination_ip(),h.get_destination_port(),next_lower_num);
			
			Recv_amount.Total+=bit_len; Recv_time.Total++;
			Recv_amount.Lower+=bit_len; Recv_time.Lower++;
			Recv_amount.LData+=bit_len; Recv_time.LData++;
			
			char *tmp_data;
			try{tmp_data=new char[len];}catch(bad_alloc){tip(2,16);return;}
			change_frame(tmp_data,backup_frame,len);//复制
			Recv_buf+=bit_len;        //缓冲区占用大小
			Recv_queue.push(tmp_data);//添加至接收队列
			
			INFO tmp_info(-1,len,bit_len,next_lower_num,Mac_num,0);
			Recv_info.push(tmp_info); //添加接收信息至队列
			
			return;
		}
		//不是顶层
		int d_ip=h.get_destination_ip();
		int d_port=h.get_destination_port();
		int seq=h.get_sequence();
		int Mac_num;
		
		if((d_port!=65535)&&((d_ip!=Local_ip)||(d_port!=Local_port))){//不是广播，且目的地不是本地，直接返回
			Recv_amount.Unknown+=bit_len;   Recv_time.Unknown++;
			return;
		}
		Mac_num=learn_lower_number(h.get_source_ip(),h.get_source_port(),Low_num);//反向地址学习(查找)
//		printf("%d ",Mac_num);
		
		if(h.get_ack()){//收到ACK
			Recv_amount.Total+=bit_len; Recv_time.Total++;
			Recv_amount.Lower+=bit_len; Recv_time.Lower++;
			Recv_amount.ACK+=bit_len;   Recv_time.ACK++;
			if(T_Print_mode) tip(2,10);
			
			if(d_port!=65535){//非广播才调整
			//关闭窗口
				for(int i=0;i<=Send_window_size-1;++i)
				if(Send_window_use[i]){
					if((seq==Send_window_info[i].sequence)&&(Mac_num==Send_window_info[i].Mac_number)){
						close_send_window(i,1);
						break;
					}
				}
			//调整同层拥堵指数
				char byte[2];
				Bit2Byte(Tmp_buf,16,byte,2);
				if(byte[1]==0) Lower_full=0;
				else if(byte[1]>0) Lower_full+=byte[1];
			}
		}
		else if(h.get_nak()){//收到NAK
			Recv_amount.Total+=bit_len; Recv_time.Total++;
			Recv_amount.Lower+=bit_len; Recv_time.Lower++;
			Recv_amount.NAK+=bit_len;   Recv_time.NAK++;
			if(T_Print_mode) tip(2,11);
			
			if(d_port!=65535)//非广播才调整
			for(int i=0;i<=Send_window_size-1;++i){
				if(Send_window_use[i]){
					if((seq==Send_window_info[i].sequence)&&(Mac_num==Send_window_info[i].Mac_number)){
						Send_window_wait[i]=10000000;//修改等待周期为大数，使其在下个时钟周期自动重传
						break;
					}
				}
			}
		}
		else{//收到普通数据
			Recv_amount.Total+=bit_len; Recv_time.Total++;
			Recv_amount.Lower+=bit_len; Recv_time.Lower++;
			Recv_amount.LData+=bit_len; Recv_time.LData++;
			
			if(seq>=4194304) return;//帧序号过大，无法接受
			if((d_port!=65535)&&Recv_data_time[Mac_num][seq]){//如果收到的不是广播，且收到过该帧，只发送ACK
				
				change_head(&h,1,1,seq,Local_ip,Local_port,Lower_mac_ip[Low_num][Mac_num],Lower_mac_port[Low_num][Mac_num]);
				if(ACK_add_clear==0) set_control(Tmp_buf,Recv_buf,Recv_buf_high_thershold);//附加控制信息
				else set_control(Tmp_buf,-1,0);//附加控制信息
				frame_len=frame(Tmp_buf,16,&h,0,Lower_mode[Low_num]);
				
				if(Lower_mode[Low_num]==0){//帧为bit型，直接发送
					if(T_Print_mode) tip(2,12);
					char *tmp_frame;
					try{tmp_frame=new char[frame_len];}catch(bad_alloc){tip(2,16);return;}
					change_frame(tmp_frame,Tmp_buf,frame_len);//复制
					
					INFO tmp_info(seq,frame_len,frame_len,Low_num,Mac_num,1);
					
					Special_window_frame=tmp_frame;
					Special_window_info=tmp_info;
					Send_Lower(-1);//发送
				}
				else{//帧为字节型，先转换为bit型(避免发送失败后添加到缓存中，产生错误)，再发送
					if(T_Print_mode) tip(2,12);
					char *tmp_frame;
					try{tmp_frame=new char[8*frame_len];}catch(bad_alloc){tip(2,16);return;}
					Byte2Bit(tmp_frame,8*frame_len,Tmp_buf,frame_len);//转换
					
					INFO tmp_info(seq,frame_len,8*frame_len,Low_num,Mac_num,1);
					
					Special_window_frame=tmp_frame;
					Special_window_info=tmp_info;
					Send_Lower(-1);//发送
				}
			}
			else{//未收到过该帧，添加其数据的bit流至接收/发送队列，并添加对应ACK至发送队列
				if(Recv_buf+frame_len>MAX_BUFFER_SIZE){
					Recv_amount.Error+=bit_len; Recv_time.Error++;
					tip(2,14);
				} 
				else{
					Recv_data_time[Mac_num][seq]=1;
					Lower_recv_data=1;
					if(T_Print_mode) tip(2,13);
					
					//添加数据
					char *tmp_data;
					try{tmp_data=new char[frame_len];}catch(bad_alloc){tip(2,16);return;}
					change_frame(tmp_data,Tmp_buf,frame_len);//复制
					Recv_buf+=frame_len;      //缓冲区占用大小
					Recv_queue.push(tmp_data);//添加至接收队列
					
					INFO tmp_info(seq,frame_len,frame_len,Low_num,Mac_num,0);
					Recv_info.push(tmp_info); //添加接收信息至队列
					
					//如果收到的不是广播，发送ACK
					if(d_port!=65535){
						change_head(&h,1,1,seq,Local_ip,Local_port,Lower_mac_ip[Low_num][Mac_num],Lower_mac_port[Low_num][Mac_num]);
						if(ACK_add_clear==0) set_control(Tmp_buf,Recv_buf,Recv_buf_high_thershold);//附加控制信息
						else set_control(Tmp_buf,-1,0);//附加控制信息
						frame_len=frame(Tmp_buf,16,&h,0,Lower_mode[Low_num]);
						
						if(Lower_mode[Low_num]==0){//下层为bit型，直接发送
							char *tmp_frame;
							try{tmp_frame=new char[frame_len];}catch(bad_alloc){tip(2,16);return;}
							change_frame(tmp_frame,Tmp_buf,frame_len);//复制
							
							INFO tmp_info2(seq,frame_len,frame_len,Low_num,Mac_num,1);
							
							Special_window_frame=tmp_frame;
							Special_window_info=tmp_info2;
							Send_Lower(-1);//发送
						}
						else{//下层为字节型，先转换为bit型(避免发送失败后添加到缓存中，产生错误)，再发送
							char *tmp_frame;
							try{tmp_frame=new char[8*frame_len];}catch(bad_alloc){tip(2,16);return;}
							Byte2Bit(tmp_frame,8*frame_len,Tmp_buf,frame_len);//转换
							
							INFO tmp_info2(seq,frame_len,8*frame_len,Low_num,Mac_num,1);
							
							Special_window_frame=tmp_frame;
							Special_window_info=tmp_info2;
							Send_Lower(-1);//发送
						}
					}
				}
			}
		}
	}
}
int main(int argc,char *argv[]){
	//初始化
	tip(0,0);
	initial(argc,argv);
	
	//循环接收、发送数据
	SOCKADDR_IN Remote_addr;                //用于存储接收到的套接字
	register bool event_happen=1;           //是否有事件发生(用于判断是否打印数据信息)
	register bool receive_blocked=0;        //该周期中接收是否阻塞(无数据收到)
	register unsigned char Error_time=0;    //周期内重传的次数
	register unsigned char No_Error_cycle=0;//未发生错误的连续周期数	
	register int tmp;                       //存储从套接字接收到的数据长度
	register unsigned short time_tmp;       //分帧时帧长度减少的次数判断依据 
	register unsigned short Upper_wait=0;   //向上层发送的等待时间
	register unsigned short Lower_wait=0;   //向下层发送的等待时间
	register unsigned long long cycle=0;    //循环周期数
	while(++cycle){
//		printf(" $%d$ ",cycle);
		//尝试接收数据
		while(1){//不断接收，直到接收失败
			tmp=recvfrom(Local_sock,Tmp_buf,MAX_BUFFER_SIZE,0,(SOCKADDR*)&Remote_addr,&SOCKADDR_SIZE);
			if(tmp==0){//另一端关闭了连接，本端也应关闭
				closesocket(Local_sock);
				Local_sock=0;
				tip(2,1);
				Sleep(1000);
				break;
			}
			if(tmp==-1){//发生异常
				tmp=WSAGetLastError();
				receive_blocked=1;
				if(tmp!=10060){//错误原因不是超时
					closesocket(Local_sock);
					Local_sock=0;
					cout<<tmp<<endl;
					tip(2,2);
					Sleep(1000);
				}
				break;
			}
			else if(tmp>0){//正常接收到数据了，开始判断数据来源
				event_happen=1;
				bool unknown=1;
				if(compare_sock(Remote_addr,Cmd_addr)&&(strncmp(Tmp_buf,"exit",5)==0)){//管理层发出了关闭指令
					unknown=0;
					shutdown();
					return 0;
				}
				if(tmp>MAX_BUFFER_SIZE){//超过缓存上限
					Recv_amount.Error+=tmp; Recv_time.Error++;
					tip(2,3);
				}
				if(!Top&&compare_sock(Remote_addr,Upper_addr)){//上层
					unknown=0;
					Recv_Upper(tmp);
				}
				for(int i=0;i<=Lower_number-1;++i)//下层
				if(compare_port(Remote_addr,Lower_addr[i])){
					unknown=0;
					Recv_Lower(tmp,i);
				}
				if(unknown){//来源未知
					Recv_amount.Unknown+=tmp; Recv_time.Unknown++;
				}
			}
		}
		
		//尝试向下发送数据
		if(Lower_send_stop) Lower_send_stop--;
		else{
		//1.重传旧数据
			for(int i=0;i<=Send_window_size-1;++i)
			if(Send_window_use[i]){//窗口被占用
				if(receive_blocked) Send_window_wait[i]+=Recv_timeout;
				if(Send_window_wait[i]>=R_Send_timeout){//窗口超过重传超时
					Send_window_wait[i]=0;
					Send_window_info[i].resend++;
					
					if(Send_window_stop[i]==0){//窗口未暂停
						if(Send_window_info[i].resend>=Send_window_stop_check){//重发次数达到暂停判定次数，暂停重传
							Send_window_stop[i]=1;//设置为暂停窗口
							Send_window_stop_num++;
						}
						else{
							if(Send_window_busy[i]==0){//窗口未繁忙
								if(Send_window_info[i].resend>=Send_window_busy_check){//重发次数达到繁忙判定次数，记录繁忙
									Send_window_busy[i]=1;//设置为繁忙窗口
									Send_window_busy_num++;
								}
							}
							event_happen=1;
							Error_time++;
							No_Error_cycle=-1;
							Send_Lower(i);//重传
						}
					}
					else if(Send_window_info[i].resend%(Send_window_stop_check*Send_window_stop_check)==0){
						event_happen=1;
						Error_time++;
						Send_Lower(i);//重传
					}
				}
			}
		//2.发送新数据
			Lower_wait+=Recv_timeout;
			while(Lower_wait>=Lower_Send_timeout){
				Lower_wait-=Lower_Send_timeout;
				if(Send_buf){
					if(Send_queue.empty()||Send_info.empty()){//发生错误，缓存不为空，但队列为空
						tip(2,6);
						Send_buf=0;                                 //重置缓存
						while(!Send_queue.empty()) Send_queue.pop();//清空队列
						while(!Send_info.empty()) Send_info.pop();  //清空队列
					}
					else{//无错误，正常传输
						int window=get_send_window();
						if(window!=-1){//有发送窗口可用
							event_happen=1;
							
							Send_window_use[window]=1;                   //占用窗口
							Send_window_frame[window]=Send_queue.front();//读取待发送数据
							Send_window_info[window]=Send_info.front();  //读取待发送数据信息
							
							Send_buf-=Send_window_info[window].buf_len;//释放空间
							Send_queue.pop();//出队
							Send_info.pop(); //出队
							
							if(Send_window_info[window].Mac_number==65535) broadcast(window);//广播
							else Send_Lower(window);//发送
						}
					}
				}
				else if((!Send_queue.empty())||(!Send_info.empty())){//发生错误，缓存为空，但队列不为空
					tip(2,6);
					Send_buf=0;                                 //重置缓存
					while(!Send_queue.empty()) Send_queue.pop();//清空队列
					while(!Send_info.empty()) Send_info.pop();  //清空队列
				}
			}
		}
		
		//如果不是顶层，尝试向上发送数据
		if(!Top){
			if(Upper_send_stop) Upper_send_stop--;
			else{
				Upper_wait+=Recv_timeout;
				while(Upper_wait>=Upper_Send_timeout){
					Upper_wait-=Upper_Send_timeout;
					if(Recv_buf){
						if(Recv_queue.empty()||Recv_info.empty()){//发生错误，缓存不为空，但队列为空
							tip(2,15);
							Recv_buf=0;                                 //重置缓存
							while(!Recv_queue.empty()) Recv_queue.pop();//清空队列
							while(!Recv_info.empty()) Recv_info.pop();  //清空队列
						}
						else{//无错误，正常传输
							event_happen=1;
							
							Recv_tmp_data=Recv_queue.front();//读取待发送数据
							Recv_tmp_info=Recv_info.front(); //读取待发送数据信息 
							
							Recv_buf-=Recv_tmp_info.buf_len;//释放空间
							Recv_queue.pop();//出队
							Recv_info.pop(); //出队
							
							Send_Upper(0);
						}
					}
					else if((!Recv_queue.empty())||(!Recv_info.empty())){//发生错误，缓存为空，但队列不为空
						tip(2,15);
						Recv_buf=0;                                 //重置缓存
						while(!Recv_queue.empty()) Recv_queue.pop();//清空队列
						while(!Recv_info.empty()) Recv_info.pop();  //清空队列
					}
				}
			}
		}
		//如果是顶层，转移接收缓存的数据到发送缓存中
		else if(Recv_buf){
			Recv_tmp_data=Recv_queue.front();//读取待发送数据
			Recv_tmp_info=Recv_info.front(); //读取待发送数据信息 
			
			while(Send_buf+Recv_tmp_info.buf_len<=MAX_BUFFER_SIZE){
				event_happen=1;
				
				Recv_buf-=Recv_tmp_info.buf_len;//释放空间
				Recv_queue.pop();//出队
				Recv_info.pop(); //出队
				
				Send_buf+=Recv_tmp_info.buf_len;//占用空间
				Send_queue.push(Recv_tmp_data);//入队
				Send_info.push(Recv_tmp_info); //入队
				
				if(Recv_buf){
					Recv_tmp_data=Recv_queue.front();//读取待发送数据
					Recv_tmp_info=Recv_info.front(); //读取待发送数据信息 
				}
				else break;
			}
		}
		
		//更改流量控制信息
		if(!Top){
		//1.拥堵指数
			if(Upper_send_data&&(Recv_buf>Recv_buf_low_thershold)&&(Recv_buf<=Recv_buf_high_thershold)){//上层拥堵指数
				event_happen=1;
				Upper_full--;
			}
			if(Lower_send_data&&(Send_buf>Send_buf_low_thershold)&&(Send_buf<=Send_buf_high_thershold)){//下层拥堵指数
				event_happen=1;
				Lower_full--;
			}
			if(Upper_recv_data&&(Send_buf>Send_buf_high_thershold)){//下层拥堵指数
				event_happen=1;
				Lower_full--;
			}
			if(Lower_recv_data&&(Recv_buf>Recv_buf_high_thershold)){//上层拥堵指数
				event_happen=1;
				Upper_full--;
			}
		//2.上层相关
			if(Upper_full>=100){//上层发送超时
				event_happen=1;
				double k=log(100-Upper_full_thershold)/log(2);
				int time=ceil((double)Recv_timeout*0.05*k);
				if(Upper_Send_timeout+time<=1000) Upper_Send_timeout+=time;
				Upper_full=3;
				Upper_send_stop=3;
			}
			else if(Upper_full>Upper_full_thershold){
				event_happen=1;
				double k=log(Upper_full-Upper_full_thershold)/log(2);
				int time=ceil((double)Recv_timeout*0.05*k);
				if(Upper_Send_timeout+time<=1000) Upper_Send_timeout+=time;
				Upper_full=3;
			}
			else if(Upper_full<=0){
				event_happen=1;
				Upper_full=Upper_full_thershold*0.5;
				int time=ceil((double)Upper_Send_timeout*0.1);
				if(Upper_Send_timeout-time>=5) Upper_Send_timeout-=time;
			}
		//3.同层相关
			if(Lower_full>=100){//同层发送超时
				event_happen=1;
				double k=log(100-Lower_full_thershold)/log(2);
				int time=ceil((double)Recv_timeout*0.05*k);
				if(Lower_Send_timeout+time<=1000) Lower_Send_timeout+=time;
				Lower_full=3;
				Lower_send_stop=3;
			}
			else if(Lower_full>Lower_full_thershold){
				event_happen=1;
				double k=log(Lower_full-Lower_full_thershold)/log(2);
				int time=ceil((double)Recv_timeout*0.05*k);
				if(Lower_Send_timeout+time<=1000) Lower_Send_timeout+=time;
				Lower_full=3;
			}
			else if(Lower_full<=0){
				event_happen=1;
				Lower_full=Lower_full_thershold*0.5;
				int time=ceil((double)Lower_Send_timeout*0.1);
				if(Lower_Send_timeout-time>=5) Lower_Send_timeout-=time;
			}
		//4.本层相关
			time_tmp=ceil((double)(Send_window_size-Send_window_stop_num)/4)*ceil((double)Recv_timeout/Lower_Send_timeout);
			if(Error_time>time_tmp)//如果重传次数过多，减少分帧长度
				if(Divide_len>=110) Divide_len-=10;
			if(Lower_send_data&&++No_Error_cycle>=7){//如果连续7周期没有重传，增加分帧长度
				No_Error_cycle=0;
				if(Divide_len<=990){
					if(!Upper_recv_data) Divide_len+=10;
					else if(Divide_len<=tmp-10) Divide_len+=10;
				}
			}
			if(Send_window_busy_num<floor(Send_window_size*0.5)){//如果窗口占用少，则关闭未使用的窗口，但总窗口数不少于10个
				if(Send_window_size>10&&Send_window_use[Send_window_size-1]==0){
					event_happen=1;
					Send_window_size--;
					if(T_Print_mode){
						cprintf("*窗口普遍空闲，已减少窗口数。",3);
						cprintf("目前窗口数",3);cprintf(Send_window_size,14);cprintf("个。",3);
						print_void(74-num_len(Send_window_size));printf("\n");
					}
				}
			}
			else if(Send_window_busy_num>=ceil(Send_window_size*0.5)+1){//如果窗口普遍繁忙，则增加数量
				if(Send_window_size<=99){
					event_happen=1;
					Send_window_size++;
					if(T_Print_mode){
						cprintf("*窗口普遍繁忙，已增加窗口数。",3);
						cprintf("目前窗口数",3);cprintf(Send_window_size,14);cprintf("个。",3);
						print_void(74-num_len(Send_window_size));printf("\n");
					}
				}
			}
			if(Recv_buf_no_empty&&(Recv_buf==0)){//接收缓存由满转空，发送清零控制信息
				char *control;
				try{control=new char[16];}catch(bad_alloc){tip(2,16);}
				set_control(control,-1,0);
				Special_window_frame=control;
				Send_Upper(-1);
			}
			if(Send_buf_no_empty&&(Send_buf==0)){//发送缓存由满转空，指示下次ACK附加清零信息
				ACK_add_clear=1;
			}
		}
		Error_time=0;              //重置重传次数
		receive_blocked=0;         //重置周期接收阻塞判断值
		Upper_send_data=0;         //重置向上发送数据判断值
		Lower_send_data=0;         //重置向下发送数据判断值
		Upper_recv_data=0;         //重置从上接收数据判断值
		Lower_recv_data=0;         //重置从下接收数据判断值
		Recv_buf_no_empty=Recv_buf;//更新接收缓存状态判断值
		Send_buf_no_empty=Send_buf;//更新发送缓存状态判断值
		
		//打印统计信息
		if(event_happen){
			print_data(Recv_timeout,Upper_Send_timeout,Lower_Send_timeout,R_Send_timeout,Divide_len,\
			           Upper_full,Lower_full,Send_buf,Recv_buf,Send_window_size,Send_window_busy_num,Send_window_stop_num,\
			           &Send_amount,&Send_time,&Recv_amount,&Recv_time,1,11);
			event_happen=0;
		}
	}
}