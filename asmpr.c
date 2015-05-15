#include "asm1.c"

//include file for lasm

int prefix(void);

int strproc(int base)
{
	switch(*rem)
	{
		default:return(91);
		case 0://if(typ!=tmem0||info!=di/si)return(38);
			putaddrsizepref();
			if(testopsize(0))acode[alen++]=0x66;
			if(par[1][0])putsegregpref(1);
			//putsegregpref(0);
			acode[alen++]=base+!(b1[0].size&1)+(opnum2<<1);return(0);
		case 'd':if(!use32)acode[alen++]=0x66;break;
		case 'w':if(use32)acode[alen++]=0x66;
		case 'b':;
	}
	acode[alen++]=base+(*rem!='b')+(opnum2<<1);
	return(0);
}
prstr()
{return(strproc(0xaa));}
prstr2()
{opnum2++;return(strproc(0xa4));}
prstr3(void)
{cputype(2);return(strproc(0x6a));}

prmov()
{
	int rev;
	if(*rem=='s'&&*(rem+1)!='x'){rem++;return(strproc(0xa4));}

	if(b1[0].typ==tMMXreg && b1[1].typ<=tsimplereg)
	{xchgpar(0,1);rev=0;}else rev=16;
	if(*rem=='d'){
		putaddrsizepref();
		cputype(6);
		if(b1[0].typ>tsimplereg || b1[1].typ!=tMMXreg)return(38);
		if(!(b1[0].size & 4))return(90);
		putrmxop(b1[1].info, 0, /*01101110*/0x6e | rev);
		return(0);
	}
	if(*rem=='q'){
		putaddrsizepref();
		cputype(6);
		if((b1[0].typ>=tsimplereg && b1[0].typ!=tMMXreg) || b1[1].typ!=tMMXreg)return(38);
		if(!(b1[0].size & 8))return(90);
		putrmxop(b1[1].info, 0, /*01101111*/0x6f | rev);
		return(0);
	}
	putopsizepref();
	if(*(rem+1)=='x')
	{
	//if(b1[0].size<getlowestbit(b1[1].size))return(90);
		cputype(3);
		putrmxop(b1[0].info,1,0xb7-(b1[1].size&1)+((*rem=='s')<<3));
		return(0);
	}
	if(b1[1].typ==tsimplereg||b1[0].typ==tsegreg)
	{xchgpar(0,1);rev=2;}else rev=0;
	switch(b1[1].typ)
	{
		default:return(38);
		case tsegreg:putrmop(b1[1].info,0,0x8c+rev);break;
		case tconstant:if(rev)return(38);
			if(b1[0].typ<=tmem2)
			{
				putrmop(0,0,0xc7-(b1[0].size&b1[1].size&1));
				appendimdata(1,getlowestbit(b1[0].size&b1[1].size));
				break;
			}
			acode[alen++]=0xb8-((b1[0].size&1)<<3)|b1[0].info;
			appendimdata(1,getlowestbit(b1[0].size));
			break;
		case tmem0:if(b1[0].typ==tsimplereg&&!b1[0].info&&b1[1].info==6)
			{putsegregpref(1);acode[alen++]=0xa0+rev+!(b1[0].size&1);appendrmdata(1);break;}
		case tmem1:case tmem2:case tsimplereg:putrmop(b1[0].info,1,0x8a-rev+!(b1[0].size&1));
	}
	return(0);
}

prxchg()
{
	putopsizepref();
	if(!(b1[0].size&b1[1].size))return(37);
	if((b1[1].typ==tsimplereg&&b1[1].size==2&&!b1[1].info)
	 ||(b1[0].typ!=tsimplereg)) xchgpar(0,1);
	if((b1[1].typ==tsimplereg)&&(b1[0].typ==tsimplereg&&b1[0].size==2&&!b1[0].info))
	{
		acode[alen++]=b1[1].info|0x90;
		return(0);
	}else
	{
		if(b1[0].typ!=tsimplereg||b1[1].typ>tsimplereg)return(38);
		b1[0].size&=b1[1].size;
		putrmop(b1[0].info,1,0x87-(b1[0].size&1));
		return(0);
	}
}

princdec()
{
	putopsizepref();
	if(b1[0].typ==tsimplereg&&!(b1[0].size&1))acode[alen++]=0x40|(opnum2<<3)|b1[0].info;
	else if(b1[0].typ<=tsimplereg) putrmop(opnum2,0,0xff-(b1[0].size&1));
	else return(38);
	return(0);
}

prmath()
{
	int xchg=0,sh;
	putopsizepref();
	if(b1[0].typ>tsimplereg)return(38);
	if(b1[0].size<getlowestbit(b1[1].size))return(90);
	if(b1[1].typ<=tmem2){xchg=2;xchgpar(0,1);}
	if(b1[1].typ==tsimplereg)
	{
		if(!(b1[0].size&b1[1].size))return(37);
		putrmop(b1[1].info,0,xchg+!(b1[1].size&1)+(opnum2<<3));
	}else if(!xchg&&b1[1].typ==tconstant)
	{
		if(b1[0].typ==tsimplereg&&!b1[0].info)
		{
			acode[alen++]=5-(b1[0].size&1)+(opnum2<<3);
			appendimdata(1,getlowestbit(b1[0].size));
		}else
		{
			sh=((!(b1[0].size&1)&&isshort(b1[1].data))<<1);
			putrmop(opnum2,0,0x81-(b1[0].size&1)+sh);
			appendimdata(1,getlowestbit(sh?b1[1].size:b1[0].size));
		}
	}else return(38);
	return(0);
}

prtest()
{
	putopsizepref();
	if(b1[1].typ<=tsimplereg&&b1[0].typ!=tsimplereg)xchgpar(0,1);
	if(b1[0].size<getlowestbit(b1[1].size))return(90);
	if(b1[1].typ==tconstant)
	{
		if(b1[0].typ==tsimplereg&&!b1[0].info)
		 {acode[alen++]=0xa9-(b1[0].size&1);appendimdata(1,b1[0].size);return(0);}
		putrmop(0,0,0xf7-(b1[0].size&1));
		appendimdata(1,getlowestbit(b1[1].size&b1[0].size));
	}else if(b1[1].typ<=tsimplereg) putrmop(b1[0].info,1,0x85-(b1[0].size&1));
	return(0);
}

prsingle()
{
	putopsizepref();
	if(opnum2==3)
	{
		if(b1[2].typ==tconstant)//imul:0-reg,1-rm,2-const
		{
			putrmop(b1[0].info,1,0x69+((b1[0].size&1)<<1));
			appendimdata(2,b1[0].size);
			return(0);
		}
	}
	putrmop(2+opnum2,0,0xf7-(b1[0].size&1));
	return(0);
}

prshft()
{
	int isr,op,im;
	putopsizepref();
	if(*rem=='r')isr=1;
	else if(*rem=='l')isr=0;
	else return(91);
	rem++;
	if(*rem=='d')
	{
		//if(opnum2!=2)return(91);
		cputype(3);
		if(b1[2].typ==tconstant)
		{
			putrmxop(b1[1].info,0,0xa4+(isr<<3));
			appendimdata(2,1);
		}else putrmxop(b1[1].info,0,0xa5+(isr<<3));
		return(0);
	}else if(*rem)return(91);
	im=0;
	if(!isr&&opnum2==3)opnum2=2;	//sal=shl
	if(b1[1].typ==tconstant)
	{
		if(b1[1].data==1)op=0xd0;
		else{cputype(2);op=0xc0;im++;}
	}else if(b1[1].typ==tsimplereg&&b1[1].info==1&&(b1[1].size&1))op=0xd2;
	else return(38);
	putrmop((opnum2<<1)+isr,0,op+!(b1[0].size&1));
	appendimdata(1,im);
	return(0);
}

praa()
{
	switch(*rem)
	{
		default: return(91);
		case 'm':acode[alen++]=0xd4;break;
		case 'd':acode[alen++]=0xd5;
	}
	acode[alen++]=0xa;
	return(0);
}

prbt()
{
	int x=0;
	switch(*rem)
	{
		default:return(91);
		case 'c':x++;
		case 'r':x++;
		case 's':x++;
		case 0:;
	}
	if(*rem&&*(rem+1))return(91);
	cputype(3);
	putopsizepref();
	if(b1[1].typ==tconstant)
	{
		putrmxop(x+4,0,0xba);
		appendimdata(1,1);
	}else	putrmxop(b1[1].info,0,0xa3+(x<<3));
	return(0);
}

prbs()
{
	putopsizepref();
	if(*(rem+1))return(91);
	if(*rem=='r')opnum2++;
	else if(*rem!='f')return(91);
	cputype(3);
	//if(b1[0].typ!=tsimplereg||b1[1].typ>tsimplereg)return(38);
	//if(b1[0].size...b1[1].size)return(90);
	putrmxop(b1[0].info,1,0xbc+opnum2);
	return(0);
}

prxchgop()
{
	cputype(4);
	putopsizepref();
	putrmxop(b1[1].info,0,0xb1+(opnum2<<4));
	return(0);
}

prbswap()
{
	if(b1[0].size&1)return(90);
	if(b1[0].typ!=tsimplereg)return(38);
	cputype(4);
	putopsizepref();
	acode[alen++]=0xf;
	acode[alen++]=0xc8+b1[0].info;
  return(0);
}

prenter()
{
	cputype(2);
	acode[alen++]=0xc8;
	appendimdata(0,2);
	appendimdata(1,1);
	return(0);
}

prpushpop(void)
{
	putaddrsize(0);
	//putaddrsizepref();
	if(testopsize(0))acode[alen++]=0x66;
	//putopsizepref();
	switch(*rem)
	{
		default:return(91);
		case 'a':cputype(2);acode[alen++]=0x60+opnum2;break;
		case 'f':acode[alen++]=0x9c+opnum2;break;
		case 0:if(!b1[0].size)return(38);else switch(b1[0].typ)
		{
			case tsimplereg:acode[alen++]=0x50|(b1[0].info)|(opnum2<<3);break;
			case tsegreg:if(b1[0].info<4)acode[alen++]=6+(b1[0].info<<3)+opnum2;
				else{acode[alen++]=0xf;acode[alen++]=0x80+(b1[0].info<<3)+opnum2;}break;
			case tconstant:if(opnum2)return(38);
				cputype(2);
				acode[alen++]=0x68+((b1[0].size&1)<<1);
				appendimdata(0,getlowestbit(b1[0].size));
				break;
			default:if(opnum2)putrmop(0,0,0x8f);else putrmop(6,0,0xff);
		}
	}
	if(par[1]){shiftpar();return(prpushpop());}
	return(0);
}

prjmp()
{
	if(b1[0].typ==tsimplereg)putopsizepref();
	 else putaddrsizepref();
	if(*rem=='f')b1[0].size=4;
	if(b1[0].typ==tconstant)
	 if(opnum2&&isshort(b1[0].data)&&(b1[0].size&1)){acode[alen++]=0xeb;appendimdata(0,1);}
	 else if(b1[0].size&3){acode[alen++]=0xe8+opnum2;appendimdata(0,2);}
	 else {acode[alen++]=opnum2?0xea:0x9a;
	 appendimdata(0,use32?4:2);memcpy(&acode[alen],&b1[0].segdata,2);alen+=2;}
	else putrmop((opnum2<<1)+3-((b1[0].size&2)>>1),0,0xff);
	return(0);
}

prjcxz()
{
	if(opnum2^use32){cputype(3);acode[alen++]=0x67;}
//if(!isshort(b1[0].data))return(90);
	acode[alen++]=0xe3;
	appendimdata(0,1);
	return(0);
}
prloop()
{
	int val=0xe2;
	if(*rem=='n'){val--;rem++;}
	if(*rem=='z'||*rem=='e')val--;
//if(!isshort(b1[0].data))return(90);
	acode[alen++]=val;
	appendimdata(0,1);
	return(0);
}

prclst()
{
	int val=0xf8;
	switch(*rem)
	{
		default:return(91);
		case 'i':val+=2;
		case 'c':break;
		case 'd':val+=4;
	}
	acode[alen++]=val|opnum2;
	return(0);
}

prcwd()
{
	if((opnum2<2)^use32){cputype(3);acode[alen++]=0x66;}
	acode[alen++]=0x98|(opnum2&1);
	return(0);
}

print()
{
	int val=0xcd;
	if(*rem=='o')val++;
	else if(*rem)return(91);
	if(b1[0].data==3)val--;
//if(b1[0].data>=256)return(90);
	acode[alen++]=val;
	if(val==0xcd)appendimdata(0,1);
	return(0);
}

prret()
{
	int ad=(*rem=='f')<<3;
	if(b1[0].typ==tconstant)
	{
		acode[alen++]=0xc2+ad;
		appendimdata(0,2);
	}else acode[alen++]=0xc3+ad;
	return(0);
}

prseg()
{
	int i;
	for(i=0;i<segregnum;i++)if(*rem==segregstr[i])break;
	//if(i>=segregnum&&b1[0].typ!=tsegreg)return(38);
	if(i>=segregnum)i=b1[0].info;
	acode[alen++]=getsegregpref(i);
	return(prefix());
}

prrep()
{
	acode[alen++]=0xf2+(!*rem||*rem=='e'||*rem=='z');
	return(prefix());
}

prinout()
{
	int ad;
	if(*rem=='s'){rem++;return(prstr3());}
	if(opnum2)xchgpar(0,1);
	if(b1[0].typ!=tsimplereg||!b1[1].size&2)return(38);
	putopsizepref();
	ad=!(b1[0].size&1)+(opnum2<<1);
	if(b1[1].typ==tconstant)
	{
		if(!b1[1].size&1)return(90);
		acode[alen++]=0xe4+ad;
		appendimdata(1,1);
	}else
	{
		if(b1[1].typ!=tsimplereg||b1[1].info!=2||b1[1].size!=2)return(38);
		acode[alen++]=0xec+ad;
	}
	return(0);
}

prlea()
{
	//if(!(b1[0].size&b1[1].size))return(37);
	putopsizepref();
	acode[alen++]=0x8d;putfullrm(b1[0].info,1);
	return(0);
}
prles()
{
	putaddrsizepref();
	if(testopsize(0))acode[alen++]=0x66;
	putrmop(b1[0].info,1,0xc4+opnum2);
	return(0);
}
prlxs()
{
	cputype(3);
	putaddrsizepref();
	if(testopsize(0))acode[alen++]=0x66;
	if(opnum2)opnum2++;
	putrmxop(b1[0].info,1,0xb2+opnum2);
	return(0);
}

char condi[36][4]={
	"o","no","b"  ,"nb","e","ne","be","nbe","s","ns","p" ,"np","l"  ,"nl","le","nle",
	"" ,""  ,"nae","ae","z","nz","na","a"  ,"" ,""  ,"pe","po","nge","ge","ng","g",
	"" ,""  ,"c"  ,"nc"};

getcondinum(void)
{
	int i;
	for(i=0;i<36;i++) if(condi[i][0]&&!strncmp(condi[i],rem,strlen(condi[i]))) return(i&15);
	return(-1);
}

prj()
{
	int i;
	if((i=getcondinum())==-1)return(91);
	if(isshort(b1[0].data)){acode[alen++]=0x70|i;appendimdata(0,1);}
	else {cputype(3);acode[alen++]=0xf;acode[alen++]=0x80|i;appendimdata(0,2);}
	return(0);
}

prset()
{
	int i;
	cputype(3);
	putaddrsizepref();
	if((i=getcondinum())==-1)return(91);
//if(!(b1[0].size&1))return(90);
	putrmxop(0,0,0x90|i);
	return(0);
}

prcpuid()
{
	cputype(5);
	acode[alen++]=0xf;
	acode[alen++]=0xa2;
	return(0);
}
premms()
{
	cputype(6);
	acode[alen++]=0xf;
	acode[alen++]=0x77;
	return(0);
}

MMadd(){
	int sz=(b1[0].size==0xf?b1[1].size:b1[0].size),optyp=0;
	if (*rem=='s') {optyp=0x10;rem++;}
	if (*rem=='u') {optyp=0x20;rem++;//&&*(rem)=='s'
	  rem++;}
	if (*rem=='d') sz=4;
	else if (*rem=='w') sz=2;
	else if (*rem=='b') sz=1;
	else if (*rem) return(91);

	//if(sz==0xf)warning Byte Ptr assumed
	if (b1[1].typ>=tmem2 && b1[1].typ!=tMMXreg) return(38);
	if (sz==8) return(90);
	putaddrsizepref();
	putrmxop(b1[0].info,1,0xfc-(opnum2<<2)+dobsf(sz)-optyp);
	return(0);
}
MMunpck()
{
	int sz=(b1[0].size==0xf?b1[1].size:b1[0].size/2),optyp;
	if (*rem=='l') optyp=0;
	else if (*rem=='h') optyp=8;
	else return(91);
	rem++;
	if (*rem=='d') sz=4;
	else if (*rem=='w') sz=2;
	else if (*rem=='b') sz=1;
	else if (*rem) return(91);
	//if(sz==0xf)warning Byte Ptr assumed
	if (b1[1].typ>=tmem2 && b1[1].typ!=tMMXreg) return(38);
	if (b1[0].size==8 || b1[1].size==1) return(90);
	putaddrsizepref();
	putrmxop(b1[0].info,1,0xa0+dobsf(sz)-optyp);
	return(0);
}
MMshift()
{
	int sz=(b1[0].size==0xf?b1[1].size:b1[0].size);
	opnum2++;
	if (*rem=='q') sz=8;
	else if (*rem=='d') sz=4;
	else if (*rem=='w') sz=2;
	else if (*rem) return(91);
	sz&=0xe;
	//if(sz==0xe)warning Word Ptr assumed
	putaddrsizepref();
	if (b1[0].typ!=tMMXreg) return(38);
	if ((opnum2==2 && sz==8) || sz==1) return(90);
	if (b1[1].typ==tconstant) {
	  putrmxop(opnum2<<1,0,0x70+dobsf(sz));appendimdata(1,1);
	} else if (b1[1].typ<=tmem2 || b1[1].typ==tMMXreg) putrmxop(b1[0].info,1,0xc0+(opnum2<<4)+dobsf(sz));
	else return(38);
	return(0);
}
prcmpMM()
{
	int sz=(b1[0].size==0xf?b1[1].size:b1[0].size);
	if (*rem=='d') sz=4;
	else if (*rem=='w') sz=2;
	else if (*rem=='b') sz=1;
	else if (*rem) return(91);
	//if(sz==0xf)warning Byte Ptr assumed
	putaddrsizepref();
	if (b1[0].typ!=tMMXreg)return(38);
	if (sz==8) return(90);
	if (b1[1].typ>=tmem2 && b1[1].typ!=tMMXreg) return(38);
	putrmxop(b1[0].info,1,0x64+(opnum2<<4)+dobsf(sz));
	return(0);
}
/*MMand()
{putaddrsizepref();putrmxop(b1[0].info,1,0xdb+(*rem=='n'?4:0));return(0);}
MMmadd()
{putaddrsizepref();putrmxop(b1[0].info,1,0xf5);return(0);}*/

#define oneMMopnum 10
char oneMMop[oneMMopnum]={0xdf,0xdb,0xf5,0xeb,0xef,0xe5,0xd5,
						0x63,0x67,0x6b};
proneMM(){
  if (b1[1].typ>=tmem2 && b1[1].typ!=tMMXreg) return(38);
  putaddrsizepref();
  putrmxop(b1[0].info,1,oneMMop[opnum2]);
  return(0);
}

#define MMmaxopcode 58
char	MMopcode[MMmaxopcode][8]={"add","sub","unpck",
			"andn","and","madd","or","xor","mulh","mull",
			"srl","sra","sll",
			"acksswb","ackssdw","ackuswb",
			"cmpgt","cmpeq",
		0};
int		(*MMopproc[MMmaxopcode])(void)={MMadd,MMadd,MMunpck,
			proneMM,proneMM,proneMM,proneMM,proneMM,proneMM,proneMM,
			MMshift,MMshift,MMshift,
			proneMM,proneMM,proneMM,
			prcmpMM,prcmpMM};

prp()
{
	int j,MMopnum;
	cputype(6);
	if (b1[0].typ!=tMMXreg) return(38);
	for(MMopnum=0;MMopnum<MMmaxopcode;MMopnum++)
	{
		if(!MMopcode[MMopnum][0])return(91);
		if(!strncmp(MMopcode[MMopnum],rem,strlen(MMopcode[MMopnum])))
		{
			rem+=strlen(MMopcode[MMopnum]);
			opnum2=0;for(j=MMopnum-1;j>=0;j--)if(MMopproc[j]==MMopproc[MMopnum])opnum2++;
			return((*MMopproc[MMopnum])());
		}
	}
	return(91);
}

#define oneopnum 11
char oneop[oneopnum]={
		0xf5,0x9e,0x9f,0xcf,0xc9,0x90,
		0x27,0x2f,0x37,0x3f,0xf0};
prone(){acode[alen++]=oneop[opnum2];return(0);}

char	opcode[maxopcode][8]={
		"push","pop","p",
		"stos","lods","scas","cmps",
		"mov","xchg","inc","dec",
		"bswap","bt","bs","cmpxchg","xadd",
		"add","or","adc","sbb","and","sub","xor","cmp",
		"not","neg","mul","imul","div","idiv",
		"test","cl","st",
		"cwde","cdq","cbw","cwd",
		"cmc","sahf","lahf","iret","leave","nop",
		"daa","das","aaa","aas","aa",
		"enter","ro","rc","sh","sa",
		"call","jmp","jcxz","jecxz","j","loop","set",
		"int","ret","seg","rep","lock",
		"in","out","lea","les","lds","lss","lfs","lgs",
		"cpuid","emms",
		0};

int	(*opproc[maxopcode])(void)={
		prpushpop,prpushpop,prp,//P = Packed Int (MMX) Ops
		prstr,prstr,prstr,prstr2,
		prmov,prxchg,princdec,princdec,
		prbswap,prbt,prbs,prxchgop,prxchgop,
		prmath,prmath,prmath,prmath,prmath,prmath,prmath,prmath,
		prsingle,prsingle,prsingle,prsingle,prsingle,prsingle,
		prtest,prclst,prclst,
		prcwd,prcwd,prcwd,prcwd,
		prone,prone,prone,prone,prone,prone,
		prone,prone,prone,prone,praa,
		prenter,prshft,prshft,prshft,prshft,
		prjmp,prjmp,prjcxz,prjcxz,prj,prloop,prset,
		print,prret,prseg,prrep,prone,
		prinout,prinout,prlea,prles,prles,prlxs,prlxs,prlxs,
		prcpuid,premms
		};

//62:bound,63:arpl,d7:xlat,f4:halt,9b:wait,copro-Befehle,
//286-protection,386:mov ??n,486:0f0?:inv*

int execproc(char *line)
{
	int j,opnum;
	for(opnum=0;opnum<maxopcode;opnum++)
	{
		if(!opcode[opnum][0])return(91);
		if(!strncmp(opcode[opnum],line,strlen(opcode[opnum])))
		{
			rem=line+strlen(opcode[opnum]);
			opnum2=0;for(j=opnum-1;j>=0;j--)if(opproc[j]==opproc[opnum])opnum2++;
			return((*opproc[opnum])());
		}
	}
	return(91);
}

prefix()
{
	char *p=par[0];
	shiftpar();
	return(execproc(p));
}