#include "asmpr.c"

main(int argc,char *argp[])
{
	char line[128],*p;
	int	i,j,err;
	FILE *f;
	for(i=1;i<argc;i++)checkparm(argp[i]);
	if(use32&&cpu<3)cpu=3;
 // strcpy(line,"xchg ax,bx");
 // strcpy(line,"xchg ds:[si+bp+5],ax");
	if(str[0])strcpy(line,str);
	else scanf("%[^\n]",line);
	strlwr(line);
	p=line;while(*p)
	{
		if(*p==9)*p=' ';
		if(*p==';')*p=0;else p++;
	}
	p=line;while(*p==' ')p++;if(p>line)strcpy(line,p);
	p=line;while(*p)
	{
		if(*(int*)p==0x2020||*(int*)p==0x2c20)strcpy(p,p+1);
		else p++;
	}
	memset(par,0,sizeof(par));
	memset(b1,0,sizeof(b1));
	p=line;
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
	 case 0:printf("%d Bytes: ",alen);
		 for(j=0;j<alen;j++)printf("%.2X",acode[j]);printf("\n");
		 printf("DB ");for(j=0;j<alen;j++)printf("$%X,",acode[j]);printf("\n");
		 /*f=fopen("a.com","wb");
		 fwrite(rcode,1,4,f);
		 fwrite(acode,1,alen,f);
		 fwrite(acode,1,255,f);
		 fclose(f);*/
		 break;
	 case 5:printf("Invalid memory pointer\n");break;
	 case 37:printf("Operands must be same size\n");break;
	 case 38:printf("Invalid instruction operands\n");break;
	 case 90:printf("Invalid operand size for instruction\n");break;
tiins:case 91:printf("This instruction is not supported\n");break;
	 default:printf("Error %d\n",err);
	}
	return(0);
}


/*
info:	4-Bit	possib.size
	4-Bit	type(const,seg-reg,general-reg,memdist,mem)
	3-Bit	seg-reg-prefix
	5-Bit	info for type (e.g.type of reg->ax->0)
	4 Byte	extra data for mem (offset;1/2 Byte rel)

info:
0-ax	al	es	bx+si
1-cx	cl	cs	bx+di
2-dx	dl	ss	bp+si
3-bx	bl	ds	bp+di
4-sp	ah	fs	si
5-bp	ch	gs	di
6-si	dh		bp/[abs]
7-di	bh	nothing	bx

*/