#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define maxopcode	255
#define parnum		4
#define segregnum	6

#define tconstant	16
#define tsegreg		8
#define tMMXreg		7
#define tsimplereg	3
#define tmem2		2
#define tmem1		1
#define tmem0		0

typedef struct inforec {unsigned size:4,typ:5,opsize:1,addrsize:1,segregpref:3,info:3,index:3,basis:3,shift:2;long data;unsigned segdata;};

char	*rem,*par[parnum],acode[16],segregstr[segregnum]={'e','c','s','d','f','g'}
	,rcode[4]={0xe,0x6a,0,0xcb},str[32]="";
int	opnum2,alen,use32=0,addr32,cpu=0,needcpu;
struct	inforec b1[parnum];

void cputype(int x)
{
	if(x>needcpu)needcpu=x;
}

int	getrmbyte(char bit345,int p2)
{
 return( (b1[p2].typ<=tsimplereg || b1[p2].typ==tMMXreg) ? (b1[p2].typ<<6)+(b1[p2].info)+(bit345<<3):-1);
}

void	appendimdata(int p,int n)
{memcpy(&acode[alen],&b1[p].data,n);alen+=n;}

void	appendrmdata(int p)
{
	int n;
	if((n=b1[p].typ)<=tmem2)
	{
		if(addr32)
		{
			if((!n&&(b1[p].info==5||(b1[p].info==4&&b1[p].basis==5)))||n==2)n=4;
		}else	if(!n&&b1[p].info==6)n=2;
		appendimdata(p,n);
	}
}

int	getsegregpref(char seg)
{return( (seg<4) ? 0x26+(seg<<3) : 0x60+seg);}//rechts:cputype(3);

void	putsegregpref(int p)
{if(b1[p].segregpref<segregnum)acode[alen++]=getsegregpref(b1[p].segregpref);}

void	putfullrm(int bit345,int p)
{
	acode[alen++]=getrmbyte(bit345,p);
	if(addr32&&b1[p].info==4)
	{
		acode[alen++]=b1[p].basis+(b1[p].index<<3)+(b1[p].shift<<6);
	}
	appendrmdata(p);
}
void	putrmop(int bit345,int p,int op)
{putsegregpref(p);acode[alen++]=op;putfullrm(bit345,p);}
void	putrmxop(int bit345,int p,int op)
{putsegregpref(p);acode[alen++]=0xf;acode[alen++]=op;putfullrm(bit345,p);}
void  putaddrsize(int p)
{
	int one32=0,no16=1;
	if(b1[p].typ<=tmem2)one32=no16=b1[p].addrsize;
	if((addr32=use32?no16:one32)!=0)cputype(3);
	if(use32?!no16:one32)acode[alen++]=0x67;
}
void	putaddrsizepref(void)
{
	int i;
	int one32=0,no16=1;
	for(i=0;i<parnum&&par[i]&&par[i][0];i++)if(b1[i].typ<=tmem2){one32|=b1[i].addrsize;no16&=b1[i].addrsize;}
	addr32=(use32&&no16)||(!use32&&one32);
	if(addr32)cputype(3);
	if(use32?!no16:one32)acode[alen++]=0x67;//((use32&&!no16)||(!use32&&one32))
}
int		testopsize(int i)
{return(use32? b1[i].typ!=tsegreg&&b1[i].size&2&&!(b1[i].size&1) : !(b1[i].size&3)&&b1[i].size&4);}
//if((use32&&b1[i].size==2)||(!use32&&b1[i].size==4))
void	putopsizepref(void)
{
	int i;
	putaddrsizepref();
	for(i=0;i<parnum&&par[i][0];i++)if(testopsize(i)){acode[alen++]=0x66;break;}//cputype(3);
}
void	xchgpar(int p1,int p2)
{char *ct;struct inforec bt1;
bt1=b1[p1];b1[p1]=b1[p2];b1[p2]=bt1;ct=par[p1];par[p1]=par[p2];par[p2]=ct;}
void	shiftpar(void)
{
	memmove(par,&par[1],(parnum-1)*sizeof(par[0]));
	memmove(b1,&b1[1],(parnum-1)*sizeof(b1[0]));
	par[parnum-1]=NULL;
	memset(&b1[parnum-1],0,sizeof(b1[0]));
}
	isshort(long p1)
{return(p1>=-128&&p1<128);}

#ifdef __TURBOC__
int	dobsf(int i)
{
	int count=0;
	_AX=i;
asm or ax,ax
	L1:
asm jz L1E
	count++;
	_AX>>=1;
asm jnc L1
	L1E:
	return(count-1);
}
#else
#if defined(__SW_3)||defined(__SW_4)||defined(__SW_5)
#pragma aux dobsf="mov cx,-1""bsf cx,ax" parm[ax] value[cx];
#else
#pragma aux dobsf="xor cx,cx""or ax,ax""jz +5""inc cx""shr ax,1""jnc -5""dec cx" parm[ax] value[cx];
#endif
#endif

#define getlowestbit(i) (1<<dobsf(i))
//int	getlowestbit(int i){return(1<<dobsf(i));}

int	testreg(char *p)
{
	int register res1=0,i;
	if(*p=='e')
	{
		res1=testreg(p+1);
		if(res1)return(res1&~0x3000|0x4000);
	}
	else if(*(p+1)=='i')
	{
		if(*p=='d')res1|=0x2007;
		if(*p=='s')res1|=0x2006;
	}
	else if(*(p+1)=='p')
	{
		if(*p=='b')res1|=0x2005;
		if(*p=='s')res1|=0x2004;
	}
	else if(*p>='a'&&*p<='d')
	{
		i=0;
		switch(*p)
		{
			case 'b':i++;
			case 'd':i++;
			case 'c':i++;
		}
		switch(*(p+1))
		{
			case 'x':res1|=0x2000;break;
			case 'h':i|=4;
			case 'l':res1|=0x1000;
		}
		res1|=i;
	}
	return(res1);
}

int	isstartofnum(char *c)
{
	if(*c=='+'||*c=='-')c++;
	return(isdigit(*c));
}

char	*readnum(char *str,long *res)
{
	char	*p=str+1,*end,*end2,base=10;
	while(isdigit(*p))p++;
	if(isxdigit(*p)&&isalnum(*(p+1)))while(isxdigit(*p))p++;
	if(isalpha(*p))switch(*p)
	{
		case 'b':base=2;p++;break;
		case 'd':base=10;p++;break;
		case 'h':base=16;p++;break;
		case 'o':base=8;p++;
	}
	end=p;
	*res=strtol(str,&end2,base);
	return(end);
}

void	getparinfo(int paramnum)
{
	char	ptrstr[40],*p=par[paramnum],*r1,*r2,*mul,*tmp;
	int	i,reg1,reg2;
	struct	inforec res1;
	long	rt;
	if(!p||!*p)return;
	memset(&res1,0,sizeof(res1));
	strcpy(ptrstr,"byte ptr ");
	if(!strncmp(ptrstr,p,8)){res1.size|=1;p+=9;}
	memcpy(ptrstr,"word",4);
	if(!strncmp(ptrstr,p,8)){res1.size|=2;p+=9;if(use32)res1.opsize=1;}
	if(*p=='d'&&!strncmp(ptrstr,p+1,8)){res1.size|=4;p+=10;if(!use32)res1.opsize=1;}
	if(*p=='q'&&!strncmp(ptrstr,p+1,8)){res1.size|=8;p+=10;}
	memcpy(ptrstr,"near",4);
	if(!strncmp(ptrstr,p,9)){res1.size|=2;p+=9;}
	strcpy(ptrstr,"far ptr ");
	if(!strncmp(ptrstr,p,7)){res1.size|=4;p+=8;}
	strcpy(ptrstr,"short ptr ");
	if(!strncmp(ptrstr,p,9)){res1.size|=1;p+=10;}

	if(*(p+1)=='s')
	{
		for(i=0;i<segregnum;i++)if(*p==segregstr[i])break;
		if(*(p+2)==':'){if(i>=segregnum)i=7;else p+=3;}
		else if(!*(p+2)&&i<segregnum){res1.typ=tsegreg;res1.size=2;res1.info=i;res1.segregpref=7;goto endoftst;}
	}else i=7;
	res1.segregpref=i;
	if(isstartofnum(p)){
		p=readnum(p,&res1.data);res1.typ=tconstant;
		if(*p==':'){
			res1.segdata=res1.data;
			res1.size|=4;
			p=readnum(++p,&res1.data);
		}
	}
	if(*p=='[')
	{
		if(!res1.size)res1.size=0xf;
		res1.typ=0;
		p++;
		while(isstartofnum(p))
		{
			p=readnum(p,&rt);
			if(isstartofnum(p)||*p=='+'||*p==']')res1.data+=rt;
			else break;
		}
		if(*p==']'){
			if(use32){res1.addrsize=1;res1.info=5;goto endoftst;}
		        res1.info=6;goto endoftst;
		}
		if((r1=strrchr(p,'e'))>=p)
		{
			res1.addrsize=1;
			res1.shift=1;
			mul=strchr(p,'*');
			if(mul!=r1-1&&mul!=r1+3){r2=r1;r1=strchr(p,'e');}
			else r2=strchr(p,'e');
			if(mul==r1-1)rt=*(mul-1)-'0';
			else if(mul==r1+3)rt=*(mul+1)-'0';
			else rt=1;
			//if(getlowestbit(rt)!=rt||rt>8)error;
			reg1=testreg(r1);
			reg2=testreg(r2);
			//if(reg1&7==4&&rt>1)error:esp!=index-reg
			if(rt==1&&((reg2&7)==5||(reg1&7)==4)){i=reg1;reg1=reg2;reg2=i;tmp=r1;r1=r2;r2=tmp;}
			if((reg2&7)==5&&(rt==1||r2!=r1))res1.typ=tmem1;
			res1.info=4;
			if(r2==r1)
			{
				if(rt>1)reg2=5;
				else if((reg1&7)!=4) res1.info=reg2;
			}
			res1.basis=reg2;res1.index=reg1;res1.shift=dobsf(rt);
			if(res1.info==4&&res1.basis==5&&!res1.typ)goto endoftst;
			goto testsize;
		}
		if(*p=='+')p++;
		reg1=testreg(p);
	//  if((reg1&0xf000)<0x2000) invalidmempointer
		reg1&=7;
		while(*p!='+'&&*p!=']'&&!isstartofnum(p))p++;
		while(isstartofnum(p)){p=readnum(p,&rt);res1.data+=rt;}
		if(*p==']'){
			if(reg1==7)res1.info|=1;
			if((reg1&6)==6)res1.info|=4;
			if(reg1==5){res1.info|=6;res1.typ=tmem1;}
			if(reg1==3)res1.info|=7;
		}else
		{
			if(*p=='+')p++;
			reg2=testreg(p);
	 // s.o.
			reg2&=7;
			while(*p!='+'&&*p!=']'&&!isstartofnum(p))p++;
			if(reg1>reg2){i=reg1;reg1=reg2;reg2=i;}
			if(reg2==7)res1.info|=1;
			if(reg1==5)res1.info|=2;
	 // if(reg1==3)res1.info|=0;
			while(isstartofnum(p)){p=readnum(p,&rt);res1.data+=rt;}
		}
testsize:
		if(res1.data)
		{
			if(isshort(res1.data))res1.typ=tmem1;
			else res1.typ=tmem2;
		}
	}else if (*p=='m' && *(p+1)=='m' && (*(p+2)>='0' && *(p+2)<='9')) {
		res1.typ=tMMXreg;
		res1.info=*(p+2)-'0';
		if(!res1.size)res1.size=8+7;
	}else if(!res1.typ)
	{
		reg1=testreg(p);
	//if(!reg1)	unknown reg
	//if(res1.size&&res1.size!=(reg1>>12)) opsizeerror
		res1.size=reg1>>12;
		res1.typ=tsimplereg;
		res1.info=reg1;
		res1.opsize=(use32? res1.size==2 : res1.size==4);
	}
	if(res1.typ==tconstant&&!res1.size)
	{
		res1.size=0xf;
		i=res1.data>>8;
		if(i&&i!=-1)res1.size&=~1;
		i=i>>8;
		if(i&&i!=-1)res1.size&=~2;
	}
endoftst:
	b1[paramnum]=res1;
}

void checkparm(char *argp)
{
	char c=*argp,*p;
	if(c=='/'||c=='-')
	{
		c=*(++argp);
		if(isdigit(c))cpu=c-48;
		if(c=='u')use32=1;
		//if(c=='x')extended=1;
	}else
	{
		p=str;while(*p)p++;*(p++)=' ';
		memcpy(p,argp,strlen(argp)+1);
	}
}

/*
info:	4-Bit	possib.size
	4-Bit	type(const,seg-reg,general-reg,memdist,mem)
	3-Bit	seg-reg-prefix
	3-Bit	info for type (e.g.type of reg->ax->0)
	3-Bit basis & index bei 386-Adresse
	4 Byte	extra data for const/mem (2/4 Byte offset;1/2/4 Byte rel. offset)

info:
wReg|bReg|segReg|mem
0-ax	al	es			bx+si
1-cx	cl	cs			bx+di
2-dx	dl	ss			bp+si
3-bx	bl	ds			bp+di
4-sp	ah	fs			si
5-bp	ch	gs			di
6-si	dh					bp/[abs]
7-di	bh	nothing	bx

*/