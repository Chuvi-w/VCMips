#include "stdafx.h"
#include "CMD5.h"





#define MD32_REG_T int
#define BUFSIZE	1024*16




#ifndef HOST_c2l
#define HOST_c2l(c,l)	(l =(((unsigned long)(*((c)++)))    ),		\
			 l|=(((unsigned long)(*((c)++)))<< 8),		\
			 l|=(((unsigned long)(*((c)++)))<<16),		\
			 l|=(((unsigned long)(*((c)++)))<<24),		\
			 l)
#endif

#define	F(b,c,d)	((((c) ^ (d)) & (b)) ^ (d))
#define	G(b,c,d)	((((b) ^ (c)) & (d)) ^ (c))
#define	H(b,c,d)	((b) ^ (c) ^ (d))
#define	I(b,c,d)	(((~(d)) | (b)) ^ (c))

#define R0(a,b,c,d,k,s,t) { \
	a+=((k)+(t)+F((b),(c),(d))); \
	a=ROTATE(a,s); \
	a+=b; };\

#define R1(a,b,c,d,k,s,t) { \
	a+=((k)+(t)+G((b),(c),(d))); \
	a=ROTATE(a,s); \
	a+=b; };

#define R2(a,b,c,d,k,s,t) { \
	a+=((k)+(t)+H((b),(c),(d))); \
	a=ROTATE(a,s); \
	a+=b; };

#define R3(a,b,c,d,k,s,t) { \
	a+=((k)+(t)+I((b),(c),(d))); \
	a=ROTATE(a,s); \
	a+=b; };

#ifndef ROTATE
#define ROTATE(a,n)     (((a)<<(n))|(((a)&0xffffffff)>>(32-(n))))
#endif

CMD5::~CMD5()
{
	
}


CMD5::CMD5(FileName_t *file)
{
	FILE *f=NULL;
	
	if (!_tfopen_s(&f, (const TCHAR*)file, _T("rb")))
	{
		int i;
		static unsigned char buf[BUFSIZE];
		Init();
		for (;;)
		{
			i=fread(buf,1,BUFSIZE,f);
			if (i <= 0) break;
			Update(buf,(unsigned long)i);
		}
		Final();
		fclose(f);
	}
}


CMD5::CMD5(char *buf)
{
	Init();
	Update(buf,strlen((const char*)buf));
	Final();
}


CMD5::CMD5(unsigned char *buf,size_t len)
{
	Init();
	Update(buf,len);
	Final();
}

CMD5::CMD5(MD5Data_t &md)
{
	Init();
	for (int i = 0; i < 2; i++)
	{
		c.md.m64[i] = md.m64[i];
	}
}


BOOL CMD5::Get(char *md5Buf, size_t szInBytes)
{
	if ((!c.md.m32[0]&&!c.md.m32[1]&&!c.md.m32[2]&&!c.md.m32[3]) || szInBytes < MD5_DIGEST_LENGTH + 1)
	{
		return FALSE;
	}
	memset(md5Buf, 0, szInBytes*sizeof(*md5Buf));
	
	for (int i = 0; i < MD5_DIGEST_LENGTH; i++)
	{
		sprintf_s(&md5Buf[2 * i], szInBytes - 2 * i, "%02x", c.md.Buff[i]);
	}
	return TRUE;
}

BOOL CMD5::Get(wchar_t *md5wBuf, size_t szInWords)
{
	if ((!c.md.m32[0]&&!c.md.m32[1]&&!c.md.m32[2]&&!c.md.m32[3]) || szInWords < MD5_DIGEST_LENGTH + 1)
	{
		return FALSE;
	}
	memset(md5wBuf, 0, szInWords*sizeof(*md5wBuf));
	for (int i = 0; i < MD5_DIGEST_LENGTH; i++)
	{
		swprintf_s(&md5wBuf[2 * i], szInWords - 2 * i, L"%02x", c.md.Buff[i]);
	}
	return TRUE;
}

MD5Data_t CMD5::Get()
{
	return c.md;
}


BOOL CMD5::Compare(const MD5Data_t &md5)
{
	for (int i = 0; i < 2; i++)
	{
		if (md5.m64[i] != c.md.m64[i])
		{
			return FALSE;
		}
	}
	return TRUE;
}

BOOL CMD5::Compare(const char *md5)
{
	char t[MD5_STRING_LENGTH];
	Get(t);
	return (!_stricmp(md5, t));
}
BOOL CMD5::Compare(const wchar_t *md5)
{
	wchar_t t[MD5_STRING_LENGTH];
	Get(t);	
	return (!_wcsicmp(md5, t));
}


void CMD5::Init()
{
	memset (&c,0,sizeof(c));
	c.md.m32[0] = (unsigned long)0x67452301L;
	c.md.m32[1] = (unsigned long)0xefcdab89L;
	c.md.m32[2] = (unsigned long)0x98badcfeL;
	c.md.m32[3] = (unsigned long)0x10325476L;
}





void  CMD5::Update (const void *data_, size_t len)
{
	const unsigned char *data=(const unsigned char *)data_;
	unsigned char *p;
	unsigned int l;
	size_t n;

	if (len==0) return;

	l=(c.Nl+(((unsigned int)len)<<3))&0xffffffffUL;


	if (l < c.Nl) 
	c.Nh++;
	c.Nh+=(unsigned int)(len>>29);	
	c.Nl=l;

	n = c.num;
	if (n != 0)
	{

		p=(unsigned char *)c.data;

		if (len >= 64 || len+n >= 64)
		{

			memcpy (p+n,data,64-n);
			block_data_order (p,1);
			n      = 64-n;
			data  += n;
			len   -= n;
			c.num = 0;
			memset (p,0,64);	
		}
		else
		{

			memcpy (p+n,data,len);
			c.num += (unsigned int)len;
			return;
		}
	}

	n = len/64;
	if (n > 0)
	{

		block_data_order (data,n);
		n    *= 64;
		data += n;
		len  -= n;
	}

	if (len != 0)
	{

		p = (unsigned char *)c.data;
		c.num = (unsigned int)len;
		memcpy (p,data,len);
	}
	return;

}

void  CMD5::Final ()
{
	unsigned char *p = (unsigned char *)c.data;
	size_t n = c.num;

	p[n] = 0x80; 
	n++;

	if (n > (64-8))
	{

		memset (p+n,0,64-n);
		n=0;
		block_data_order (p,1);
	}
	memset (p+n,0,64-8-n);

	p += 64-8;
	(void)(*((p)++)=(unsigned char)(((c.Nl) )&0xff), *((p)++)=(unsigned char)(((c.Nl)>> 8)&0xff), *((p)++)=(unsigned char)(((c.Nl)>>16)&0xff), *((p)++)=(unsigned char)(((c.Nl)>>24)&0xff), c.Nl);
	(void)(*((p)++)=(unsigned char)(((c.Nh) )&0xff), *((p)++)=(unsigned char)(((c.Nh)>> 8)&0xff), *((p)++)=(unsigned char)(((c.Nh)>>16)&0xff), *((p)++)=(unsigned char)(((c.Nh)>>24)&0xff), c.Nh);
	p -= 64;
	block_data_order (p,1);
	c.num=0;
	memset (p,0,64);
}


void CMD5::block_data_order (const void *data_, size_t num)
{
	const unsigned char *data=(const unsigned char *)data_;
	register unsigned MD32_REG_T A,B,C,D,l;
	unsigned MD32_REG_T	XX0, XX1, XX2, XX3, XX4, XX5, XX6, XX7,
				XX8, XX9,XX10,XX11,XX12,XX13,XX14,XX15;
	# define X(i)	XX##i


	A=c.md.m32[0];
	B=c.md.m32[1];
	C=c.md.m32[2];
	D=c.md.m32[3];

	for (;num--;)
	{
		HOST_c2l(data,l); X( 0)=l;		HOST_c2l(data,l); X( 1)=l;
		/* Round 0 */
		R0(A,B,C,D,X( 0),  7,0xd76aa478L);	HOST_c2l(data,l); X( 2)=l;
		R0(D,A,B,C,X( 1), 12,0xe8c7b756L);	HOST_c2l(data,l); X( 3)=l;
		R0(C,D,A,B,X( 2), 17,0x242070dbL);	HOST_c2l(data,l); X( 4)=l;
		R0(B,C,D,A,X( 3), 22,0xc1bdceeeL);	HOST_c2l(data,l); X( 5)=l;
		R0(A,B,C,D,X( 4),  7,0xf57c0fafL);	HOST_c2l(data,l); X( 6)=l;
		R0(D,A,B,C,X( 5), 12,0x4787c62aL);	HOST_c2l(data,l); X( 7)=l;
		R0(C,D,A,B,X( 6), 17,0xa8304613L);	HOST_c2l(data,l); X( 8)=l;
		R0(B,C,D,A,X( 7), 22,0xfd469501L);	HOST_c2l(data,l); X( 9)=l;
		R0(A,B,C,D,X( 8),  7,0x698098d8L);	HOST_c2l(data,l); X(10)=l;
		R0(D,A,B,C,X( 9), 12,0x8b44f7afL);	HOST_c2l(data,l); X(11)=l;
		R0(C,D,A,B,X(10), 17,0xffff5bb1L);	HOST_c2l(data,l); X(12)=l;
		R0(B,C,D,A,X(11), 22,0x895cd7beL);	HOST_c2l(data,l); X(13)=l;
		R0(A,B,C,D,X(12),  7,0x6b901122L);	HOST_c2l(data,l); X(14)=l;
		R0(D,A,B,C,X(13), 12,0xfd987193L);	HOST_c2l(data,l); X(15)=l;
		R0(C,D,A,B,X(14), 17,0xa679438eL);
		R0(B,C,D,A,X(15), 22,0x49b40821L);
		/* Round 1 */
		R1(A,B,C,D,X( 1),  5,0xf61e2562L);
		R1(D,A,B,C,X( 6),  9,0xc040b340L);
		R1(C,D,A,B,X(11), 14,0x265e5a51L);
		R1(B,C,D,A,X( 0), 20,0xe9b6c7aaL);
		R1(A,B,C,D,X( 5),  5,0xd62f105dL);
		R1(D,A,B,C,X(10),  9,0x02441453L);
		R1(C,D,A,B,X(15), 14,0xd8a1e681L);
		R1(B,C,D,A,X( 4), 20,0xe7d3fbc8L);
		R1(A,B,C,D,X( 9),  5,0x21e1cde6L);
		R1(D,A,B,C,X(14),  9,0xc33707d6L);
		R1(C,D,A,B,X( 3), 14,0xf4d50d87L);
		R1(B,C,D,A,X( 8), 20,0x455a14edL);
		R1(A,B,C,D,X(13),  5,0xa9e3e905L);
		R1(D,A,B,C,X( 2),  9,0xfcefa3f8L);
		R1(C,D,A,B,X( 7), 14,0x676f02d9L);
		R1(B,C,D,A,X(12), 20,0x8d2a4c8aL);
		/* Round 2 */
		R2(A,B,C,D,X( 5),  4,0xfffa3942L);
		R2(D,A,B,C,X( 8), 11,0x8771f681L);
		R2(C,D,A,B,X(11), 16,0x6d9d6122L);
		R2(B,C,D,A,X(14), 23,0xfde5380cL);
		R2(A,B,C,D,X( 1),  4,0xa4beea44L);
		R2(D,A,B,C,X( 4), 11,0x4bdecfa9L);
		R2(C,D,A,B,X( 7), 16,0xf6bb4b60L);
		R2(B,C,D,A,X(10), 23,0xbebfbc70L);
		R2(A,B,C,D,X(13),  4,0x289b7ec6L);
		R2(D,A,B,C,X( 0), 11,0xeaa127faL);
		R2(C,D,A,B,X( 3), 16,0xd4ef3085L);
		R2(B,C,D,A,X( 6), 23,0x04881d05L);
		R2(A,B,C,D,X( 9),  4,0xd9d4d039L);
		R2(D,A,B,C,X(12), 11,0xe6db99e5L);
		R2(C,D,A,B,X(15), 16,0x1fa27cf8L);
		R2(B,C,D,A,X( 2), 23,0xc4ac5665L);
		/* Round 3 */
		R3(A,B,C,D,X( 0),  6,0xf4292244L);
		R3(D,A,B,C,X( 7), 10,0x432aff97L);
		R3(C,D,A,B,X(14), 15,0xab9423a7L);
		R3(B,C,D,A,X( 5), 21,0xfc93a039L);
		R3(A,B,C,D,X(12),  6,0x655b59c3L);
		R3(D,A,B,C,X( 3), 10,0x8f0ccc92L);
		R3(C,D,A,B,X(10), 15,0xffeff47dL);
		R3(B,C,D,A,X( 1), 21,0x85845dd1L);
		R3(A,B,C,D,X( 8),  6,0x6fa87e4fL);
		R3(D,A,B,C,X(15), 10,0xfe2ce6e0L);
		R3(C,D,A,B,X( 6), 15,0xa3014314L);
		R3(B,C,D,A,X(13), 21,0x4e0811a1L);
		R3(A,B,C,D,X( 4),  6,0xf7537e82L);
		R3(D,A,B,C,X(11), 10,0xbd3af235L);
		R3(C,D,A,B,X( 2), 15,0x2ad7d2bbL);
		R3(B,C,D,A,X( 9), 21,0xeb86d391L);

		A = c.md.m32[0] += A;
		B = c.md.m32[1] += B;
		C = c.md.m32[2] += C;
		D = c.md.m32[3] += D;
	}
}



