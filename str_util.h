#ifndef _STR_UTIL_H_
#define _STR_UTIL_H_

#include <time.h>
#include <string>
#include <vector>
#include <map>
#include <string.h>

#ifdef _WINDOWS
#define strcasecmp _stricmp
#ifndef errno
#define errno GetLastError()
#endif
#else
#include <errno.h>
#include <ctype.h>
#include <unistd.h>
#endif

bool extract_id(const std::string& name, const std::string& prefix, int& id);
void parse_frame_info(const std::string& frame_id, int& cam_id, int& exp_id, int& sample_id);

typedef std::vector<std::string> StringList;

char* _trim(char* s);
const char* int2str(int n, char* buf);
int str2int(const char* pc);
bool isnumber(const char* pc);
bool isnumber2(const char* pc);
bool isfloat(const char* pc);
StringList split(const char* s, char sep);
StringList split(const char* s, const char* seps);
const char* eatup(const char* pc, const char* delimiters);
const char* eatuptil(const char* pc, const char* delimiters);

const char* eatup(const char* pc, const char* delimiters, char* eated);
const char* eatuptil(const char* pc, const char* delimiters, char* eated);

std::string& trim(std::string& str,const char* delimiters);
std::string& ltrim(std::string& str,const char* delimiters);
std::string& rtrim(std::string& str,const char* delimiters);
std::string& lstrerase(std::string& str, const char* pattern);
std::string& rstrerase(std::string& str, const char* pattern);
std::string& sreplace(std::string& str, const char* olds, const char* news);

bool IsAbsolutePath(const char* filename);
std::string strerror_r2(int errnum);
const char* basename_r(const char* filename);
const char* dirname_r(const char* filename, char* buf);

//struct tm* MyLocalTime(time_t t, struct tm& stm);
//const char* LocalTimeString(time_t t, char* buf);
//char* CurrentTimeString(char* buf);

bool endWith(const std::string& str, const std::string& tail);

#ifdef _WINDOWS
typedef struct _stat FileStat;
#else
#include <unistd.h>
typedef struct stat FileStat;
#endif
int StatFile(const char* filename, FileStat& fileStat);

template <size_t size> class STRING
{
private:
    char m_data[size];
    
public:
    STRING()
    {
        memset(this, 0, sizeof(STRING));
    }

    STRING(const char* s)
    {
        if(s == NULL) memset(this, 0, sizeof(STRING));
        else strncpy(m_data, s, sizeof(m_data));
        m_data[sizeof(m_data)-1] = '\0';
    }

    STRING(const std::string& s)
    {
        strncpy(m_data, s.c_str(), sizeof(m_data));
        m_data[sizeof(m_data)-1] = '\0';
    }

    STRING(const STRING& o)
    {
        memcpy(this, &o, sizeof(STRING));
    }

    STRING& operator=(const STRING& o)
    {
        if(this == &o) return *this;
        memcpy(this, &o, sizeof(STRING));
        return *this;
    }

    STRING& operator=(const char* s)
    {
        if(s == NULL) memset(this, 0, sizeof(STRING));
        else strncpy(m_data, s, sizeof(m_data));
        m_data[sizeof(m_data)-1] = '\0';
        return *this;
    }

    operator const char*() const
    {
        return m_data;
    }

    operator char*()
    {
        return m_data;
    }

    const int Size() const
    {
        return size;
    }

    STRING& Upper()
    {
        for(int i=0; i<size; i++)
            m_data[i] = toupper(m_data[i]);
        return *this;
    }

    STRING& Lower()
    {
        for(int i=0; i<size; i++)
            m_data[i] = tolower(m_data[i]);
        return *this;
    }
};

template<size_t size> bool operator < (const STRING<size>& a, const STRING<size>& b)
{
    return (strcmp((const char*)a, (const char*)b) < 0);
}

template <size_t size>
    struct StringCompare : public std::binary_function<STRING<size>, STRING<size>, bool>
    {
    	bool operator()(const STRING<size>& _X, const STRING<size>& _Y) const
    	{
            return strcmp(_X, _Y) < 0;
        }
    };

template <size_t size>
    struct StringIgnoreCaseCompare : public std::binary_function<STRING<size>, STRING<size>, bool>
    {
    	bool operator()(const STRING<size>& _X, const STRING<size>& _Y) const
    	{
            return strcasecmp(_X, _Y) < 0;
        }
    };

#endif
