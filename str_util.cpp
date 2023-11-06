#ifdef _WINDOWS
#include <windows.h>
#include <lmerr.h>
#endif
#include "str_util.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <string>

bool endWith(const std::string & str, const std::string& tail)
{
	if (str.size() <= tail.size()) return false;
	else
	return str.compare(str.size() - tail.size(), tail.size(), tail) == 0;
}

bool extract_id(const std::string& name, const std::string& prefix, int& id)
{
	size_t n0 = name.size(), n1 = prefix.size();
	if (n0 <= n1) return false;

	id = 0;
	for (size_t p = n0 - n1; p >= 0; --p) {
	//for (size_t p = n0 - n1; p > 0; --p) {
		if (name.substr(p, n1) != prefix)
		{
			if (p == 0) break;
			else
			continue;
		}

		const char* p2 = name.c_str() + p + n1;
		while ((*p2) >= '0' && (*p2) <= '9') {
			id = id * 10 + (int)((*p2) - '0');
			++p2;
		}
		return true;
	}
	return false;
}


void parse_frame_info(const std::string& frame_id, int& cam_id, int& exp_id, int& sample_id)
{

	//H9_TRACE("FRAME ID IS %s...", frame_id.c_str());
	if (!extract_id(frame_id, "_c", cam_id))
		cam_id = 0;
	if (!extract_id(frame_id, "_e", exp_id))
		exp_id = 0;
	if (extract_id(frame_id, "_s", sample_id) || extract_id(frame_id, "RODUCT_", sample_id))
		printf("This is image for sample %d, camera %d, exposure %d\n", sample_id, cam_id, exp_id);
}

char* _trim(char* s)
{
	char* pb = s;
    char* pe = s + strlen(s) - 1;
    while((*pb==' '||*pb=='\t'||*pb=='\n'||*pb=='\r') && *pb!='\0') pb++;
    while(pe != pb && (*pe==' '||*pe=='\t'||*pe=='\n'||*pe=='\r')) pe--;
    *(pe+1) = '\0';
    return pb;
}

StringList split(const char* s, char sep)
{
	StringList sl;
	char* pcStart = (char*)s;
	char* pc = NULL;
	while((pc = strchr(pcStart, sep)) != NULL)
	{
		sl.push_back(std::string());
		std::string& str = sl.back();
		str.append(pcStart, pc - pcStart);
		pcStart = pc + 1;
	}

	sl.push_back(std::string());
	std::string& str2 = sl.back();
	str2.append(pcStart);
	
	return sl;
}

StringList split(const char* s, const char* seps)
{
	StringList sl;
	const char* pb = s;
	char chr = '\0';
	int pos = 0;
	while(chr = pb[pos++], chr != '\0')
	{
		if(strchr(seps, chr) != NULL)
		{
			sl.push_back(std::string());
			std::string& sb = sl.back();
			if(pos > 1) sb.append(pb, pos-1);
			pb += pos - 1;
			pb = eatup(pb, seps);
			pos = 0;
		}
	}
	
	if(pb[0] != '\0') sl.push_back(pb);
	return sl;
}

const char* eatup(const char* pc, const char* delimiters)
{
	while(pc[0] != '\0' && strchr(delimiters, pc[0]) != NULL) pc++;
    return pc;
}

const char* eatuptil(const char* pc, const char* delimiters)
{
	while(pc[0] != '\0' && strchr(delimiters, pc[0]) == NULL) pc++;
    //if(pc[0] == '\0') return NULL;
    //else return pc;
	return pc;
}

const char* eatup(const char* pc, const char* delimiters, char* eated)
{
	const char* pNew = eatup(pc, delimiters);
	int len = (int)pNew - (int)pc;
	memcpy(eated, pc, len);
	eated[len] = '\0';
	return pNew;
}

const char* eatuptil(const char* pc, const char* delimiters, char* eated)
{
	const char* pNew = eatuptil(pc, delimiters);
	int len = (int)pNew - (int)pc;
	memcpy(eated, pc, len);
	eated[len] = '\0';
	return pNew;
}

const char* int2str(int n, char* buf)
{
	sprintf(buf, "%d", n);
	return buf;
}

int str2int(const char* pc)
{
	if(pc == NULL || pc[0] == '\0') return 0;

	bool negative = false;
	const char* pb = pc;
	if(pc[0] == '+' || pc[0] == '-')
	{
		pb = pc + 1;
		if(pc[0] == '-') negative = true;
	}

	size_t len = strlen(pb);
	if(len > 2 && pb[0] == '0' && toupper(pb[1]) == 'X')
		return strtol(pc,NULL,16);
	
	return strtol(pc,NULL,10);
}

bool isnumber(const char* pc)
{
	pc = eatup(pc, " \t\r");
    if(pc[0] == '-') pc++;
    pc = eatup(pc, "0123456789");
    return pc[0] == '\0';
}

bool isnumber2(const char* pc)
{
	pc = eatup(pc, " \t\r");
	if(pc[0] == '-') pc++;
	if(pc[0] == '0' && (pc[1] == 'x' || pc[1] == 'X')) pc += 2;
	pc = eatup(pc, "0123456789ABCDEFabcdef");
	return pc[0] == '\0';
}

bool isfloat(const char* pc)
{
	pc = eatup(pc, " \t\r");
    if(pc[0] == '-') pc++;
    pc = eatup(pc, "0123456789");
    
    if(pc[0] == '\0') return true;
    if(pc[0] != '.') return false;
    
    pc = eatup(pc+1, "0123456789");
    return pc[0] == '\0';
}

std::string& trim(std::string& str,const char* delimiters)
{
	return ltrim(rtrim(str," \t\r\n"), " \t\r\n");
}

std::string& ltrim(std::string& str,const char* delimiters)
{
	while(str.size() > 0)
	{
		if(strchr(delimiters, str[0]) != NULL) str.erase(0, 1);
		else break;
	}
	return str;
}

std::string& rtrim(std::string& str,const char* delimiters)
{
	int lastidx = 0;
	while(str.size() > 0)
	{
		lastidx = str.size() - 1;
		if(strchr(delimiters, str[lastidx]) != NULL) str.erase(lastidx, 1);
		else break;
	}
	return str;
}

std::string& lstrerase(std::string& str, const char* pattern)
{
	int len = strlen(pattern);
	if(strncmp(str.c_str(), pattern, len) != 0) return str;
	str.erase(0, len);
	return str;
}

std::string& rstrerase(std::string& str, const char* pattern)
{
	int size = str.size();
	int len = strlen(pattern);
	if(size < len) return str;
	const char* pc = str.c_str();
	pc += size - len;
	if(strncmp(pc, pattern, len) != 0) return str;
	str.erase(size-len, len);
	return str;
}

std::string& sreplace(std::string& str, const char* olds, const char* news)
{
	int beg = 0;
    int pos = 0;
    while(pos = str.find(olds, beg), pos != (int)std::string::npos)
    {
        str.replace(pos, strlen(olds), news);
        beg = pos + strlen(news);
    }
    return str;
}

//std::string getworkdir()
//{
//#ifdef _WINDOWS
//	char szFullDir[MAX_PATH];
//	char szDrv[MAX_PATH];
//	char szPath[MAX_PATH];
//	char szName[MAX_PATH];
//	char szExt[MAX_PATH];
//
//	GetModuleFileName(NULL, (LPWSTR)szFullDir, sizeof(szFullDir));
//	char* pSlash = strrchr(szFullDir, '\\');
//	if(pSlash == NULL) pSlash = strrchr(szFullDir, '/');
//	if(pSlash != NULL) pSlash[1] = '\0';
//	return szFullDir;	
//#else
//	return "./";
//#endif
//}

bool IsAbsolutePath(const char* filename)
{
#ifdef _WINDOWS
	return strchr(filename, ':') != NULL;
#else
	return filename[0] == '/';
#endif
}

std::string strerror_r2(int errnum)
{
	char buf[512];
	size_t n = sizeof(buf);
	
#ifdef _WINDOWS
	buf[0] = '\0';
	HMODULE hModule = NULL;
	static HMODULE hNetmsgMod = LoadLibraryEx(TEXT("netmsg.dll"),NULL,LOAD_LIBRARY_AS_DATAFILE);
	WORD langID = MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT);
	DWORD flags = FORMAT_MESSAGE_IGNORE_INSERTS|FORMAT_MESSAGE_FROM_SYSTEM;
	if(errnum >= NERR_BASE && errnum <= MAX_NERR)
	{
		hModule = hNetmsgMod;
		flags |= FORMAT_MESSAGE_FROM_HMODULE;
	}

	FormatMessageA(flags,hModule,errnum,langID,buf,n,NULL);
	return buf;
#else
	buf[0] = '\0';
	strerror_r(errnum, buf, n);
	return buf;
#endif
}

const char* basename_r(const char* filename)
{
	const char* p = strrchr(filename, '/');
	if(!p) p = strrchr(filename, '\\');
	if(!p) return filename;
	return p+1;
}

const char* dirname_r(const char* filename, char* buf)
{
	const char* bn = basename_r(filename);
	if(bn == filename) return ".";
	memcpy(buf, filename, bn-filename-1);
	buf[bn-filename-1] = '\0';
	return buf;
}

//struct tm* MyLocalTime(time_t t, struct tm& stm)
//{
//	memset(&stm, 0, sizeof(stm));
//#ifdef _WINDOWS
//	static ThreadMutex mux;
//	mux.Lock();
//	struct tm* ptm = localtime(&t);
//	if(ptm) stm = *ptm;
//	mux.Unlock();
//#else
//	localtime_r(&t, &stm);
//#endif
//	return &stm;
//}

//const char* LocalTimeString(time_t t, char* buf)
//{
//	struct tm sysTime;
//	MyLocalTime(t, sysTime);
//	sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d",
//		sysTime.tm_year+1900, sysTime.tm_mon+1, sysTime.tm_mday,
//		sysTime.tm_hour, sysTime.tm_min, sysTime.tm_sec);
//	return buf;
//}

//char* CurrentTimeString(char* buf)
//{
//	LocalTimeString(time(NULL), buf);
//	return buf;
//}

int StatFile(const char* filename, FileStat& fileStat)
{
#ifdef _WINDOWS
	return _stat(filename, &fileStat);
#else
	return stat(filename, &fileStat);
#endif
}
