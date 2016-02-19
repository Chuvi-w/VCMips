#ifndef HEADER_CMD5_H
#define HEADER_CMD5_H



#define MD5_LONG unsigned int
#define MD5_CBLOCK	64
#define MD5_LBLOCK	(MD5_CBLOCK/4)
#define MD5_DIGEST_LENGTH 16
#define MD5_STRING_LENGTH (MD5_DIGEST_LENGTH*2+1)


typedef union MD5Data_u
{

	MD5_LONG m32[4];
	uint64_t m64[2];
	unsigned char Buff[sizeof(MD5_LONG) * 4];
}MD5Data_t;
typedef TCHAR* FileName_t;
class CMD5
{
public:
	CMD5(char *buf);
	CMD5(MD5Data_t &md);
	CMD5(unsigned char *buf,size_t len);
	CMD5(FileName_t *file);
	~CMD5();

	BOOL Get(char *md5Buf,size_t szInBytes);
	BOOL Get(wchar_t *md5wBuf, size_t szInWords);
	MD5Data_t Get();
	
	BOOL Compare(const MD5Data_t &md5);
	BOOL Compare(const char *md5);
	BOOL Compare(const wchar_t *md5);
private:
	void Init();
	void Update(const void *data, size_t len);
	void Final();
	void Transform(const unsigned char *b);
	void block_data_order (const void *data_, size_t num);
	typedef struct MD5state_st
	{
		MD5Data_t md;
		MD5_LONG Nl,Nh;
		MD5_LONG data[MD5_LBLOCK];
		unsigned int num;
	} MD5_CTX;
	MD5_CTX c;

public: //Templates
	template <size_t _SizeInLetters, typename CharType> BOOL Get(CharType(&md5Buf)[_SizeInLetters])
	{
		return Get(md5Buf, _SizeInLetters);
	}
};

#endif
