#if !defined(OPEN_LPREDITOR_LIB_H)
#define OPEN_LPREDITOR_LIB_H
#pragma once

#include <stddef.h>
#include "defect_data.h"


#ifndef LPREDITOR_EXPORTS
#ifdef __cplusplus
extern "C"
{
#endif
#ifdef _WINDOWS
	__declspec(dllimport)
#else //_WINDOWS
#endif //_WINDOWS

	size_t init_session(size_t len, const char* model_file);

#ifdef _WINDOWS
	__declspec(dllimport)
#else //_WINDOWS
#endif //_WINDOWS

	bool close_session(size_t id);

#ifdef _WINDOWS
	__declspec(dllimport)
#else //_WINDOWS
#endif //_WINDOWS
	bool detect(H9::DefectData& m_defectdata, size_t step, size_t id, size_t lpn_len, char* lpn);
	//bool detect(H9::DefectData& m_defectdata, size_t step, size_t id, size_t lpn_len, char* lpn,std::map<int,std::string>& defStr,std::set<int>& normalSet);

#ifdef __cplusplus
}
#endif
#else 
	//__declspec(dllexport)
#endif //LPREDITOR_EXPORTS


#endif // !defined(OPEN_LPREDITOR_LIB_H)
