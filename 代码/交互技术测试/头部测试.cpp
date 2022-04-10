//头部测试
#include<iostream>
#include<cstdio>
#include<cstdlib>
#include<windows.h>
using namespace std;

void String2Num(int *ip,int *port,string addr);//将string型的地址转为数字型
struct HEAD{//头部
	char type[4];        //第1位为：层次信息 ACK NAK标识 | 后3位为：帧序号
	char source[6];      //前4位为ip | 后2位为端口
	char destination[6]; //前4位为ip | 后2位为端口
	HEAD();                                                       //头部构造函数
	HEAD(char type,char *sequence,char *source,char *destination);//头部构造函数
	void change_type(bool ack,bool nak,int layer);//修改type
	void change_sequence(int sequence);           //修改sequence
	void change_source(int ip,int port);          //修改source
	void change_destination(int ip,int port);     //修改destination
	bool get_ack();            //获取ACK标识
	bool get_nak();            //获取NAK标识
	int get_layer();           //获取层次类型
	int get_sequence();        //获取帧序号
	int get_source_ip();       //获取发送方IP
	int get_source_port();     //获取发送方端口
	int get_destination_ip();  //获取接收方IP
	int get_destination_port();//获取接收方端口
};
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

const int HEAD_SIZE=sizeof(HEAD);//头部长度

void cprintf(string str,WORD color){//彩色字体 
	WORD colorOld;
	HANDLE handle = ::GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(handle, &csbi);
	colorOld = csbi.wAttributes;
	SetConsoleTextAttribute(handle, color);
	cout << str;
	SetConsoleTextAttribute(handle, colorOld);
}
void cprintf(int num,WORD color){//彩色字体 
	WORD colorOld;
	HANDLE handle = ::GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(handle, &csbi);
	colorOld = csbi.wAttributes;
	SetConsoleTextAttribute(handle, color);
	cout << num;
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
int Byte2Bit(char* bit,int bitlength,char* byte,int bytelength){//将字节数组转为bit数组
	//*bit为输出，返回值为其大小
	
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
	if(mode==0) printf("(长度 %d bit)\n",length);
	else printf("(长度 %d bit)\n",length*8);
	
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
	printf("\n");
}
void print_byte(char* A,int length,string word,int mode,int color){//按16进制字节形式打印数据缓冲区内容
	//mode:原始数据格式，0为bit数组，1为字节数组
	int LineCount=0;
	if(mode==0) length=Bit2Byte(A,length,A,length);
	
	if(color>8) cprintf(word,16*color-114);
	else if(color<8) cprintf(word,16*color+128);
	else cprintf(word,142);
	if(mode==0) printf("(长度 %d )\n",length/16);
	else printf("(长度 %d )\n",length/2);
	
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

int main(){
	bool ack=1,nak=0;
	int layer=2,sequence=56,length=16777215;
	unsigned char s_ip[4]={1,255,255,2};
	int s_port=28319;
	unsigned char d_ip[4]={12,8,128,255};
	int d_port=1024;
	
	int s_ip_num=0,d_ip_num=0;
	for(int i=0;i<=3;++i) *((char*)&s_ip_num+i)=*(s_ip+i);
	for(int i=0;i<=3;++i) *((char*)&d_ip_num+i)=*(d_ip+i);
	
	HEAD h;
	print_bit((char*)&h,HEAD_SIZE,"原头部：",1,11);
	
	h.change_type(ack,nak,layer);
	print_bit((char*)&h,HEAD_SIZE,"修改type后的头部：",1,11);
	
	h.change_sequence(sequence);
	print_bit((char*)&h,HEAD_SIZE,"修改sequence后的头部：",1,11);
	
	h.change_source(s_ip_num,s_port);
	print_bit((char*)&h,HEAD_SIZE,"修改source后的头部：",1,11);
	
	h.change_destination(d_ip_num,d_port);
	print_bit((char*)&h,HEAD_SIZE,"修改destination后的头部：",1,11);
	
	HEAD h2;
	change_head(&h2,1,layer,sequence,"1.255.255.2:28319","12.8.128.255:1024");
	print_bit((char*)&h2,HEAD_SIZE,"一键修改后的头部：",1,10);
	
	printf("  原来的ack：%d\n提取后的ack：%d\n\n",ack,h2.get_ack());
	printf("  原来的nak：%d\n提取后的nak：%d\n\n",nak,h2.get_nak());
	printf("  原来的layer：%d\n提取后的layer：%d\n\n",layer,h2.get_layer());
	printf("  原来的sequence：%d\n提取后的sequence：%d\n\n",sequence,h2.get_sequence());
	printf("  原来的source_ip：%d\n提取后的source_ip：%d\n\n",s_ip_num,h2.get_source_ip());
	printf("  原来的source_port：%d\n提取后的source_port：%d\n\n",s_port,h2.get_source_port());
	printf("  原来的destination_ip：%d\n提取后的destination_ip：%d\n\n",d_ip_num,h2.get_destination_ip());
	printf("  原来的destination_port：%d\n提取后的destination_port：%d\n\n",d_port,h2.get_destination_port());
	
	system("pause");
	return 0;
}
