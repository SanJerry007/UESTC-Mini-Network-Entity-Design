//成帧测试
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

const int MAX_BUFFER_SIZE=65000; //缓存空间
const int HEAD_SIZE=sizeof(HEAD);//头部长度
int F_Print_mode=1;              //本层帧处理信息打印模式，0为不打印帧的处理信息，1为打印  

int num_len(int num){//求数字长度 
	int k=0;
	if(num==0) k=1;
	while(num!=0){
		num/=10;
		k++;
	}
	return k; 
}
void print_void(int num){//打印空格 
	for(int i=1;i<=num;++i) printf(" ");
}
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
		unsigned short secend_sum_high=sum>>16;              //第二次的高16位(进位)
		unsigned short secend_sum_low=sum&(0x0000ffff);      //第二次的低16位
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
		unsigned short secend_sum_high=sum>>16;        //第二次的高16位(进位)
		unsigned short secend_sum_low=sum&(0x0000ffff);//第二次的低16位
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
		unsigned short secend_sum_high=sum>>16;              //第二次的高16位(进位)
		unsigned short secend_sum_low=sum&(0x0000ffff);      //第二次的低16位
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
		unsigned short secend_sum_high=sum>>16;        //第二次的高16位(进位)
		unsigned short secend_sum_low=sum&(0x0000ffff);//第二次的低16位
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
	if(wrong) return -2;//wrong=1时，返回-2 (目的：恢复头部以尝试发送NAK) 
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

int main(){
	HEAD ha,ba; 
	char a[MAX_BUFFER_SIZE]={'1','1','0'};
	memset(a,1,sizeof(a));	
	int len=100;
	
	len=frame(a,len,&ha,0,0);//成帧
	if(len==-1) cprintf("空间不足，成帧失败\n",12);
	else cprintf("成帧成功！\n",10);
	
//	memset(a+10,!(bool)*(a+10),1);//修改 
//	memset(a+21,!(bool)*(a+21),1);//修改 
//	print_bit(a,len,"修改第11位后的帧：",0,9);
	
	len=deframe(a,len,&ba,0,1);//恢复 
	if(len==-2) cprintf("帧误且纠错失败，恢复失败\n",12);
	else cprintf("恢复成功！\n",11);
	
	system("pause");
	return 0;
}
