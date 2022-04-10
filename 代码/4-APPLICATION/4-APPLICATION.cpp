//4 APPLICATION Layer(应用层)
#include<iostream>
#include<cstdio>
#include<cstdlib>
#include<cmath>
#include<queue>
#include<winsock2.h>
#include<windows.h>
#include<conio.h>
#include"CfgFileParms.h"
#include"BasicFunction.h"
//#pragma comment(lib,"ws2_32.lib")
using namespace std;

CCfgFileParms Link_file;               //网元信息
const int MAX_BUFFER_SIZE=65000;       //缓存空间(bit)
int SOCKADDR_SIZE=sizeof(SOCKADDR_IN); //套接字地址长度(字节)

WSADATA     Local_data; //本层初始化信息
SOCKET      Local_sock; //本层套接字
SOCKADDR_IN Local_addr; //本层套接字地址
SOCKADDR_IN Lower_addr; //下层套接字地址(网络层)
SOCKADDR_IN Cmd_addr;   //管理层套接字地址

int Sub_net=0;  //本层虚拟子网号
int Local_ip;   //本层真实IP
int Local_port; //本层真实端口

string Destination_ip;     //目的IP地址 (默认为子网广播地址)
int Destination_ip_subnet; //目的IP的子网 (默认Sub_net)
int Destination_ip_port;   //目的IP的真实端口 (默认65535)

int Source_ip_num=0;                  //来源IP的总数
int Source_ip_subnet[128];            //来源IP的子网
int Source_ip_port[128];              //来源IP的真实端口
bool Source_ip_seq[128][4194304];     //来源IP的帧序号统计
int Source_ip_max_seq[128];           //来源IP的帧序号结束值(最大值)
priority_queue<FRAME*>Recv_data[512]; //收到的数据

int Print_mode=0;   //本层收发信息打印模式，0为不打印收发的内容，1为打印bit流，2为打印8进制 (默认1)
int F_Print_mode=0; //本层数据信息打印模式，0为不打印数据信息，1为打印 (默认0)
int T_Print_mode=0; //本层提示信息打印模式，0为只打印发生错误时的提示信息，1增加打印收发处理信息，2增加打印流量控制信息 (默认0)
int Lower_mode=1;   //下层接口模式，0为bit数组，1为字节数组 (默认1)
int Lower_number=1; //下层实体数量 (默认1)(最大为8)

int Auto_full=25;          //自动发送拥堵指数 (默认25)
int Lower_full=25;         //下层拥堵指数 (默认25)
int Recv_timeout=20;       //接收超时(ms) (默认20)(作为整个程序运行的时间单位)(重要)
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

bool Auto_send=0;      //是否自动发送
int Auto_send_mode=0;  //自动发送模式。0为忽视控制信息，始终以自动发送速率发送；1为根据控制信息实时调整速率，可能会适当降低
int Auto_send_time=20; //自动发送超时，在自动发送模式为0时使用
int Auto_send_len=15;  //自动发送长度 (字节型)(默认15)(最大65000)
char *Auto_send_data;  //自动发送的数据 (字节型)(为01010101的循环)

int Send_buf_high_thershold=0.4*MAX_BUFFER_SIZE; //发送缓存较满判定阈值
int Send_buf_low_thershold=0.25*MAX_BUFFER_SIZE; //发送缓存较空判定阈值
int Auto_full_thershold=25;  //自动发送拥堵指数较大判定阈值
int Lower_full_thershold=25; //下层拥堵指数较大判定阈值

int Auto_send_stop=0;     //自动发送暂停周期数
int Lower_send_stop=0;    //向下发送暂停周期数
bool Lower_send_data=0;   //是否向下层发送了数据信息
bool Auto_recv_data=0;    //是否自动接收了数据信息
bool Lower_recv_data=0;   //是否从下层接收了数据信息
bool Send_buf_no_empty=0; //上个周期发送缓存是否不空

int Recv_data_seq;           //接收到的最大数据帧序号 (上限为131072)
char Recv_data_time[131072]; //接收到的数据帧序号次数统计

const int HEAD_SIZE=0; //保证程序编译通过的无用变量声明 

void shutdown(){//退出
	WSACleanup();
	if(Tmp_buf) delete [] Tmp_buf;
	if(Special_window_frame) delete [] Special_window_frame;
	if(Send_tmp_data) delete [] Send_tmp_data;
	if(Auto_send_data) delete [] Auto_send_data;
}
void tip(int type,int num){//提示
	if(type==0&&num==0){
		cprintf("#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#",46);
		cprintf("        应        用        层        ",46);
		cprintf("#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#\n",46);
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
				case 10:{cprintf("*已开启自动发送！",3);
						print_void(102);printf("\n");break;}
				case 11:{cprintf("*已关闭自动发送！",3);
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
			s2="APPLICATION";
			s3=argv[3];
			break;
		}
		default:tip(1,1);
	}
	CCfgFileParms File(s1,s2,s3);
	Link_file=File;
	Link_file.getValueInt(Sub_net,(char*)"Sub_net");//子网号
	Link_file.getValueInt(Print_mode,(char*)"Print_mode");    //收发信息打印模式
	Link_file.getValueInt(T_Print_mode,(char*)"T_Print_mode");//提示信息打印模式
	Link_file.getValueInt(Recv_timeout,(char*)"Recv_timeout");//接收超时
	Link_file.getValueInt(Lower_Send_timeout,(char*)"Lower_Send_timeout");//下层发送超时
	Link_file.getValueInt(Auto_send_time,(char*)"Auto_send_time");//自动发送超时
	Link_file.getValueInt(Auto_send_mode,(char*)"Auto_send_mode");//自动发送模式
	Link_file.getValueInt(Auto_send_len,(char*)"Auto_send_len");  //自动发送长度
	string Destination_ip=Link_file.getValueStr("Destination_ip");//目的地址
	int num_tmp[2]={Sub_net,65535};
	if(Destination_ip.length()>0) Many_String2Num(Destination_ip,num_tmp,2);
	Destination_ip_subnet=num_tmp[0];
	Destination_ip_port=num_tmp[1];
	//读取本层IP与端口信息
	string info=Link_file.getUDPAddrString(CCfgFileParms::LOCAL,0);//本层
	String2Num(&Local_ip,&Local_port,info);
	tip(0,1);
	
	//缓存
	Tmp_buf=new char[MAX_BUFFER_SIZE];//临时缓存
	Auto_send_data=new char[MAX_BUFFER_SIZE];//自动发送的数据
	if(Auto_send_data) memset(Auto_send_data,85,MAX_BUFFER_SIZE);
	tip((!Tmp_buf)||(!Auto_send_data),2);
	
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
		//下层套接字
		Lower_addr=Link_file.getUDPAddr(CCfgFileParms::LOWER,0);//自动配置
		Link_file.getValueInt(Lower_mode,(char*)"Lower_mode");//接口模式
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
	Link_file.print(10);
	cprintf("按回车键 开启/关闭 自动发送，按\"`\"键重设目标IP地址\n",10);
}
bool check_end_frame(char* data){//判断一个帧是否为结束帧
	//返回值：1为是，0为否
	
	
	return 0;
}
void Send_Lower(int i){//向下层发送数据，输入为窗口类型
	//注意：窗口号为-1时代表使用特殊窗口(专用于收发控制信息)，为1时代表自动发送
	char *Send_data;
	int buf_len;
	
	switch(i){
		case -1:{//控制信息
			Send_data=Special_window_frame;
			  buf_len=16;
			  break;
		}
		case 1:{//自动发送
			Send_data=Auto_send_data;
			  buf_len=Auto_send_len*8;
			  break;
		}
		default:{//普通数据
			Send_data=Send_tmp_data;
			  buf_len=Send_tmp_info.buf_len;
			  break;
		}
	}
	
	int Send_len,len=buf_len/8+(buf_len>0&&buf_len%8!=0);
	char byte[len+3];
	if(i!=1){//非自动发送 
		//先将发送帧转为字节型，再发送
		Bit2Byte(Send_data,buf_len,byte+3,len);
		set_destination_ip_info(Destination_ip_subnet,Destination_ip_port,byte);
		len+=3;
		Send_len=sendto(Local_sock,byte,len,0,(SOCKADDR*)&Lower_addr,SOCKADDR_SIZE);
	}
	else{//自动发送 
		len=Auto_send_len+3;
		set_destination_ip_info(Destination_ip_subnet,Destination_ip_port,Auto_send_data);
		Send_len=sendto(Local_sock,Auto_send_data,len,0,(SOCKADDR*)&Lower_addr,SOCKADDR_SIZE);
	}
//	cout<<Destination_ip_subnet<<" * "<<Destination_ip_port<<endl;
	
	if(Send_len<=0){//发送失败，则准备重传
		Send_amount.Error+=buf_len; Send_time.Error++;
		if(i!=-1&&i!=1){//只重传数据帧，不重传控制帧和自动发送的帧
			Send_buf+=buf_len;             //增加缓存 
			Send_queue.push(Send_tmp_data);//添加帧至队列末
			Send_info.push(Send_tmp_info); //添加帧信息至队列末
		}
		
		if(Print_mode){//输出消息
			if(i==-1){
				if(Print_mode==1) print_bit(Send_data,buf_len,"向下层发送控制信息失败...其bit流：",0,10);
				else print_byte(byte,len,"向下层发送控制信息失败...其字节流：",1,10);
			}
			else{
				if(Print_mode==1){
					if(i!=1) print_bit(Send_data,buf_len,"向下层发送数据失败...数据bit流：",0,10);
					else print_bit(Send_data,len,"向下层发送数据失败...数据bit流：",1,10);
				}
				else{
					if(i!=1) print_byte(byte,len,"向下层发送数据失败...数据字节流：",1,10);
					else print_byte(Send_data,len,"向下层发送数据失败...数据字节流：",1,10);
				}
			}
		}
		if(i==-1) delete[] Special_window_frame;
	}
	else{//发送成功
		Send_amount.Total+=buf_len; Send_time.Total++;
		Send_amount.Lower+=buf_len; Send_time.Lower++;
		switch(i){
			case -1: {Send_amount.LCtrl+=buf_len; Send_time.LCtrl++;break;}//控制信息
			case  1: {Send_amount. Auto+=buf_len; Send_time. Auto++; Lower_send_data=1;break;}//自动发送
			default: {Send_amount.LData+=buf_len; Send_time.LData++; Lower_send_data=1;break;}//普通数据
		}
		
		if(Print_mode){//输出消息
			if(i==-1){
				if(Print_mode==1) print_bit(Send_data,buf_len,"向下层发送控制信息成功！其bit流：",0,10);
				else print_byte(byte,len,"向下层发送控制信息成功！其字节流：",1,10);
			}
			else{
				if(Print_mode==1){
					if(i!=1) print_bit(Send_data,buf_len,"向下层发送数据成功！数据bit流：",0,10);
					else print_bit(Send_data,len,"向下层发送数据成功！数据bit流：",1,10);
				}
				else{
					if(i!=1) print_byte(byte,len,"向下层发送数据成功！数据字节流：",1,10);
					else print_byte(Send_data,len,"向下层发送数据成功！数据字节流：",1,10);
				}
			}
		}
		if(i==-1) delete[] Special_window_frame;
		else if(i!=1) delete[] Send_tmp_data;
	}
}
int Send_Cmd(char *buf,int len){//向管理层发送数据，返回发送长度
	return sendto(Local_sock,buf,len,0,(SOCKADDR*)&Cmd_addr,SOCKADDR_SIZE);
}
void Recv_Lower(int len){//从下层收到
	if(len==2&&(unsigned char)*Tmp_buf==0xff){//如果是控制信息，则调整同层拥堵指数
		Recv_amount.Total+=16; Recv_time.Total++;
		Recv_amount.Lower+=16; Recv_time.Lower++;
		Recv_amount.LCtrl+=16; Recv_time.LCtrl++;
		if(!(Auto_send&&(Auto_send_mode==0))){//如果不是采用 模式0的自动发送 进行发送，则调整
			char n=*(Tmp_buf+1);
			if(n==0) Lower_full=0;
			else if(n>0) Lower_full+=n;
		}
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
		
		int ip_subnet=0,ip_port=0,seq=0;
		get_source_ip_info(&ip_subnet,&ip_port,&seq,Tmp_buf,len);
		char *data=Tmp_buf+6;
		
		
		//以下为文件输入输出部分，由于时间有限，尚未完成，但思路已经较为清晰
		if(seq==-1){//是广播
			
		}
		else{
			if(seq==0){//是起始帧
				//分配队列，判定文件类型，记录文件名称
				
			}
			else{
				if(check_end_frame(data)){//是结束帧
					//设置标志，队列序号不再增长，每接收到帧判断是否要输出队列至文件
					
				}
				else{//是数据帧
					//寻找对应队列，并入队
					
				}
			}
		}
		
/*		char *tmp_frame;
		try{tmp_frame=new char[8*len];}catch(bad_alloc){tip(2,9);return;}
		Byte2Bit(tmp_frame,8*len,Tmp_buf,len);//先转换为bit型，再添加
		
		Recv_buf+=8*len;           //缓冲区占用大小
		Recv_queue.push(tmp_frame);//添加至接收队列
		
		INFO tmp_info(0,len,8*len,0,0);
		Recv_info.push(tmp_info);  //添加接收信息至队列
*/	}
}
int main(int argc,char *argv[]){
	//初始化
	tip(0,0);
	initial(argc,argv);
	
	//循环接收、发送数据
	SOCKADDR_IN Remote_addr;                  //用于存储接收到的套接字
	register bool event_happen=1;             //是否有事件发生(用于判断是否打印数据信息)
	register int tmp;                         //存储从套接字接收到的数据长度
	register unsigned short Auto_send_wait=0; //自动发送的等待时间
	register unsigned short Auto_recv_wait=0; //自动接收的等待时间
	register unsigned short Lower_wait=0;     //向下层发送的等待时间
	register unsigned long long cycle=0;      //循环周期数
	while(++cycle){
//		printf(" $%d$ ",cycle);
		//检测键盘输入，开启/关闭自动发送
		if(kbhit()){
			char tmp=getch();
			if(tmp==13){
				event_happen=1;
				Auto_send=!Auto_send;
				if(Auto_send==0) tip(2,11);
				else tip(2,10);
			}
			if(tmp=='`'){
				event_happen=1;
				cprintf("请输入新的目的IP地址：",10);
				HANDLE gh_std_out=GetStdHandle(STD_OUTPUT_HANDLE);
				CONSOLE_SCREEN_BUFFER_INFO bInfo;
				GetConsoleScreenBufferInfo(gh_std_out,&bInfo);//获取光标位置 
				print_void(97);printf("\n");
				goto_xy(bInfo.dwCursorPosition.X,bInfo.dwCursorPosition.Y);//光标返回 
				cin>>Destination_ip;
				String2Num(&Destination_ip_subnet,&Destination_ip_port,Destination_ip);
//				cout<<Destination_ip<<"  "<<Destination_ip_subnet<<"  "<<Destination_ip_port;
			}
		}
		
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
				if(compare_port(Remote_addr,Lower_addr)){//下层
					unknown=0;
					Recv_Lower(tmp);
				}
				if(unknown){//来源未知
					Recv_amount.Unknown+=tmp; Recv_time.Unknown++;
				}
			}
		}
		
		//尝试向下发送数据
		if(Auto_send){//如果开启了自动发送，则进行自动发送操作
			Auto_send_wait+=Recv_timeout;
			if(Auto_send_mode==0){//模式0，速度恒定
				while(Auto_send_wait>=Auto_send_time){
					event_happen=1;
					Auto_send_wait-=Auto_send_time;
					Send_Lower(1);
				}
			}
			else{//模式1，速度动态调整
				Auto_recv_wait+=Recv_timeout;
				if(Auto_send_stop) Auto_send_stop--;
				else{
					while(Auto_recv_wait>=Auto_send_time){
						event_happen=1;
						Auto_recv_data=1;
						Auto_recv_wait-=Auto_send_time;
						Send_buf+=8*Auto_send_len;
						if(Send_buf>Send_buf_high_thershold){
							char control[16],byte[2];
							set_control(control,Send_buf,Send_buf_high_thershold);
							Bit2Byte(control,16,byte,2);
							if(byte[1]==0) Auto_full=0;
							else if(byte[1]>0) Auto_full+=byte[1];
						}
					}
				}
				while(Auto_send_wait>=Lower_Send_timeout&&Send_buf>0){
					event_happen=1;
					Auto_send_wait-=Lower_Send_timeout;
					Send_buf-=8*Auto_send_len;
					Send_Lower(1);
				}
			}
		}
		else{//从文件读入，然后发送
			if(Lower_send_stop) Lower_send_stop--;
			else{
				Lower_wait+=Recv_timeout;
				while(Lower_wait>=Lower_Send_timeout){
					Lower_wait-=Lower_Send_timeout;
					if(Send_buf){
						if(Send_queue.empty()||Send_info.empty()){//发生错误，缓存不为空，但队列为空
/*							tip(2,5);
							Send_buf=0;                                 //重置缓存
							while(!Send_queue.empty()) Send_queue.pop();//清空队列
							while(!Send_info.empty()) Send_info.pop();  //清空队列
*/						}
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
/*					else if((!Send_queue.empty())||(!Send_info.empty())){//发生错误，缓存为空，但队列不为空
						tip(2,5);
						Send_buf=0;                                 //重置缓存
						while(!Send_queue.empty()) Send_queue.pop();//清空队列
						while(!Send_info.empty()) Send_info.pop();  //清空队列
					}*/
				}
			}
		}
		
		//更改流量控制信息
		if(!(Auto_send&&(Auto_send_mode==0))){//如果不是采用 模式0的自动发送 进行发送，则调整
			//1.拥堵指数
			if(Lower_send_data&&(Send_buf>Send_buf_low_thershold)&&(Send_buf<=Send_buf_high_thershold)){//下层拥堵指数
				event_happen=1;
				Lower_full--;
			}
			if(Auto_recv_data&&(Send_buf>Send_buf_high_thershold)){//下层拥堵指数
				event_happen=1;
				Lower_full--;
			}
			if(Auto_send&&Send_buf<Send_buf_high_thershold){//自动发送拥堵指数
				event_happen=1;
				Auto_full--;
			}
			//2.自动发送相关
			if(Auto_full>=100){//自动发送超时
				event_happen=1;
				double k=log(100-Auto_full_thershold)/log(2);
				int time=ceil((double)Recv_timeout*0.05*k);
				if(Auto_send_time+time<=1000) Auto_send_time+=time;
				Auto_full=3;
				Auto_send_stop=3;
			}
			else if(Auto_full>Auto_full_thershold){
				event_happen=1;
				double k=log(Auto_full-Auto_full_thershold)/log(2);
				int time=ceil((double)Recv_timeout*0.05*k);
				if(Auto_send_time+time<=1000) Auto_send_time+=time;
				Auto_full=3;
			}
			else if(Auto_full<=0){
				event_happen=1;
				Auto_full=Auto_full_thershold*0.5;
				int time=ceil((double)Auto_send_time*0.1);
				if(Auto_send_time-time>=5) Auto_send_time-=time;
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
			if(Send_buf_no_empty&&(Send_buf==0)){//发送缓存由满转空，发送清零控制信息
				for(int i=0;i<=Lower_number-1;++i){
					char *control;
					try{control=new char[16];}catch(bad_alloc){tip(2,16);}
					set_control(control,-1,0);
					Special_window_frame=control;
					Send_Lower(-1);
				}
			}
			Auto_recv_data=0;          //重置自动接收数据判断值
			Lower_send_data=0;         //重置向下发送数据判断值
			Lower_recv_data=0;         //重置从下接收数据判断值
			Send_buf_no_empty=Send_buf;//更新发送缓存状态判断值
		}
		
		//打印统计信息
		if(event_happen){//注意此处将Auto_send_time写到了Send_timeout的位置，将Auto_full写到了Upper_full的位置
			print_data(Recv_timeout,Auto_send_time,Lower_Send_timeout,0,0,\
			           Auto_full,Lower_full,Send_buf,Recv_buf,0,0,0,\
			           &Send_amount,&Send_time,&Recv_amount,&Recv_time,3,10);
			event_happen=0;
		}
	}
}
