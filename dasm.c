#include <conio.h>
#include "asmpr.c"

#ifdef __TURBOC__
void	readbuf(char *p)
{
	_AH=10;
	_DX=(int)p;
asm int 0x21
}
#else
void readbuf(char *);
char wherex(void);
char wherey(void);
void gotoxy(char,char);
#pragma aux readbuf="mov ah,10","int 0x21" parm[dx];
//inc(x),(y)
#pragma aux wherex="mov ah,3","mov bh,0","int 0x10" value[dl] modify[ax bh cx dh];
#pragma aux wherey="mov ah,3","mov bh,0","int 0x10" value[dh] modify[ax bh cx dl];
//dec(x),(y)
#pragma aux gotoxy="mov ah,2","mov bh,0","int 0x10" parm[dl][dh] modify exact[ax];
#endif

void readbuffer(char *p)
{if(*(p+1))*(p+*(p+1)+2)=13;readbuf(p);p+=*(p+1)+2;*p=0;}

char *readstring(char *line)
{
	static char buffer[128]={127,0};
	readbuffer(buffer);
	strcpy(line,buffer+2);
	gotoxy(40,wherey());
	return line;
}

void assembleline(char *line,FILE *f)
{
	char *p;//,line[128];
	int	j,err;
	//strcpy(line,str);
	strlwr(line);
	p=line;while(*p)
	{
		if(*p==9)*p=' ';
		if(*p==';')*p=0;else p++;
	}
	p=line;while(*p==' ')p++;if(p>line)strcpy(line,p);
	p=line;while(*p)
	{
		//if(*p==' '&&(*(p+1)==' '||*(p+1)==','))
		if(*(int*)p==0x2020||*(int*)p==0x2c20)strcpy(p,p+1);
		else p++;
	}
	p=line;
	memset(par,0,sizeof(par));
	memset(b1,0,sizeof(b1));
	while(*p!=' '&&*p)p++;
	j=0;
	while(*p)
	{
		*p=0;p++;
		while(*p&&(*p==' '))p++;
		if(*p==','){p++;while(*p&&(*p==' '))p++;}
		par[j++]=p;
		while(*p!=','&&*p)p++;
	}
	needcpu=0;
	for(j=parnum-1;j>=0;j--)getparinfo(j);
	alen=0;
	switch(err=execproc(line))
	{
		case 0:printf("%d Byte%c: ",alen,alen>1?'s':32);
			for(j=0;j<alen;j++)printf("%.2X",acode[j]);
			fwrite(acode,1,alen,f);
			if(needcpu>cpu)printf("  need %d86!",needcpu);
			printf("\n");
			break;
		case 5:printf("Invalid memory pointer\n");break;
		case 37:printf("Operands must be same size\n");break;
		case 38:printf("Invalid instruction operands\n");break;
		case 90:printf("Invalid operand size for instruction\n");break;
		case 91:printf("This instruction is not supported\n");break;
		default:printf("Error %d\n",err);
	}
}

main(int argc,char *argp[])
{
	char line[128],c;
	int i;
	FILE *f;
	for(i=1;i<argc;i++)checkparm(argp[i]);
	if(use32&&cpu<3)cpu=3;
	f=fopen("d.com","wb");
	fwrite(rcode,1,4,f);
	do
	{
		if(!*readstring(line))break;
		//c=scanf("%[^\n]%c",line,&c);if(!(c&&line[0]))break;
		assembleline(line,f);
	} while(1);
	fwrite(acode,1,255,f);
	fclose(f);
	return 0;
}
