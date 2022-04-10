//basic function of all layers
#include<cstdio>
#include<iostream>
#include<cstring>
#include<cmath>
#include<winsock2.h>
#include<windows.h>
#include"BasicFunction.h"
//#pragma comment (lib,"wsock32.lib")
using namespace std;

HEAD NULL_HEAD;//空结构体，用于特殊用途
INFO NULL_INFO;//空结构体，用于特殊用途

DATA::DATA(){memset(this,0,sizeof(DATA));}//统计信息构造函数
void print_data(int Recv_timeout,int Upper_Send_timeout,int Lower_Send_timeout,int R_Send_timeout,int Divide_len,\
                int Upper_full,int Lower_full,int Send_buf,int Recv_buf,int Send_window_size,int Send_window_busy_num,int Send_window_stop_num,\
                DATA *Send_amount,DATA *Send_time,DATA *Recv_amount,DATA *Recv_time,int layer,int color){//打印统计信息
	//layer代表层次信息，1为链路层，2为网络层，3为应用层
	HANDLE gh_std_out=GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO bInfo;
	GetConsoleScreenBufferInfo(gh_std_out,&bInfo);//获取光标位置 
	switch(layer){
		case 1:{//链路层
			cprintf("向上发送超时：",color);cprintf(Upper_Send_timeout,14);print_void(6-num_len(Upper_Send_timeout));
			cprintf("向下发送超时：",color);cprintf(Lower_Send_timeout,14);print_void(6-num_len(Lower_Send_timeout));
			cprintf("接收超时：",color);cprintf(Recv_timeout,14);print_void(6-num_len(Recv_timeout));
			cprintf("重传超时：",color);cprintf(R_Send_timeout,14);print_void(6-num_len(R_Send_timeout));
			cprintf("分帧长度：",color);cprintf(Divide_len,14);print_void(6-num_len(Divide_len));
			print_void(31);printf("\n");
			cprintf("上层拥堵指数：",color);cprintf(Upper_full,14);print_void(6-num_len(Upper_full));
			cprintf("下层拥堵指数：",color);cprintf(Lower_full,14);print_void(6-num_len(Lower_full));
			cprintf("接收缓存：",color);cprintf(Recv_buf,14);print_void(6-num_len(Recv_buf));
			cprintf("发送缓存：",color);cprintf(Send_buf,14);print_void(6-num_len(Send_buf));
			cprintf("发送窗口：",color);cprintf(Send_window_size,14);print_void(6-num_len(Send_window_size));
			cprintf("繁忙窗口：",color);cprintf(Send_window_busy_num,14);print_void(6-num_len(Send_window_busy_num));
			cprintf("暂停窗口：",color);cprintf(Send_window_stop_num,14);print_void(5-num_len(Send_window_stop_num));
			printf("\n");
			cprintf_data("接收：整体：总计",Recv_amount->Total,Recv_time->Total,0,13,0,color);
			cprintf_data(            "失败",Recv_amount->Error,Recv_time->Error,0,13,0,color);
			cprintf_data(            "未知",Recv_amount->Unknown,Recv_time->Unknown,1,13,39,color);
			cprintf_data("      上层：总计",Recv_amount->Upper,Recv_time->Upper,0,13,0,color);
			cprintf_data(            "数据",Recv_amount->UData,Recv_time->UData,0,13,0,color);
			cprintf_data(            "控制",Recv_amount->UCtrl,Recv_time->UCtrl,1,13,39,color);
			cprintf_data("      下层：总计",Recv_amount->Lower,Recv_time->Lower,0,13,0,color);
			cprintf_data(            "数据",Recv_amount->LData,Recv_time->LData,0,13,0,color);
			cprintf_data(            "误帧",Recv_amount->Wrong,Recv_time->Wrong,0,13,0,color);
			cprintf_data(             "ACK",Recv_amount->ACK  ,Recv_time->ACK  ,0,11,0,color);
			cprintf_data(             "NAK",Recv_amount->NAK  ,Recv_time->NAK  ,1,10,0,color);
			cprintf_data("发送：整体：总计",Send_amount->Total,Send_time->Total,0,13,0,color);
			cprintf_data(            "失败",Send_amount->Error,Send_time->Error,1,13,61,color);
			cprintf_data("      上层：总计",Send_amount->Upper,Send_time->Upper,0,13,0,color);
			cprintf_data(            "数据",Send_amount->UData,Send_time->UData,0,13,0,color);
			cprintf_data(            "控制",Send_amount->UCtrl,Send_time->UCtrl,1,13,39,color);
			cprintf_data("      下层：总计",Send_amount->Lower,Send_time->Lower,0,13,0,color);
			cprintf_data(            "数据",Send_amount->LData,Send_time->LData,0,13,0,color);
			cprintf_data(            "重传",Send_amount->Re   ,Send_time->Re   ,0,13,0,color);
			cprintf_data(             "ACK",Send_amount->ACK  ,Send_time->ACK  ,0,11,0,color);
			cprintf_data(             "NAK",Send_amount->NAK  ,Send_time->NAK  ,1,10,0,color);
			break;
		}
		case 2:{//网络层
			cprintf("向上发送超时：",color);cprintf(Upper_Send_timeout,14);print_void(6-num_len(Upper_Send_timeout));
			cprintf("向下发送超时：",color);cprintf(Lower_Send_timeout,14);print_void(6-num_len(Lower_Send_timeout));
			cprintf("接收超时：",color);cprintf(Recv_timeout,14);print_void(6-num_len(Recv_timeout));
			print_void(63);printf("\n");
			cprintf("上层拥堵指数：",color);cprintf(Upper_full,14);print_void(6-num_len(Upper_full));
			cprintf("下层拥堵指数：",color);cprintf(Lower_full,14);print_void(6-num_len(Lower_full));
			cprintf("接收缓存：",color);cprintf(Recv_buf,14);print_void(6-num_len(Recv_buf));
			cprintf("发送缓存：",color);cprintf(Send_buf,14);print_void(6-num_len(Send_buf));
			print_void(47);printf("\n");
			cprintf_data("接收：整体：总计",Recv_amount->Total,Recv_time->Total,0,13,0,color);
			cprintf_data(            "失败",Recv_amount->Error,Recv_time->Error,0,13,0,color);
			cprintf_data(            "未知",Recv_amount->Unknown,Recv_time->Unknown,1,13,39,color);
			cprintf_data("      上层：总计",Recv_amount->Upper,Recv_time->Upper,0,13,0,color);
			cprintf_data(            "数据",Recv_amount->UData,Recv_time->UData,0,13,0,color);
			cprintf_data(            "控制",Recv_amount->UCtrl,Recv_time->UCtrl,1,13,39,color);
			cprintf_data("      下层：总计",Recv_amount->Lower,Recv_time->Lower,0,13,0,color);
			cprintf_data(            "数据",Recv_amount->LData,Recv_time->LData,0,13,0,color);
			cprintf_data(            "控制",Recv_amount->LCtrl,Recv_time->LCtrl,1,13,39,color);
			cprintf_data("发送：整体：总计",Send_amount->Total,Send_time->Total,0,13,0,color);
			cprintf_data(            "失败",Send_amount->Error,Send_time->Error,1,13,61,color);
			cprintf_data("      上层：总计",Send_amount->Upper,Send_time->Upper,0,13,0,color);
			cprintf_data(            "数据",Send_amount->UData,Send_time->UData,0,13,0,color);
			cprintf_data(            "控制",Send_amount->UCtrl,Send_time->UCtrl,1,13,39,color);
			cprintf_data("      下层：总计",Send_amount->Lower,Send_time->Lower,0,13,0,color);
			cprintf_data(            "数据",Send_amount->LData,Send_time->LData,0,13,0,color);
			cprintf_data(            "控制",Send_amount->LCtrl,Send_time->LCtrl,1,13,39,color);
			break;
		}
		case 3:{//应用层
			cprintf("自动发送超时：",color);cprintf(Upper_Send_timeout,14);print_void(6-num_len(Upper_Send_timeout));//特别写入此位置
			cprintf("向下发送超时：",color);cprintf(Lower_Send_timeout,14);print_void(6-num_len(Lower_Send_timeout));
			cprintf("接收超时：",color);cprintf(Recv_timeout,14);print_void(6-num_len(Recv_timeout));
			print_void(63);printf("\n");
			cprintf("自动发送指数：",color);cprintf(Upper_full,14);print_void(6-num_len(Upper_full));//特别写入此位置
			cprintf("下层拥堵指数：",color);cprintf(Lower_full,14);print_void(6-num_len(Lower_full));
			cprintf("接收缓存：",color);cprintf(Recv_buf,14);print_void(6-num_len(Recv_buf));
			cprintf("发送缓存：",color);cprintf(Send_buf,14);print_void(6-num_len(Send_buf));
			print_void(47);printf("\n");
			cprintf_data("接收：整体：总计",Recv_amount->Total,Recv_time->Total,0,13,0,color);
			cprintf_data(            "失败",Recv_amount->Error,Recv_time->Error,0,13,0,color);
			cprintf_data(            "未知",Recv_amount->Unknown,Recv_time->Unknown,1,13,39,color);
			cprintf_data("      下层：总计",Recv_amount->Lower,Recv_time->Lower,0,13,0,color);
			cprintf_data(            "数据",Recv_amount->LData,Recv_time->LData,0,13,0,color);
			cprintf_data(            "控制",Recv_amount->LCtrl,Recv_time->LCtrl,1,13,39,color);
			cprintf_data("发送：整体：总计",Send_amount->Total,Send_time->Total,0,13,0,color);
			cprintf_data(            "失败",Send_amount->Error,Send_time->Error,1,13,61,color);
			cprintf_data("      下层：总计",Send_amount->Lower,Send_time->Lower,0,13,0,color);
			cprintf_data(            "数据",Send_amount->LData,Send_time->LData,0,13,0,color);
			cprintf_data(            "自动",Send_amount->Auto ,Send_time->Auto ,0,13,0,color);
			cprintf_data(            "控制",Send_amount->LCtrl,Send_time->LCtrl,1,13,17,color);
			break;
		}
	}
	goto_xy(bInfo.dwCursorPosition.X,bInfo.dwCursorPosition.Y);//光标返回 
}
HEAD::HEAD(){memset(this,0,sizeof(HEAD));}//头部构造函数
HEAD::HEAD(char type,char *sequence,char *source,char *destination){//头部构造函数
	*(this->type)=type;
	for(int i=1;i<=3;++i) *(this->type+i)=*(sequence+i-1);
	for(int i=0;i<=3;++i) *(this->source+i)=*(source+i);
	for(int i=0;i<=3;++i) *(this->destination+i)=*(destination+i);
}
void HEAD::change_type(bool ack,bool nak,int layer){//修改type
	memset(&(this->type),0,1);
	if(ack) *(this->type)|=0b10000000;
	if(nak) *(this->type)|=0b01000000;
	switch(layer){
		case 0:{*(this->type)|=0b00000000;break;}//物理层
		case 1:{*(this->type)|=0b00000001;break;}//链路层
		case 2:{*(this->type)|=0b00000010;break;}//网络层
		case 3:{*(this->type)|=0b00000011;break;}//应用层
	}
}
void HEAD::change_sequence(int sequence){//修改sequence
	char *tmp=(char*)&sequence;
	for(int i=1;i<=3;++i)
		*(this->type+i)=*(tmp+i-1);
}
void HEAD::change_source(int ip,int port){//修改source
	char *tmp;
	for(int i=0;i<=3;++i){
		tmp=(char*)&ip+i;
		source[i]=*tmp;
	}
	for(int i=4;i<=5;++i){
		tmp=(char*)&port+i-4;
		source[i]=*tmp;
	}
}
void HEAD::change_destination(int ip,int port){//修改destination
	char *tmp;
	for(int i=0;i<=3;++i){
		tmp=(char*)&ip+i;
		destination[i]=*tmp;
	}
	for(int i=4;i<=5;++i){
		tmp=(char*)&port+i-4;
		destination[i]=*tmp;
	}
}
bool HEAD::get_ack(){//获取ACK标识
	return *(this->type)&0b10000000;
}
bool HEAD::get_nak(){//获取NAK标识
	return *(this->type)&0b01000000;
}
int HEAD::get_layer(){//获取层次类型
	return *(this->type)&0b00000011;
}
int HEAD::get_sequence(){//获取帧序号
	int sequence=0;
	char *tmp=(char*)&sequence;
	for(int i=1;i<=3;++i)
		*(tmp+i-1)=*(this->type+i);
	return sequence;
}
int HEAD::get_source_ip(){//获取发送方IP
	//返回值为一个int型整数，其占用4个字节，每个字节代表IP的一个数字
	int *ip=(int*)&source;
	return *ip;
}
int HEAD::get_source_port(){//获取发送方端口
	int port=0;
	char *tmp=(char*)&port;
	for(int i=4;i<=5;++i)
		*(tmp+i-4)=*(this->source+i);
	return port;
}
int HEAD::get_destination_ip(){//获取接收方IP
	//返回值为int，其占用4个字节，每个字节代表IP的一个数字
	int *ip=(int*)&destination;
	return *ip;
}
int HEAD::get_destination_port(){//获取发送方端口
	int port=0;
	char *tmp=(char*)&port;
	for(int i=4;i<=5;++i)
		*(tmp+i-4)=*(this->destination+i);
	return port;
}
void change_head(HEAD *h,int mode,int layer,int seq,string sour,string dest){
	//mode代表模式，0为发送，1为返回ACK，2为返回NAK
	switch(mode){
		case 0:{h->change_type(0,0,layer);break;}
		case 1:{h->change_type(1,0,layer);break;}
		case 2:{h->change_type(0,1,layer);break;}
	}
	h->change_sequence(seq);
	
	int s_ip=0,s_port=0;
	int d_ip=0,d_port=0;
	
	String2Num(&s_ip,&s_port,sour);
	h->change_source(s_ip,s_port);
	
	String2Num(&d_ip,&d_port,dest);
	h->change_destination(d_ip,d_port);
}
void change_head(HEAD *h,int mode,int layer,int seq,int s_ip,int s_port,int d_ip,int d_port){//修改头部
	//mode代表模式，0为发送，1为返回ACK，2为返回NAK
	switch(mode){
		case 0:{h->change_type(0,0,layer);break;}
		case 1:{h->change_type(1,0,layer);break;}
		case 2:{h->change_type(0,1,layer);break;}
	}
	h->change_sequence(seq);
	h->change_source(s_ip,s_port);
	h->change_destination(d_ip,d_port);
}
void change_frame(char *object,char *source,int len){//转移帧内容
	//*object为目标输出内容，*source为源内容
	for(int i=0;i<=len-1;++i) *(object+i)=*(source+i);
}
INFO::INFO(){memset(this,0,sizeof(INFO));}//收发帧信息构造函数
INFO::INFO(int sequence,int len,int buf_len,int Lower_number,int Mac_number,int lable){//收发帧信息构造函数
	this->sequence=sequence;
	this->len=len;
	this->buf_len=buf_len;
	this->Lower_number=Lower_number;
	this->Mac_number=Mac_number;
	this->lable=lable;
	resend=0;
}

FRAME::FRAME(){sequence=0;data=NULL;}//帧数据信息构造函数 
FRAME::FRAME(int sequence,char *data){//帧数据信息构造函数 
	this->sequence=sequence;
	this->data=data;
}

int num_len(int num){//求数字长度 
	int k=0;
	if(num==0) k=1;
	while(num!=0){
		num/=10;
		k++;
	}
	return k; 
}
void goto_xy(int x,int y){//定位光标位置到指定坐标
    HANDLE hOut;
    hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD pos = { x,y };
    SetConsoleCursorPosition(hOut, pos);
}
void print_void(int num){//打印空格 
	if(num<=0) return;
	for(int i=1;i<=num;++i) printf(" ");
}
void cprintf(string str,WORD color){//彩色字体 
	WORD colorOld;
	HANDLE handle = ::GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(handle, &csbi);
	colorOld = csbi.wAttributes;
	SetConsoleTextAttribute(handle, color);
	printf("%s",str.c_str());
	SetConsoleTextAttribute(handle, colorOld);
}
void cprintf(int num,WORD color){//彩色字体 
	WORD colorOld;
	HANDLE handle = ::GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(handle, &csbi);
	colorOld = csbi.wAttributes;
	SetConsoleTextAttribute(handle, color);
	printf("%d",num);
	SetConsoleTextAttribute(handle, colorOld);
}
void cprintf_16(int num,WORD color){//彩色字体 
	WORD colorOld;
	HANDLE handle = ::GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(handle, &csbi);
	colorOld = csbi.wAttributes;
	SetConsoleTextAttribute(handle, color);
	printf("%02x",num);
	SetConsoleTextAttribute(handle, colorOld);
}
void cprintf_data(string str,int amount,int time,bool last,int width,int voidth,WORD color){//彩色字体 
	//str：数据的名称。 amount：bit数。 time：次数。 
	//last： 是否为最后一个数据(是的话会打印一个换行)，1为是，0为否 
	//width：数字部分所占的总长度(含空格) 
	//voidth：在末尾打印的空格数量 
	WORD colorOld;
	HANDLE handle = ::GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(handle, &csbi);
	colorOld = csbi.wAttributes;
	SetConsoleTextAttribute(handle, color);
	printf("%s",str.c_str());
	SetConsoleTextAttribute(handle, 14);
	printf("%d",amount);
	SetConsoleTextAttribute(handle, color);
	printf("位(");
	SetConsoleTextAttribute(handle, 14);
	printf("%d",time);
	SetConsoleTextAttribute(handle, color);
	printf("次)");
	print_void(voidth);
	if(last==1){
		for(int i=1;i<=width-1-num_len(amount)-num_len(time);++i) printf(" ");
		printf("\n");
	}
	else for(int i=1;i<=width-num_len(amount)-num_len(time);++i) printf(" ");
	SetConsoleTextAttribute(handle, colorOld);
}
bool compare_addr(SOCKADDR_IN a,SOCKADDR_IN b){//比较地址
	if(memcmp(&(a.sin_addr.s_addr),&(b.sin_addr.s_addr),sizeof(a.sin_addr.s_addr))==0) return 1;
	else return 0;
}
bool compare_port(SOCKADDR_IN a,SOCKADDR_IN b){//比较端口
	return a.sin_port==b.sin_port;
}
bool compare_sock(SOCKADDR_IN a,SOCKADDR_IN b){//比较套接字
	return compare_addr(a,b)&&compare_port(a,b);
}
void code(unsigned long x,char A[],int length){//将整数x中length长度的位数编码至A中
	unsigned long test=1<<(length-1);//高位在前
	for(int i=0;i<=length-1;++i){//本算法利用了移位操作和"与"计算，逐位测出x的每一位是0还是1
		if(test&x) A[i]=1;
		else A[i]=0;
		test>>=1; 
	}
}
unsigned long decode(char A[],int length){//将A解码为长度length的整数x
	unsigned long x=0;
	for(int i=0;i<length;++i){
		x<<=1;
		if(A[i]) x|=1;
	}
	return x;
}
void Many_String2Num(string s,int *n,int num){//将多个string型的数字转为int
	int length=s.length();
	
	char num_tmp[num][10],s2[30];
	memset(num_tmp,0,sizeof(num_tmp));
	memset(s2,0,sizeof(s2));
	
	int num_cnt=0,cnt=0;
	bool minus=0;
	
	strcpy(s2,s.c_str());//string转char
	for(int i=1;i<=length-1;++i){
		if(s2[i]==','||s2[i]==':'){
			*(n+num_cnt)=atoi(num_tmp[num_cnt]);
			if(minus) *(n+num_cnt)=-*(n+num_cnt);
			num_cnt++;
			minus=0;
			cnt=0;
		}
		else{
			if(s2[i]=='-') minus=!minus;
			else num_tmp[num_cnt][cnt++]=s2[i];
		}
	}
	*(n+num_cnt)=atoi(num_tmp[num_cnt]);
	if(minus) *(n+num_cnt)=-*(n+num_cnt);
}
void String2Num(int *ip,int *port,string addr){//将string型的地址转为数字型
	int length=addr.length();
	
	char s[30],*tmp=NULL;
	int ip_num[4];
	
	char num_tmp[5][10];
	int num_cnt=0,cnt=0;
	
	memset(s,0,sizeof(s));
	memset(ip_num,0,sizeof(ip_num));
	memset(num_tmp,0,sizeof(num_tmp));
	
	strcpy(s,addr.c_str());//string转char
	for(int i=0;i<=length-1;++i){
		if(s[i]=='.'||s[i]==':'){
			ip_num[num_cnt]=atoi(num_tmp[num_cnt]);
			num_cnt++;
			cnt=0;
		}
		else num_tmp[num_cnt][cnt++]=s[i];
	}
	for(int i=0;i<=3;++i){
		tmp=(char*)ip+i;
		*tmp=(char)ip_num[i];
	}
	*port=atoi(num_tmp[num_cnt]);
	return;
}
int Byte2Bit(char* bit,int bitlength,char* byte,int bytelength){//将字节数组转为bit数组，*bit为输出，返回值为其大小
	int length=min(bytelength,bitlength/8);
	for(int i=0;i<=length-1;++i)//每次编码8位
		code(byte[i],&(bit[i*8]),8);
	return length*8;
}
int Bit2Byte(char* bit,int bitlength,char* byte,int bytelength){//将bit数组转为字节数组
	//*byte为输出，返回值为其大小
	
	int length=min(bytelength*8,bitlength);
	int retlength;
	if(bitlength>bytelength*8)//截断转换
		retlength=bytelength;
	else if(bitlength%8!=0)   //bit数略小
		retlength=bitlength/8+1;
	else                      //恰好相等
		retlength=bitlength/8;
	
	int cnt=0;
	memset(byte,0,retlength);
	for(int i=0;i<=length-1;++i){
		if(i%8==0&&i!=0) cnt++;
		if(*(bit+i)){
			int mod=i%8;
			switch(mod){
				case 0:{byte[cnt]|=0b10000000;break;}
				case 1:{byte[cnt]|=0b01000000;break;}
				case 2:{byte[cnt]|=0b00100000;break;}
				case 3:{byte[cnt]|=0b00010000;break;}
				case 4:{byte[cnt]|=0b00001000;break;}
				case 5:{byte[cnt]|=0b00000100;break;}
				case 6:{byte[cnt]|=0b00000010;break;}
				case 7:{byte[cnt]|=0b00000001;break;}
			}
		}
	}
	return retlength;
}
void print_bit(char* A,int length,string word,int mode,int color){//按bit形式打印数据缓冲区内容
	//mode:原始数据格式，0为bit数组，1为字节数组
	int LineCount=0;
	
	if(color>8) cprintf(word,16*color-114);
	else if(color<8) cprintf(word,16*color+128);
	else cprintf(word,142);
	if(mode==0){
		printf("(长度 %d bit)",length);
		print_void(108-word.length()-num_len(length));
	}
	else{
		printf("(长度 %d bit)",length*8);
		print_void(108-word.length()-num_len(length*8));
	}
	printf("\n");
	
	if(mode==0){
		for(int i=0;i<=length-1;++i){
			if(LineCount>=64){
				LineCount=0;
				printf("\n");
			}
			if(LineCount%8==0){
				printf("   ");
				if(LineCount){
					cprintf(" ",255);
					printf("   ");
				}
			}
			cprintf(A[i],color);
			LineCount++;
		}
	}
	else{
		char B[8];
		for(int i=0;i<=length-1;++i){
			code(A[i],B,8);
			if(LineCount>=64){
				LineCount=0;
				printf("\n");
			}
			if(LineCount%8==0){
				printf("   ");
				if(LineCount){
					cprintf(" ",255);
					printf("   ");
				}
			}
			for(int j=0;j<=7;++j){
				cprintf(B[j],color);
				LineCount++;
			}
		}
	}
	print_void(116-15*(LineCount/8)-LineCount%8);
	printf("\n");
}
void print_byte(char* A,int length,string word,int mode,int color){//按16进制字节形式打印数据缓冲区内容
	//mode:原始数据格式，0为bit数组，1为字节数组
	int LineCount=0;
	if(mode==0) length=Bit2Byte(A,length,A,length);
	
	if(color>8) cprintf(word,16*color-114);
	else if(color<8) cprintf(word,16*color+128);
	else cprintf(word,142);
	if(mode==0){
		printf("(长度 %d )",length/16);
		print_void(111-word.length()-num_len(length/16));
	}
	else{
		printf("(长度 %d )",length/2);
		print_void(111-word.length()-num_len(length/2));
	}
	printf("\n");
	
	for(int i=0;i<=length-1;++i){
		if(LineCount>=64){
			LineCount=0;
			printf("\n");
		}
		cprintf_16(A[i],color);
		if(LineCount%4==0){
			printf(" ");
			if(LineCount){
				cprintf(" ",255);
				printf(" ");
			}
		}
		LineCount++;
	}
	printf("\n");
}
bool IsPowerOfTwo(int n){//判断一个数是否为2的幂，0否1是
    int x=n;
	if(n==0) return 0;
	return (x&(x-1))==0;
}
int hamming_length(int len){//计算海明纠错码长度
	if(len==0) return 0;
	int ans=1,last=2;
	while(len+ans+1>last){
		ans++;
		last*=2;
	}
	return ans;
}
bool hamming_check(char *buf,int len,bool mode){//在确认有错误的情况下，使用海明码纠错
	//mode为*buf数据格式，0为bit流，1为字节
	//*buf为输出，返回0代表纠错失败，1代表成功 
	
	if(mode==0){//输入为bit
		int ham_len=hamming_length(len);//纠错码长度
		char old_ham[ham_len];//旧纠错码
		char new_ham[ham_len];//新纠错码
		memset(new_ham,0,ham_len);//新纠错码初始化
		register int cnt=0;//计算纠错码时，最多需要查看的位数
		
		for(int i=0;i<=len-1;++i){
			if(IsPowerOfTwo(i+1)){//经过一个校验位
				old_ham[cnt]=*(buf+i);//记录旧纠错码
				cnt++;//计数器+1
			}
			else{//经过数据位
				if((bool)buf[i]==1){//如果其表示的bit为1，则修改新纠错码的值
					int num_data=i+1;//数据位位数
					int num_che=1;   //校验位位数
					for(int j=1;j<=cnt;++j){
						if(num_data&1) new_ham[j-1]=((bool)new_ham[j-1])^1;
						num_data>>=1;
						num_che<<=1;
					}
				}
			}
		}
		
		int sum=0;//新旧纠错码异或之后的结果
		int num=1;//临时存储2的n次幂
		for(int i=0;i<=ham_len-1;++i){
			if(old_ham[i]!=new_ham[i]) sum+=num;
			num<<=1;
		}
		
		if(F_Print_mode) print_bit(buf,len,"海明编码纠正前的头部与数据：",0,8);
		if(sum==0||sum>len){//找不到错误，返回0(纠错失败)
			if(F_Print_mode) print_bit(buf,len,"海明编码纠正后的头部与数据：(无法纠正)",0,12); 
			return 0;
		}
		else{//找到错误，进行纠正
			*(buf+sum-1)=!((bool)*(buf+sum-1));
			if(F_Print_mode){
				cprintf("纠正了第",240);cprintf(sum,240);cprintf("位\n",240);
				print_bit(buf,len,"海明编码纠正后的头部与数据：(纠正完成)",0,7);
			}
			return 1;
		}
	}
	else{//输入为字节
		
	}
}
int hamming(char *buf,char *h,int len,bool mode){//将数据拼接并进行海明编码
	//其中*h为头部数据指针
	//mode为*buf *h数据格式，0为bit流，1为字节
	//*buf为输出，返回值为其长度
	
	if(mode==0){//输入为bit
		//拼接头部、数据
		char new_buf[len+8*HEAD_SIZE];
		len+=8*HEAD_SIZE;
		for(int i=0;i<=8*HEAD_SIZE-1;++i) new_buf[i]=*(h+i);
		for(int i=8*HEAD_SIZE;i<=len-1;++i) new_buf[i]=*(buf+i-8*HEAD_SIZE);
		if(F_Print_mode) print_bit(new_buf,len,"拼接后的头部与数据：",0,11);
		
		//进行编码
		len+=hamming_length(len);
		register int cnt=0;//计算纠错码时，最多需要查看的位数
		
		for(int i=0;i<=len-1;++i){//生成新的数据
			if(IsPowerOfTwo(i+1)){//经过一个校验位
				*(buf+i)=(bool)0;//初始化纠错码位
				cnt++;//计数器+1
			}
			else{//经过数据位
				int pos=i-cnt;//对应的new_buf位置
				*(buf+i)=new_buf[pos];
				if((bool)new_buf[pos]==1){//如果其表示的bit为1，则修改对应校验位的值
					int num_data=i+1;//数据位位数
					int num_che=1;   //校验位位数
					for(int j=1;j<=cnt;++j){
						if(num_data&1) *(buf+num_che-1)=((bool)*(buf+num_che-1))^1;
						num_data>>=1;
						num_che<<=1;
					}
				}
			}
		}
		return len;
	}
	else{//输入为字节 
		
	}
}
int dehamming(char *buf,int len,bool mode){//将数据解码
	//mode为*buf数据格式，0为bit流，1为字节
	//*buf为输出，返回值为其长度
	
	if(mode==0){//输入为bit
		register int last_pos=0;//上一个需要被替换的位置 
		for(int i=0;i<=len-1;++i){
			if(!IsPowerOfTwo(i+1)){//经过数据位
				*(buf+last_pos)=*(buf+i);
				last_pos++;
			}
		}
		return len-hamming_length(len);
	}
	else{//输入为字节
		
	}
}
unsigned int generate_check_sum(char *buf,int len){//生成校验和(未分段的最初始值)
	//默认*buf输入为字节类型
	
	int size=len/2;     //以2字节为单位，分出来的整组数
	int left=len%2;     //以2字节为单位，分组后剩下的字节数
	unsigned int sum=0; //各组数相加后的值
	
	for(int i=0;i<=size-1;++i) sum+=*((unsigned short*)buf+i); //整组的直接求和
	if(left){ //单个字节的，扩充后再求和
		unsigned short num=*((unsigned short*)buf+size); //扩充(实际上就是正常取数)
		num=num&(0xffff0000);                            //将后8位置零
		sum+=num;
	}
	return sum;
}
int add_check_sum(char *buf,int len,bool mode){//在数据最后添加一个 16bit / 2字节 的校验和
	//mode为*buf数据格式，0为bit流，1为字节
	//*buf为输出，返回值为其长度
	
	if(mode==0){//输入为bit
		int byte_len=len/8+(len%8!=0);
		char byte[byte_len];
		Bit2Byte(buf,len,byte,byte_len);//先将*buf转为字节
		
		unsigned int   sum=generate_check_sum(byte,byte_len);//完整值
		unsigned short sum_high=sum>>16;                     //高16位(进位)
		unsigned short sum_low=sum&(0x0000ffff);             //低16位
		unsigned int   secend_sum=sum_high+sum_low;          //高低位相加仍可能有进位，因此再取一遍高低位
		unsigned short secend_sum_high=secend_sum>>16;              //第二次的高16位(进位)
		unsigned short secend_sum_low=secend_sum&(0x0000ffff);      //第二次的低16位
		unsigned short check_sum=~(secend_sum_high+secend_sum_low);//校验和
		
		char bit[16];
		Byte2Bit(bit,16,(char*)&check_sum,2);//将校验和转为bit
		if(F_Print_mode) print_bit(bit,16,"计算出的校验和：",0,13);
		
		for(int i=len;i<=len+15;++i) *(buf+i)=bit[i-len];//将校验和附加至末尾
		return len+16;
	}
	else{//输入为字节
		unsigned int   sum=generate_check_sum(buf,len);//完整值
		unsigned short sum_high=sum>>16;               //高16位(进位)
		unsigned short sum_low=sum&(0x0000ffff);       //低16位
		unsigned int   secend_sum=sum_high+sum_low;    //高低位相加仍可能有进位，因此再取一遍高低位
		unsigned short secend_sum_high=secend_sum>>16;        //第二次的高16位(进位)
		unsigned short secend_sum_low=secend_sum&(0x0000ffff);//第二次的低16位
		unsigned short check_sum=~(secend_sum_high+secend_sum_low);//校验和
		
		*(buf+len)=*((char*)&check_sum);//将校验和附加至末尾
		*(buf+len+1)=*((char*)&check_sum+1);//将校验和附加至末尾
		return len+2;
	}
}
bool check_check_sum(char *buf,int len,char *check_sum,bool mode){//检查数据的校验和
	//mode为*buf数据格式，0为bit流，1为字节
	//*buf为输出，返回值为是否正确，0为错，1为对
	
	if(mode==0){//输入为bit
		int byte_len=len/8+(len%8!=0);
		char byte[byte_len];
		Bit2Byte(buf,len,byte,byte_len);//先将*buf转为字节
		
		unsigned int   sum=generate_check_sum(byte,byte_len);//不含旧校验和的完整值
		
		Bit2Byte(check_sum,16,byte,2);//将*check_sum转为字节
		sum+=*((unsigned short*)&byte);//加上旧校验和
		
		unsigned short sum_high=sum>>16;                     //高16位(进位)
		unsigned short sum_low=sum&(0x0000ffff);             //低16位
		unsigned int   secend_sum=sum_high+sum_low;          //高低位相加仍可能有进位，因此再取一遍高低位
		unsigned short secend_sum_high=secend_sum>>16;              //第二次的高16位(进位)
		unsigned short secend_sum_low=secend_sum&(0x0000ffff);      //第二次的低16位
		unsigned short new_check_sum=~(secend_sum_high+secend_sum_low);//新校验和
		
		if(F_Print_mode){
			char bit[16];
			Byte2Bit(bit,16,(char*)&new_check_sum,2);
			if(new_check_sum) print_bit(bit,16,"重计算出的校验和：(有误)",0,12);
			else print_bit(bit,16,"重计算出的校验和：(正确)",0,13);
		}
		
		return !(new_check_sum);//返回新校验和，为0则返回1，否则返回0
	}
	else{//输入为字节
		unsigned int   sum=generate_check_sum(buf,len);//不含旧校验和的完整值
		
		sum+=*((unsigned short*)&check_sum);//加上旧校验和
		
		unsigned short sum_high=sum>>16;               //高16位(进位)
		unsigned short sum_low=sum&(0x0000ffff);       //低16位
		unsigned int   secend_sum=sum_high+sum_low;    //高低位相加仍可能有进位，因此再取一遍高低位
		unsigned short secend_sum_high=secend_sum>>16;        //第二次的高16位(进位)
		unsigned short secend_sum_low=secend_sum&(0x0000ffff);//第二次的低16位
		unsigned short new_check_sum=~(secend_sum_high+secend_sum_low);//新校验和
		
		return !(new_check_sum);//返回新校验和，为0则返回1，否则返回0
	}
}
int frame_bit(char *buf,char *h,int len){//对bit成帧 
	//其中*h为头部数据(bit)指针
	//*buf为输出，返回值为其长度，返回-1时代表成帧后长度超过缓存上限
	
	if(len+8*(4+HEAD_SIZE)+hamming_length(8*HEAD_SIZE+len)\
	   >MAX_BUFFER_SIZE) return -1;//若添加头部、海明纠错码、校验和、帧头帧尾后超长，返回-1
	
	//链接头部，海明编码，添加校验和
	len=hamming(buf,h,len,0);     //进行海明编码，同时整合头部与数据
	if(F_Print_mode) print_bit(buf,len,"海明编码后的头部与数据：",0,6);
	
	len=add_check_sum(buf,len,0); //在buf后面添加校验和
	if(F_Print_mode) print_bit(buf,len,"添加校验和后的头部与数据：",0,13);
	
	char new_buf[(int)(len*1.2+17)]; //新的数据
	int new_len=0;                   //新的数据长度
	register int cnt=0;              //计数
	
	for(int i=0;i<=len-1;++i){//添加数据，修改连续的6个1
		new_buf[new_len++]=*(buf+i);
		if(*(buf+i)) cnt++;
		else cnt=0;
		if(cnt==5){
			new_buf[new_len++]=0;
			cnt=0;
		}
	}
	
	//链接帧头帧尾，成帧
	int final_len=new_len+16;//成帧后的最终长度
	if(final_len>MAX_BUFFER_SIZE) return -1;//超长，返回-1
	for(int i=8;i<=7+new_len;++i) *(buf+i)=new_buf[i-8];//成帧——头部+数据部分
	for(int i=0;i<=7;++i){//成帧——帧头帧尾部分
		if(i==0||i==7){
			*(buf+i)=0;
			*(buf+final_len-i-1)=0;
		}
		else{
			*(buf+i)=1;
			*(buf+final_len-i-1)=1;
		}
	}
	if(F_Print_mode) print_bit(buf,final_len,"成帧后的头部与数据：",0,10);
	return final_len;
}
int frame_byte(char *buf,char *h,int len){//对字节成帧
	//其中*h为头部数据(字节)指针
	//*buf为输出，返回值为其长度，返回-1时代表成帧后长度超过缓存上限
	
	return len;
}
int frame(char *buf,int len,HEAD *h,bool input_mode,bool output_mode){//成帧
	//其中*h为头部数据(字节)指针，input/output mode为*buf数据格式，0为bit流，1为字节
	//*buf为输出，返回值为其长度，返回-1时代表成帧后长度超过缓存上限
	
	if(output_mode==0){//输出为bit
		char bit[8*HEAD_SIZE];
		Byte2Bit(bit,HEAD_SIZE*8,(char*)h,HEAD_SIZE);//要先将头部转换为bit
		
		if(F_Print_mode){
			print_bit(bit,8*HEAD_SIZE,"原头部：",0,11);
			print_bit(buf,len,"原数据：",input_mode,11);
		}
		
		if(input_mode==0)//输入为bit，直接输出bit
			return frame_bit(buf,bit,len);
		else{//输入为字节，转换后输出bit
			if(len*8>MAX_BUFFER_SIZE) return -1;
			char byte[len];
			for(int i=0;i<=len-1;++i) byte[i]=*(buf+i);
			len=Byte2Bit(buf,len*8,byte,len);//将数据转换为bit
			return frame_bit(buf,bit,len);
		}
	}
	else{//输出为字节
		if(input_mode==1)//输入为字节，直接输出字节
			return frame_byte(buf,(char*)h,len);
		else{//输入为bit，转换后输出字节
			char bit[len];
			for(int i=0;i<=len-1;++i) bit[i]=*(buf+i);
			len=Bit2Byte(bit,len,buf,len/8+(len>0&&len%8!=0));
			return frame_byte(buf,(char*)h,len);
		}
	}
}
int deframe_bit(char *buf,char *h,int len){//对bit恢复
	//其中*h为头部数据(bit)指针
	//*buf,*h为输出，返回值为其长度，返回-2时代表帧误且无法恢复
	
	//去除帧头帧尾
	char new_buf[len-8];//新的数据
	int new_len=0;      //新的数据长度
	register int cnt=0; //计数
	bool in=0;          //是否进入了帧内
	int in_time=0;      //遇到 帧头/尾 的次数
	for(int i=0;i<=len-1;++i){
		if(*(buf+i)) cnt++;
		else cnt=0;
		if(cnt==6){
			if(*(buf+i+1)==0){
				if(++in_time>2) return -2;//遇到两次以上的定界符，直接返回-2
				i+=2;
				cnt=0;
				in=!in;
			}
			else return -2;//遇到连续的7个1，直接返回-2
		}		
		if(in&&i<=len-1){
			new_buf[new_len++]=*(buf+i);
			if(cnt==5&&*(buf+i+1)==0){
				i++;
				cnt=0;
				continue;
			}
		}
	}
	if(in_time<=1) return -2;//遇到不足两次的定界符，直接返回-2
	new_len-=6;//去除判断帧尾导致的多余字节
	if(new_len<8*HEAD_SIZE+16) return -2;//去除后长度小于头部和校验和长度和，显然错误，返回-2
	if(F_Print_mode) print_bit(new_buf,new_len,"拆帧后的头部、数据与校验和：",0,13);
	
	//恢复头部、数据、校验和
	char check_sum[16];//16bit校验和
	for(int i=new_len-16;i<=new_len-1;++i) check_sum[i-(new_len-16)]=new_buf[i];//恢复——校验和 
	new_len-=16;
	
	bool wrong=0;
	if(check_check_sum(new_buf,new_len,check_sum,0)==0){//检查校验和，若不对则进行纠错
		if(hamming_check(new_buf,new_len,0)==0) wrong=1;//使用海明码纠错，若失败则wrong=1
		if(check_check_sum(new_buf,new_len,check_sum,0)==0) wrong=1;//成功后再次检测校验和，不对则wrong=1
	}
	if(F_Print_mode&&wrong==0) print_bit(new_buf,new_len,"拆帧并纠错后的头部与数据：",0,6);
	
	new_len=dehamming(new_buf,new_len,0);//解码海明编码
	if(F_Print_mode&&wrong==0) print_bit(new_buf,new_len,"解码后的头部与数据：",0,11); 
	
	for(int i=0;i<=8*HEAD_SIZE-1;++i) *(h+i)=new_buf[i];//恢复——头部 
	if(wrong) return -2;//wrong=1时，返回-2 (延迟返回的目的：恢复头部以尝试发送NAK) 
	for(int i=8*HEAD_SIZE;i<=new_len-1;++i) *(buf+i-8*HEAD_SIZE)=new_buf[i];//恢复——数据 
	new_len-=8*HEAD_SIZE; 
	if(F_Print_mode){
		print_bit(h,8*HEAD_SIZE,"最终恢复后的头部：",0,11);
		print_bit(buf,new_len,"最终恢复后的数据：",0,11);
	}
	return new_len;
}
int deframe_byte(char *buf,char *h,int len){//对字节恢复
	//其中*h为头部数据(字节)指针
	//*buf,*h为输出，返回值为其长度，返回-1时代表成帧后长度超过缓存上限
	
	return len;
}
int deframe(char *buf,int len,HEAD *h,bool input_mode,bool output_mode){//恢复
	//其中*h为头部数据(字节)指针，input/output mode为*buf数据格式，0为bit流，1为字节
	//*buf,*h为输出，返回值为其长度，返回-1时代表分帧后长度超过缓存上限，-2时代表帧误且无法恢复
	
	if(input_mode==0){//输入为bit
		if(len>MAX_BUFFER_SIZE) return -1;
		char bit[8*HEAD_SIZE];
		Byte2Bit(bit,8*HEAD_SIZE,(char*)h,HEAD_SIZE);
		len=deframe_bit(buf,bit,len);//先分帧
		
		Bit2Byte(bit,HEAD_SIZE*8,(char*)h,HEAD_SIZE);//将头部转换为字节
		
		if(output_mode==0)//输出为bit，则直接输出
			return len;
		else{//输出为字节，转换后再输出
			if(len<0) return len;
			char bit[len];
			for(int i=0;i<=len-1;++i) bit[i]=*(buf+i);
			return Bit2Byte(bit,len,buf,len/8+(len>0&&len%8!=0));
		}
	}
	else{//输入为字节
		if(len*8>MAX_BUFFER_SIZE) return -1;
		len=deframe_byte(buf,(char*)h,len);//先分帧
		
		if(output_mode==1)//输出为字节，则直接输出
			return len;
		else{//输出为bit，转换后再输出
			if(len<0) return len;
			char byte[len];
			for(int i=0;i<=len-1;++i) byte[i]=*(buf+i);
			return Byte2Bit(buf,len*8,byte,len);
		}
	}
}
void set_control(char *control,int buf,int thershold){//创建控制信息
	//control长度为2字节，第1个字节为全1，第2个字节为数字n
	char byte[2];
	memset(byte,255,1);
	if(buf<0) byte[1]=0;              //特判负数，代表生成清零信息
	else if(buf<thershold) byte[1]=-1;//未达到阈值，则n=-1，代表信息无效
	else{
		if(buf>=thershold*0.9) byte[1]=100;
		else{
			double k=(double)(buf-thershold)/(MAX_BUFFER_SIZE-thershold);
			char n=ceil(100*k*k);
			byte[1]=n;
		}
	}
	Byte2Bit(control,16,byte,2);
}

void set_source_ip_info(int ip_subnet,int ip_port,int seq,char *buf){//设置向上层发送的数据的源信息(数据为字节型)(组成虚拟IP地址)
	char *tmp;
	tmp=(char*)&ip_subnet; 
	*buf=*tmp;
	tmp=(char*)&ip_port;
	for(int i=1;i<=2;++i) *(buf+i)=*(tmp+i-1);
	tmp=(char*)&seq;
	for(int i=3;i<=5;++i) *(buf+i)=*(tmp+i-3);
}
void set_destination_ip_info(int ip_subnet,int ip_port,char *buf){//设置向下层发送的数据的目的信息(数据为字节型)(组成虚拟IP地址)
	char *tmp;
	tmp=(char*)&ip_subnet; 
	*buf=*tmp;
	tmp=(char*)&ip_port;
	for(int i=1;i<=2;++i) *(buf+i)=*(tmp+i-1);
}
bool get_source_ip_info(int *ip_subnet,int *ip_port,int *seq,char *buf,int len){//获取从下层接收的数据的源信息(数据为字节型)(组成虚拟IP地址)
	//返回值：0代表失败(数据过短)，1代表成功 
	if(len<6) return 0;
	*ip_subnet=0;*ip_port=0;*seq=0;
	char *tmp;
	tmp=(char*)ip_subnet; 
	*tmp=*buf;
	tmp=(char*)ip_port;
	for(int i=1;i<=2;++i) *(tmp+i-1)=*(buf+i);
	tmp=(char*)seq;
	for(int i=3;i<=5;++i) *(tmp+i-3)=*(buf+i);
	return 1;
}
bool get_destination_ip_info(int *ip_subnet,int *ip_port,char *buf,int len){//获取从上层接收的数据的目的信息(数据为字节型)(组成虚拟IP地址)
	//返回值：0代表失败(数据过短)，1代表成功 
	if(len<6) return 0;
	*ip_subnet=0;*ip_port=0;
	char *tmp;
	tmp=(char*)ip_subnet; 
	*tmp=*buf;
	tmp=(char*)ip_port;
	for(int i=1;i<=2;++i) *(tmp+i-1)=*(buf+i);
	return 1;
}

void set_source_mac_info(int mac_ip,int mac_port,int seq,char *buf){//设置向上层发送的数据的源信息(数据为字节型)(组成虚拟MAC地址)
	char *tmp;
	tmp=(char*)&mac_ip; 
	for(int i=0;i<=3;++i) *(buf+i)=*(tmp+i);
	tmp=(char*)&mac_port;
	for(int i=4;i<=5;++i) *(buf+i)=*(tmp+i-4);
	tmp=(char*)&seq;
	for(int i=6;i<=8;++i) *(buf+i)=*(tmp+i-6);
}
void set_destination_mac_info(int mac_ip,int mac_port,char *buf){//设置向下层发送的数据的目的信息(数据为字节型)(组成虚拟MAC地址)
	char *tmp;
	tmp=(char*)&mac_ip; 
	for(int i=0;i<=3;++i) *(buf+i)=*(tmp+i);
	tmp=(char*)&mac_port;
	for(int i=4;i<=5;++i) *(buf+i)=*(tmp+i-4);
}
bool get_source_mac_info(int *mac_ip,int *mac_port,int *seq,char *buf,int len){//获取从下层接收的数据的源信息(数据为字节型)(组成虚拟MAC地址)
	//返回值：0代表失败(数据过短)，1代表成功 
	if(len<9) return 0;
	*mac_ip=0;*mac_port=0;*seq=0;
	char *tmp;
	tmp=(char*)mac_ip; 
	for(int i=0;i<=3;++i) *(tmp+i)=*(buf+i);
	tmp=(char*)mac_port;
	for(int i=4;i<=5;++i) *(tmp+i-4)=*(buf+i);
	tmp=(char*)seq;
	for(int i=6;i<=8;++i) *(tmp+i-6)=*(buf+i);
	return 1;
}
bool get_destination_mac_info(int *mac_ip,int *mac_port,char *buf,int len){//获取从上层接收的数据的目的信息(数据为字节型)(组成虚拟MAC地址)
	//返回值：0代表失败(数据过短)，1代表成功 
	if(len<6) return 0;
	*mac_ip=0;*mac_port=0;
	char *tmp;
	tmp=(char*)mac_ip; 
	for(int i=0;i<=3;++i) *(tmp+i)=*(buf+i);
	tmp=(char*)mac_port;
	for(int i=4;i<=5;++i) *(tmp+i-4)=*(buf+i);
	return 1;
}

void print_ip(int ip,bool pvoid,int color){//打印ip
	int num[4],length=3;
	char *tmp;
	tmp=(char*)&ip;
	for(int i=0;i<=3;++i){
		num[i]=*(tmp+i);
		length+=num_len(num[i]);
		cprintf(num[i],color);
		if(i!=3) cprintf(".",color);
	}
	if(pvoid) print_void(15-length);
}
void print_port(int port,bool pvoid,int color){//打印端口
	int num,length=0;
	cprintf(port,color);
	if(pvoid) print_void(5-num_len(port));
}
