//basic function of all layers
#ifndef BASIC_FUNCTION
#define BASIC_FUNCTION

#include<cstdio>
#include<iostream>
#include<cstring>
#include<winsock2.h>
#include<windows.h>
//#pragma comment (lib,"wsock32.lib")
using namespace std;

struct DATA{//统计信息
	unsigned int Total,Error,Unknown;                    //总数，Total=所有成功，Error=所有失败，Unknown=所有来源不明
	unsigned int Upper,UData,UCtrl;                      //上层，Upper=所有，Data=数据帧，Ctrl=控制帧
	unsigned int Lower,LData,LCtrl,Re,Wrong,ACK,NAK,Auto;//下层，Lower=所有，Data=数据帧，Ctrl=控制帧，Re=重传，Wrong=误帧，Auto=自动
	DATA();
};
void print_data(int Recv_timeout,int Upper_Send_timeout,int Lower_Send_timeout,int R_Send_timeout,int Divide_len,\
                int Upper_full,int Lower_full,int Send_buf,int Recv_buf,int Send_window_size,int Send_window_busy_num,int Send_window_stop_num,\
                DATA *Send_amount,DATA *Send_time,DATA *Recv_amount,DATA *Recv_time,int layer,int color);//打印统计信息

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
extern HEAD NULL_HEAD;//空结构体，用于特殊用途
void change_head(HEAD *h,int mode,int layer,int seq,string sour,string dest);//修改头部
void change_head(HEAD *h,int mode,int layer,int seq,int s_ip,int s_port,int d_ip,int d_port);//修改头部
void change_frame(char *object,char *source,int len);//转移帧内容

struct INFO{//收发帧信息
	int sequence;     //帧的序号
	int len;          //帧的实际长度(bit/字节)
	int buf_len;      //帧在缓存中的长度(字节)
	int Lower_number; //下层实体号
	int Mac_number;   //下层mac序号
	int lable;        //为0代表此帧为数据帧，为1代表此帧为ACK，为2代表此帧为NAK，为3代表此帧为控制帧
	int resend;       //重传次数
	INFO();                                                                          //收发帧信息构造函数
	INFO(int sequence,int len,int buf_len,int Lower_number,int Mac_number,int lable);//收发帧信息构造函数
};
extern INFO NULL_INFO;//空结构体，用于特殊用途

struct FRAME{//收到的帧数据信息
	int sequence;
	char *data;
	FRAME();
	FRAME(int sequence,char *data);
	bool operator > (const FRAME &b) const{
        return sequence<b.sequence;
    }
};

extern const int MAX_BUFFER_SIZE;//缓存空间(bit)
extern const int HEAD_SIZE;      //头部长度(字节)
extern int F_Print_mode;         //帧处理信息打印模式

int num_len(int num);               //求数字长度 
void goto_xy(int x,int y);          //定位光标位置到指定坐标
void print_void(int num);           //打印空格 
void cprintf(string str,WORD color);//彩色字体
void cprintf(int num,WORD color);   //彩色字体
void cprintf_16(int num,WORD color);//彩色字体
void cprintf_data(string str,int amount,int time,bool last,int width,int voidth,WORD color);//彩色字体 

bool compare_addr(SOCKADDR_IN a,SOCKADDR_IN b);//比较地址
bool compare_port(SOCKADDR_IN a,SOCKADDR_IN b);//比较端口
bool compare_sock(SOCKADDR_IN a,SOCKADDR_IN b);//比较套接字

void code(unsigned long x,char A[],int length);//编码
unsigned long decode(char A[],int length);     //解码

void Many_String2Num(string s,int *n,int num);//将多个string型的数字转为int
void String2Num(int *ip,int *port,string addr);//将string型的地址转为数字型

int Byte2Bit(char* bit,int bitlength,char* byte,int bytelength);//将字节数组转为bit数组
int Bit2Byte(char* bit,int bitlength,char* byte,int bytelength);//将bit数组转为字节数组

void print_bit(char* A,int length,string word,int mode,int color); //按bit形式打印数据缓冲区内容
void print_byte(char* A,int length,string word,int mode,int color);//按16进制字节形式打印数据缓冲区内容

bool IsPowerOfTwo(int n);//判断一个数是否为2的幂
int hamming_length(int len);//计算海明纠错码长度
bool hamming_check(char *buf,int len,bool mode);//在确认有错误的情况下，使用海明码纠错
int hamming(char *buf,char *h,int len,bool mode);//将数据拼接并进行海明编码
int dehamming(char *buf,int len,bool mode);//将海明编码的数据解码

unsigned int generate_check_sum(char *buf,int len);//生成校验和(未分段的最初始值)
int add_check_sum(char *buf,int len,bool mode);//在数据最后添加一个 16bit / 2字节 的校验和
bool check_check_sum(char *buf,int len,char *check_sum,bool mode);//检查数据的校验和

int frame_bit(char *buf,char *h,int len);//对bit成帧 
int frame_byte(char *buf,char *h,int len);//对字节成帧
int frame(char *buf,int len,HEAD *h,bool input_mode,bool output_mode);//成帧
int deframe_bit(char *buf,char *h,int len);//对bit恢复
int deframe_byte(char *buf,char *h,int len);//对字节恢复
int deframe(char *buf,int len,HEAD *h,bool input_mode,bool output_mode);//恢复

void set_control(char *control,int buf,int thershold);//创建控制信息

void set_source_ip_info(int ip_subnet,int ip_port,int seq,char *buf);//设置向上层发送的数据的源信息 (组成虚拟IP地址)
void set_destination_ip_info(int ip_subnet,int ip_port,char *buf);//设置向下层发送的数据的目的信息 (组成虚拟IP地址)
bool get_source_ip_info(int *ip_subnet,int *ip_port,int *seq,char *buf,int len);//获取从下层接收的数据的源信息 (组成虚拟IP地址)
bool get_destination_ip_info(int *ip_subnet,int *ip_port,char *buf,int len);//获取从上层接收的数据的目的信息 (组成虚拟IP地址)

void set_source_mac_info(int mac_ip,int mac_port,int seq,char *buf);//设置向上层发送的数据的源信息 (组成虚拟MAC地址)
void set_destination_mac_info(int mac_ip,int mac_port,char *buf);//设置向下层发送的数据的目的信息 (组成虚拟MAC地址)
bool get_source_mac_info(int *mac_ip,int *mac_port,int *seq,char *buf,int len);//获取从下层接收的数据的源信息 (组成虚拟MAC地址)
bool get_destination_mac_info(int *mac_ip,int *mac_port,char *buf,int len);//获取从上层接收的数据的目的信息 (组成虚拟MAC地址)

void print_ip(int ip,bool pvoid,int color);//打印ip
void print_port(int port,bool pvoid,int color);//打印端口

#endif