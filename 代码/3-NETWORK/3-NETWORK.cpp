//3 NETWORK Layer(网络层)
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
int SOCKADDR_SIZE=sizeof(SOCKADDR_IN); //套接字地址长度(字节)

WSADATA     Local_data;    //本层初始化信息
SOCKET      Local_sock;    //本层套接字
SOCKADDR_IN Local_addr;    //本层套接字地址
SOCKADDR_IN Upper_addr;    //上层套接字地址(应用层)
SOCKADDR_IN Lower_addr[8]; //下层套接字地址(链路层)
SOCKADDR_IN Cmd_addr;      //管理层套接字地址

int Top=0;             //是否为顶层，即是否为路由器
int Sub_net=0;         //本层虚拟子网号
int Local_ip;          //本层真实IP
int Local_port;        //本层真实端口
int Sub_net_num=1;     //子网总数 (默认1)(最大512)
int Sub_net_ip[512];   //子网对应真实IP (默认Local_ip)
int Sub_net_port[512]; //子网对应真实端口 (默认-1)
int Sub_net_next[512]; //到达对应子网的出口 (默认-1)(-1代表自身处于的子网)(非负数代表下层接口号)'
int Router[512];       //与对应子网的出口相连的是不是路由器 (默认0)

int Source_subnet;     //临时存储接收到的帧，其源子网
int Source_ip;         //临时存储接收到的帧，其源真实IP
int Source_port;       //临时存储接收到的帧，其源真实端口
int Destination_subnet;//临时存储接收到的帧，其目的子网
int Destination_ip;    //临时存储接收到的帧，其目的真实IP
int Destination_port;  //临时存储接收到的帧，其目的真实端口
int Sequence;          //临时存储接收到的帧，其序号

int Print_mode=0;   //本层收发信息打印模式，0为不打印收发的内容，1为打印bit流，2为打印8进制 (默认1)
int T_Print_mode=0; //本层提示信息打印模式，0为只打印发生错误时的提示信息，1增加打印收发处理信息，2增加打印流量控制信息 (默认0)
int Upper_mode=1;   //上层接口模式，0为bit数组，1为字节数组 (默认1)
int Lower_mode[8]={1,1,1,1,1,1,1,1}; //下层接口模式，0为bit数组，1为字节数组 (默认1)
int Lower_number=1; //下层实体数量 (默认1)(最大为8)

int Upper_full=25;         //上层拥堵指数 (默认25)
int Lower_full=25;         //下层拥堵指数 (默认25)
int Recv_timeout=20;       //接收超时(ms) (默认20)(作为整个程序运行的时间单位)(重要)
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

char *Send_tmp_data;        //待向下发送的数据
INFO Send_tmp_info;         //待向下发送的数据信息
char *Special_window_frame; //特殊发送窗口的数据
INFO Special_window_info;   //特殊发送窗口的数据信息 
char *Recv_tmp_data;        //待向上发送的数据
INFO Recv_tmp_info;         //待向上发送的数据信息

int Recv_buf_high_thershold=0.4*MAX_BUFFER_SIZE; //接收缓存较满判定阈值
int Recv_buf_low_thershold=0.25*MAX_BUFFER_SIZE; //接收缓存较空判定阈值
int Send_buf_high_thershold=0.4*MAX_BUFFER_SIZE; //发送缓存较满判定阈值
int Send_buf_low_thershold=0.25*MAX_BUFFER_SIZE; //发送缓存较空判定阈值
int Upper_full_thershold=25; //上层拥堵指数较大判定阈值
int Lower_full_thershold=25; //下层拥堵指数较大判定阈值

int Upper_send_stop=0;    //向上发送暂停周期数
int Lower_send_stop=0;    //向下发送暂停周期数
bool Upper_send_data=0;   //是否向上层发送了数据信息
bool Lower_send_data=0;   //是否向下层发送了数据信息
bool Upper_recv_data=0;   //是否从上层接收了数据信息
bool Lower_recv_data=0;   //是否从下层接收了数据信息
bool Recv_buf_no_empty=0; //上个周期接收缓存是否不空
bool Send_buf_no_empty=0; //上个周期发送缓存是否不空

int F_Print_mode=0;    //保证程序编译通过的无用变量声明 
const int HEAD_SIZE=0; //保证程序编译通过的无用变量声明 

void shutdown(){//退出
	WSACleanup();
	if(Tmp_buf) delete [] Tmp_buf;
	if(Special_window_frame) delete [] Special_window_frame;
	if(Recv_tmp_data) delete [] Recv_tmp_data;
	if(Send_tmp_data) delete [] Send_tmp_data;
}
void tip(int type,int num){//提示
	if(type==0&&num==0){
		cprintf("#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#",94);
		cprintf("        网        络        层        ",94);
		cprintf("#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#\n",94);
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
				case 3:{cprintf("*发送：收到数据，已保存至发送缓存",3);
						print_void(86);printf("\n");break;}//T_Print_mode
				case 4:{cprintf("*发送：发送缓存达到上限，已丢弃即将发送的数据",3);
						print_void(74);printf("\n");break;}
				case 5:{cprintf("*发送：发送队列与缓存空间不匹配，已重置发送队列与缓存",3);
						print_void(66);printf("\n");break;}
				case 6:{cprintf("*接收：收到数据，已保存至接收缓存",3);
						print_void(86);printf("\n");break;}//T_Print_mode
				case 7:{cprintf("*接收：接收缓存达到上限，已丢弃接收的数据",3);
						print_void(78);printf("\n");break;}
				case 8:{cprintf("*接收：接收队列与缓存空间不匹配，已重置接收队列与缓存",3);
						print_void(66);printf("\n");break;}
				case 9:{cprintf("*内存分配：失败！",12);
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
			s2="NETWORK";
			s3=argv[3];
			break;
		}
		default:tip(1,1);
	}
	CCfgFileParms File(s1,s2,s3);
	Link_file=File;
	//读取本层IP与端口信息
	string info=Link_file.getUDPAddrString(CCfgFileParms::LOCAL,0);//本层
	String2Num(&Local_ip,&Local_port,info);
	//读取其他信息
	for(int i=0;i<=511;++i){
		Sub_net_ip[i]=Local_ip;
		Sub_net_port[i]=-1;
		Sub_net_next[i]=-1;
	}
	string S_Sub_net_ip,S_Sub_net_port,S_Sub_net_next,S_Router;
	Link_file.getValueInt(Sub_net,(char*)"subnet");//子网号
	Link_file.getValueInt(Top,(char*)"Top"); //是否为顶层
	Link_file.getValueInt(Print_mode,(char*)"Print_mode");    //收发信息打印模式
	Link_file.getValueInt(T_Print_mode,(char*)"T_Print_mode");//提示信息打印模式
	Link_file.getValueInt(Recv_timeout,(char*)"Recv_timeout");//接收超时
	Link_file.getValueInt(Upper_Send_timeout,(char*)"Upper_Send_timeout");//上层发送超时
	Link_file.getValueInt(Lower_Send_timeout,(char*)"Lower_Send_timeout");//下层发送超时
	Link_file.getValueInt(Sub_net_num,(char*)"subnet_num");//子网总数
	S_Sub_net_ip=Link_file.getValueStr("subnet_ip");    //子网对应真实IP (int型表示)
	S_Sub_net_port=Link_file.getValueStr("subnet_port");//子网对应真实端口
	S_Sub_net_next=Link_file.getValueStr("subnet_next");//子网出口
	S_Router=Link_file.getValueStr("router");//子网出口是否为路由器
	if(S_Sub_net_ip.length()>0) Many_String2Num(S_Sub_net_ip,Sub_net_ip,Sub_net_num);
	if(S_Sub_net_port.length()>0) Many_String2Num(S_Sub_net_port,Sub_net_port,Sub_net_num);
	if(S_Sub_net_next.length()>0) Many_String2Num(S_Sub_net_next,Sub_net_next,Sub_net_num);
	if(S_Router.length()>0) Many_String2Num(S_Router,Router,Sub_net_num);
	//下层实体数量
	Lower_number=Link_file.getUDPAddrNumber(CCfgFileParms::LOWER);
	if(Lower_number<0) tip(1,1);
	tip(0,1);
	
	//缓存
	Tmp_buf=new char[MAX_BUFFER_SIZE]; //临时缓存
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
		Upper_addr=Link_file.getUDPAddr(CCfgFileParms::UPPER,0);//自动配置
		Link_file.getValueInt(Upper_mode,(char*)"Upper_mode");//接口模式
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
	Link_file.print(13);
}
void Send_Upper(int i){//向上层发送数据，输入为窗口类型
	//注意：窗口号为-1时代表使用特殊窗口(专用于收发控制信息)
	char *Recv_data;
	int buf_len;
	
	if(i!=-1){//普通数据
		Recv_data=Recv_tmp_data;
		  buf_len=Recv_tmp_info.buf_len;
	}
	else{//控制信息
		Recv_data=Special_window_frame;
		  buf_len=16;
	}
	
	//信息预处理
	int len=buf_len/8+(buf_len>0&&buf_len%8!=0);
	char byte[len];
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
				if(Print_mode==1) print_bit(Recv_data,buf_len,"向上层发送数据失败...数据bit流：",0,13);
				else print_byte(byte,len,"向上层发送数据失败...数据字节流：",1,13);
			}
			else{
				if(Print_mode==1) print_bit(Recv_data,buf_len,"向上层发送控制信息失败...其bit流：",0,13);
				else print_byte(byte,len,"向上层发送控制信息失败...其字节流：",1,13);
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
				if(Print_mode==1) print_bit(Recv_data,buf_len,"向上层发送数据成功！数据bit流：",0,13);
				else print_byte(byte,len,"向上层发送数据成功！数据字节流：",1,13);
			}
			else{
				if(Print_mode==1) print_bit(Recv_data,buf_len,"向上层发送控制信息成功！其bit流：",0,13);
				else print_byte(byte,len,"向上层发送控制信息成功！其字节流：",1,13);
			}
		}
		if(i!=-1) delete[] Recv_tmp_data;
		else delete[] Special_window_frame;
	}
}
void Send_Lower(int i){//向下层发送数据，输入为窗口类型
	//注意：窗口号为-1时代表使用特殊窗口(专用于收发控制信息)
	char *Send_data;
	int buf_len,Low_num;
	
	if(i!=-1){//普通数据
		Send_data=Send_tmp_data;
		  buf_len=Send_tmp_info.buf_len;
		  Low_num=Send_tmp_info.Lower_number;
	}
	else{//控制信息
		Send_data=Special_window_frame;
		  buf_len=16;
		  Low_num=Special_window_info.Lower_number;
	}
	
	//信息预处理
	int len=buf_len/8+(buf_len>0&&buf_len%8!=0);
	char byte[len+6];
	if(i!=-1){//普通数据
		Bit2Byte(Send_data,buf_len,byte+6,len);//先将发送帧转为字节型
		
		Destination_subnet=byte[9];
		Destination_ip=Sub_net_ip[Destination_subnet];
		if((Destination_subnet!=Sub_net)&&(Router[Destination_subnet])){//要传输的数据不在子网内，且自己的下一跳是路由器
			Destination_port=Sub_net_port[Destination_subnet];
		}
		else{//要传输的数据在子网内
			if(*((unsigned short*)(byte+10))!=65535) Destination_port=*((unsigned short*)(byte+10))-100;
			else Destination_port=65535;
		}
		
		set_destination_mac_info(Destination_ip,Destination_port,byte);//转化
//		cout<<Destination_ip<<" # "<<Destination_port<<endl;
		
		len+=6;
	}
	else//控制信息
		Bit2Byte(Send_data,buf_len,byte,len);//先将发送帧转为字节型
	
	//发送
	int Send_len=sendto(Local_sock,byte,len,0,(SOCKADDR*)&Lower_addr[Low_num],SOCKADDR_SIZE);
	if(Send_len<=0){//发送失败，则准备重传
		Send_amount.Error+=buf_len; Send_time.Error++;
		if(i!=-1){//只重传数据帧，不重传控制帧
			Send_buf+=buf_len;             //增加缓存 
			Send_queue.push(Send_tmp_data);//添加帧至队列末
			Send_info.push(Send_tmp_info); //添加帧信息至队列末
		}
		
		if(Print_mode){//输出消息
			if(i!=-1){
				if(Print_mode==1) print_bit(Send_data,buf_len,"向下层发送数据失败...数据bit流：",0,13);
				else print_byte(byte,len,"向下层发送数据失败...数据字节流：",1,13);
			}
			else{
				if(Print_mode==1) print_bit(Send_data,buf_len,"向下层发送控制信息失败...其bit流：",0,13);
				else print_byte(byte,len,"向下层发送控制信息失败...其字节流：",1,13);
			}
		}
		if(i==-1) delete[] Special_window_frame;
	}
	else{//发送成功
		Send_amount.Total+=buf_len; Send_time.Total++;
		Send_amount.Lower+=buf_len; Send_time.Lower++;
		if(i!=-1) {Send_amount.LData+=buf_len; Send_time.LData++; Lower_send_data=1;}
		if(i==-1) {Send_amount.LCtrl+=buf_len; Send_time.LCtrl++;}
		
		if(Print_mode){//输出消息
			if(i!=-1){
				if(Print_mode==1) print_bit(Send_data,buf_len,"向下层发送数据成功！数据bit流：",0,13);
				else print_byte(byte,len,"向下层发送数据成功！数据字节流：",1,13);
			}
			else{
				if(Print_mode==1) print_bit(Send_data,buf_len,"向下层发送控制信息成功！其bit流：",0,13);
				else print_byte(byte,len,"向下层发送控制信息成功！其字节流：",1,13);
			}
		}
		if(i!=-1) delete[] Send_tmp_data;
		else delete[] Special_window_frame;
	}
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
	
	//获取下发的目标实体号
	get_destination_ip_info(&Destination_subnet,&Destination_port,Tmp_buf,len);
	
	int Low_num=Sub_net_next[Destination_subnet];
	if(Low_num==-1) Low_num=0;
	
	//将数据加至发送缓存
	if(Send_buf+8*len>MAX_BUFFER_SIZE){
		Recv_amount.Error+=8*len; Recv_time.Error++;
		tip(2,4);
	}
	else{
		Upper_recv_data=1;
		Recv_amount.Total+=8*len; Recv_time.Total++;
		Recv_amount.Upper+=8*len; Recv_time.Upper++;
		Recv_amount.UData+=8*len; Recv_time.UData++;
		if(T_Print_mode) tip(2,3);
		
		char *tmp_frame;
		try{tmp_frame=new char[8*len+48];}catch(bad_alloc){tip(2,9);return;}
		Byte2Bit(tmp_frame,8,(char*)&Sub_net,1);//头部成帧
		Byte2Bit(tmp_frame+8,16,(char*)&Local_port,2);//头部成帧
		Byte2Bit(tmp_frame+24,8,(char*)&Destination_subnet,1);//头部成帧
		Byte2Bit(tmp_frame+32,16,(char*)&Destination_port,2);//头部成帧
		Byte2Bit(tmp_frame+48,8*len,Tmp_buf,len);//先转换为bit型，再添加
//		cout<<"@@@"<<Sub_net<<" "<<Local_port<<" "<<Destination_subnet<<" "<<Destination_port<<endl;
		len+=6;
		
		Send_buf+=8*len;           //缓冲区占用大小
		Send_queue.push(tmp_frame);//添加至发送队列
		
		INFO tmp_info(-1,len,8*len,Low_num,-1,0);
		Send_info.push(tmp_info);  //添加发送信息至队列
	}
	
	//判断缓存空间，并发送控制信息
	if(Send_buf>Send_buf_high_thershold){
		char *control;
		try{control=new char[16];}catch(bad_alloc){tip(2,9);return;}
		set_control(control,Send_buf,Send_buf_high_thershold);
		Special_window_frame=control;
		Send_Upper(-1);
	}
}
void Recv_Lower(int len,int i){//从下层收到
	if(len==2&&(unsigned char)*Tmp_buf==0xff){//如果是控制信息，则调整同层拥堵指数
		Recv_amount.Total+=16; Recv_time.Total++;
		Recv_amount.Lower+=16; Recv_time.Lower++;
		Recv_amount.LCtrl+=16; Recv_time.LCtrl++;
		char n=*(Tmp_buf+1);
		if(n==0) Lower_full=0;
		else if(n>0) Lower_full+=n;
		return;
	}//否则认为是数据信息，正常传输
	
	//将数据加至接收缓存
	if(Recv_buf+8*len>MAX_BUFFER_SIZE){
		Recv_amount.Error+=8*len; Recv_time.Error++;
		tip(2,7);
	}
	else{
		Lower_recv_data=1;
		Recv_amount.Total+=8*len; Recv_time.Total++;
		Recv_amount.Lower+=8*len; Recv_time.Lower++;
		Recv_amount.LData+=8*len; Recv_time.LData++;
		if(T_Print_mode) tip(2,6);
		
		int Source_mac_port=0;
		get_source_mac_info(&Source_ip,&Source_mac_port,&Sequence,Tmp_buf,len);
		len-=9;
		
		Source_subnet=*(Tmp_buf+9);
		Source_port=*((unsigned short*)(Tmp_buf+10));
		Destination_subnet=*(Tmp_buf+12);
		Destination_port=*((unsigned short*)(Tmp_buf+13));
//		cout<<Source_ip<<" "<<Source_mac_port<<" "<<Source_subnet<<" @ "<<Destination_port<<" "<<Destination_subnet<<endl;
		
		int Low_num=Sub_net_next[Destination_subnet];
//		cout<<Low_num<<" # "<<Source_mac_port<<endl;
		if((Low_num==-1)||(Source_mac_port==65535)){//自己就是目的子网，向上层发
			
			char *tmp_frame;
			try{tmp_frame=new char[8*len];}catch(bad_alloc){tip(2,9);return;}
			
			set_source_ip_info(Source_subnet,Source_port,-1,Tmp_buf+9);
			Byte2Bit(tmp_frame,8*len,Tmp_buf+9,len);//先转换为bit型，再添加
			
			Recv_buf+=8*len;           //缓冲区占用大小
			Recv_queue.push(tmp_frame);//添加至接收队列
			
			INFO tmp_info(-1,len,8*len,-1,-1,0);
			Recv_info.push(tmp_info);  //添加接收信息至队列
		}
		else if(Low_num==i)//应该是网内的信息，不该传出网外，直接返回
			return;
		else{//向下一个接口转发
			char *tmp_frame;
			try{tmp_frame=new char[8*len];}catch(bad_alloc){tip(2,9);return;}
			Byte2Bit(tmp_frame,8*len,Tmp_buf+9,len);//先转换为bit型，再添加
			
			Send_buf+=8*len;           //缓冲区占用大小
			Send_queue.push(tmp_frame);//添加至发送队列
			
			INFO tmp_info(-1,len,8*len,Low_num,-1,0);
			Send_info.push(tmp_info);  //添加发送信息至队列
		}
	}
	
	//判断缓存空间，并发送控制信息
	if(Recv_buf>Recv_buf_high_thershold){
		char *control;
		try{control=new char[16];}catch(bad_alloc){tip(2,9);return;}
		set_control(control,Recv_buf,Recv_buf_high_thershold);
		Special_window_frame=control;
		INFO tmp_info(-1,-1,-1,i,-1,3);
		Special_window_info=tmp_info;
		Send_Lower(-1);
	}
}
int main(int argc,char *argv[]){
	//初始化
	tip(0,0);
	initial(argc,argv);
	
	//循环接收、发送数据
	SOCKADDR_IN Remote_addr;              //用于存储接收到的套接字
	register bool event_happen=1;         //是否有事件发生(用于判断是否打印数据信息)
	register int tmp;                     //存储从套接字接收到的数据长度
	register unsigned short Upper_wait=0; //向上层发送的等待时间
	register unsigned short Lower_wait=0; //向下层发送的等待时间
	register unsigned long long cycle=0;  //循环周期数
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
				if(!Top&&compare_sock(Remote_addr,Upper_addr)){//非路由器，上层
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
			Lower_wait+=Recv_timeout;
			while(Lower_wait>=Lower_Send_timeout){
				Lower_wait-=Lower_Send_timeout;
				if(Send_buf){
					if(Send_queue.empty()||Send_info.empty()){//发生错误，缓存不为空，但队列为空
						tip(2,5);
						Send_buf=0;                                 //重置缓存
						while(!Send_queue.empty()) Send_queue.pop();//清空队列
						while(!Send_info.empty()) Send_info.pop();  //清空队列
					}
					else{//无错误，正常传输
						event_happen=1;
						
						Send_tmp_data=Send_queue.front();//读取待发送数据
						Send_tmp_info=Send_info.front(); //读取待发送数据信息
						
						Send_buf-=Send_tmp_info.buf_len;//释放空间
						Send_queue.pop();//出队
						Send_info.pop(); //出队
						
						Send_Lower(0);//发送
					}
				}
				else if((!Send_queue.empty())||(!Send_info.empty())){//发生错误，缓存为空，但队列不为空
					tip(2,5);
					Send_buf=0;                                 //重置缓存
					while(!Send_queue.empty()) Send_queue.pop();//清空队列
					while(!Send_info.empty()) Send_info.pop();  //清空队列
				}
			}
		}
		
		//尝试向上发送数据
		if(Upper_send_stop) Upper_send_stop--;
		else if(!Top){//非路由器
			Upper_wait+=Recv_timeout;
			while(Upper_wait>=Upper_Send_timeout){
				Upper_wait-=Upper_Send_timeout;
				if(Recv_buf){
					if(Recv_queue.empty()||Recv_info.empty()){//发生错误，缓存不为空，但队列为空
						tip(2,8);
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
					tip(2,8);
					Recv_buf=0;                                 //重置缓存
					while(!Recv_queue.empty()) Recv_queue.pop();//清空队列
					while(!Recv_info.empty()) Recv_info.pop();  //清空队列
				}
			}
		}
		
		//更改流量控制信息
		if(!Top){//非路由器
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
			//3.下层相关
			if(Lower_full>=100){//下层发送超时
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
			if(Recv_buf_no_empty&&(Recv_buf==0)){//接收缓存由满转空，发送清零控制信息
				char *control;
				try{control=new char[16];}catch(bad_alloc){tip(2,16);}
				set_control(control,-1,0);
				Special_window_frame=control;
				Send_Upper(-1);
			}
			if(Send_buf_no_empty&&(Send_buf==0)){//发送缓存由满转空，发送清零控制信息
				for(int i=0;i<=Lower_number-1;++i){
					char *control;
					try{control=new char[16];}catch(bad_alloc){tip(2,16);}
					set_control(control,-1,0);
					Special_window_frame=control;
					INFO tmp_info(-1,-1,-1,i,-1,-1);
					Special_window_info=tmp_info;
					Send_Lower(-1);
				}
			}
			Upper_send_data=0;         //重置向上发送数据判断值
			Lower_send_data=0;         //重置向下发送数据判断值
			Upper_recv_data=0;         //重置从上接收数据判断值
			Lower_recv_data=0;         //重置从下接收数据判断值
			Recv_buf_no_empty=Recv_buf;//更新接收缓存状态判断值
			Send_buf_no_empty=Send_buf;//更新发送缓存状态判断值
		}
		else{//路由器
			//1.拥堵指数
			if(Lower_send_data&&(Send_buf>Send_buf_low_thershold)&&(Send_buf<=Send_buf_high_thershold)){//下层拥堵指数
				event_happen=1;
				Lower_full--;
			}
			if(Lower_recv_data&&(Send_buf>Send_buf_high_thershold)){//上层拥堵指数
				event_happen=1;
				Lower_full--;
			}
			//2.下层相关
			if(Lower_full>=100){//下层发送超时
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
			//3.本层相关
			if(Send_buf_no_empty&&(Send_buf==0)){//发送缓存由满转空，发送清零控制信息
				for(int i=0;i<=Lower_number-1;++i){
					char *control;
					try{control=new char[16];}catch(bad_alloc){tip(2,16);}
					set_control(control,-1,0);
					Special_window_frame=control;
					INFO tmp_info(-1,-1,-1,i,-1,-1);
					Special_window_info=tmp_info;
					Send_Lower(-1);
				}
			}
			Lower_send_data=0;         //重置向下发送数据判断值
			Lower_recv_data=0;         //重置从下接收数据判断值
			Send_buf_no_empty=Send_buf;//更新发送缓存状态判断值
		}
		
		//打印统计信息
		if(event_happen){
			print_data(Recv_timeout,Upper_Send_timeout,Lower_Send_timeout,0,0,\
			           Upper_full,Lower_full,Send_buf,Recv_buf,0,0,0,\
			           &Send_amount,&Send_time,&Recv_amount,&Recv_time,2,13);
			event_happen=0;
		}
	}
}
