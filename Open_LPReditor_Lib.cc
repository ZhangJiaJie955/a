#include <iterator>
#include <iostream>
#include <assert.h>
#include <cmath>
#include <exception>
#include <fstream>
#include <limits>
#include <numeric>
#include <string>
#include <sstream>

#include <onnxruntime_c_api.h>
#include <onnxruntime_cxx_api.h>
#include <cuda_provider_factory.h>

#include <opencv2/opencv.hpp>
#include <opencv2/dnn/dnn.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>


#include "yolov5_anpr_onnx_detector.h"
#include "Open_LPReditor_Lib.h"
#include "ONNX_detector.h"

#include "h9Log.h"


#include <thread>         // std::thread
#include <mutex>          // std::mutex, std::unique_lock, std::defer_lock

std::mutex mtx;           // mutex for critical section

//extern std::unique_ptr<Ort::Env> ort_env;
//step 1 declare a global instance of ONNX Runtime api
const OrtApi* g_ort = OrtGetApiBase()->GetApi(ORT_API_VERSION);
std::list<Ort::Env*> envs;
std::list<Ort::SessionOptions*> lsessionOptions;
std::list<Yolov5_anpr_onxx_detector*> detectors;
//detectors_ids : a list that contains the ids of all the detectors that are currently allocated by the library
std::list<size_t> detectors_ids;
//*****************************************************************************
// helper function to check for status
void CheckStatus(OrtStatus* status)
{
	if (status != NULL) {
		const char* msg = g_ort->GetErrorMessage(status);
		fprintf(stderr, "%s\n", msg);
		g_ort->ReleaseStatus(status);
		exit(1);
	}
}

std::list<Yolov5_anpr_onxx_detector*>::const_iterator get_detector(size_t id, const std::list<Yolov5_anpr_onxx_detector*>& detectors,
	const std::list<size_t>& detectors_ids
	//, const Yolov5_anpr_onxx_detector * * ref
) {
	assert(detectors_ids.size() == detectors.size());
	std::list<Yolov5_anpr_onxx_detector*>::const_iterator it(detectors.begin());
	std::list<size_t>::const_iterator it_id(detectors_ids.begin());
	while (it != detectors.end() && it_id != detectors_ids.end()) {
		if (*it_id == id) {
			//ref= *(*it);
			//(*it)->dump();
			return it;
		}
		else {
			it_id++;
			it++;
		}
	}
	return detectors.end();
}

size_t get_new_id(const std::list<size_t>& detectors_ids) {
	if (detectors_ids.size()) {
		auto result = std::minmax_element(detectors_ids.begin(), detectors_ids.end());
		return *result.second + 1;
	}
	else return 1;
}

bool close_session(size_t id, std::list<Ort::Env*>& _envs, std::list<Ort::SessionOptions*>& _lsessionOptions, std::list<Yolov5_anpr_onxx_detector*>& _detectors,
	std::list<size_t>& _detectors_ids) {
	assert(_detectors_ids.size() == _detectors.size()
		&& _detectors_ids.size() == _envs.size()
		&& _detectors_ids.size() == _lsessionOptions.size());
	std::list<Yolov5_anpr_onxx_detector*>::iterator it(_detectors.begin());
	std::list<size_t>::iterator it_id(_detectors_ids.begin());
	std::list<Ort::SessionOptions*>::iterator it_sessionOptions(_lsessionOptions.begin());
	std::list<Ort::Env*>::iterator it_envs(_envs.begin());
	while (it != _detectors.end() && it_id != _detectors_ids.end()
		&& it_envs != _envs.end() && it_sessionOptions != _lsessionOptions.end()
		) {
		if (*it_id == id) {
			std::cout << "delete env and options and net,id:%d"<<id << std::endl;
			if (*it_envs != nullptr) delete* it_envs;
			if (*it_sessionOptions != nullptr) delete* it_sessionOptions;
			if (*it != nullptr) delete* it;
			it_envs = _envs.erase(it_envs);
			it_sessionOptions = _lsessionOptions.erase(it_sessionOptions);
			it = _detectors.erase(it);
			it_id = _detectors_ids.erase(it_id);
			return true;
		}
		else {
			it_sessionOptions++;
			it_envs++;
			it_id++;
			it++;
		}
	}
	return false;
}

extern "C"
#ifdef _WINDOWS
__declspec(dllexport)
#endif //_WINDOWS
size_t init_session(size_t len, const char* model_file)
{
	assert(detectors_ids.size() == detectors.size());
	const std::string model_filename(model_file, len);
	std::cout << "modelfile is sucesssful " << model_filename << std::endl;
	//step 2 declare an onnx runtime environment
	std::string instanceName{ "image-classification-inference" };
	//Ort::Env* penv = new Ort::Env(OrtLoggingLevel::ORT_LOGGING_LEVEL_WARNING, instanceName.c_str());
	Ort::Env* penv = new Ort::Env(OrtLoggingLevel::ORT_LOGGING_LEVEL_VERBOSE, instanceName.c_str());//edit by lijun
	if (penv != nullptr) {
		//step 3 declare options for the runtime environment
		Ort::SessionOptions* psessionOptions = new Ort::SessionOptions();
		if (psessionOptions != nullptr) {
			psessionOptions->SetIntraOpNumThreads(1);
			psessionOptions->SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_EXTENDED);
			//psessionOptions->SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);//edit by lijun

#ifdef LPR_EDITOR_USE_CUDA
			// Optionally add more execution providers via session_options
			// E.g. for CUDA include cuda_provider_factory.h and uncomment the following line:
			// nullptr for Status* indicates success
			/*
			OrtCUDAProviderOptions cuda_options{
				0,
				//OrtCudnnConvAlgoSearch::EXHAUSTIVE ,
				OrtCudnnConvAlgoSearchExhaustive,
				std::numeric_limits<size_t>::max(),
				0,
				true
				};

			psessionOptions->AppendExecutionProvider_CUDA(cuda_options);
			*/
			OrtStatus* status = OrtSessionOptionsAppendExecutionProvider_CUDA(*psessionOptions, 0);
			if (status == nullptr) {
			
#endif //LPR_EDITOR_USE_CUDA
				Yolov5_anpr_onxx_detector* onnx_net = nullptr;
#ifdef _WIN32
				//step 4 declare an onnx session (ie model), by giving references to the runtime environment, session options and file path to the model
				std::wstring widestr = std::wstring(model_filename.begin(), model_filename.end());
				onnx_net = new Yolov5_anpr_onxx_detector(*penv, widestr.c_str(), *psessionOptions);
				
#else
				onnx_net = new Yolov5_anpr_onxx_detector(*penv, model_filename.c_str(), *psessionOptions);
#endif
				if (onnx_net != nullptr && penv != nullptr && psessionOptions != nullptr) {
					std::unique_lock<std::mutex> lck(mtx, std::defer_lock);
					lck.lock();
					envs.push_back(penv);
					lsessionOptions.push_back(psessionOptions);
					detectors.push_back(onnx_net);
					size_t id = get_new_id(detectors_ids);
					detectors_ids.push_back(id);
					std::cout << "YOLO onnx new create net by:" << model_filename <<"--id:"<<id<< std::endl;
					lck.unlock();
					return id;
				}
				else {
					std::cout << "error while creating onnxruntime session with file : " << model_filename.c_str() << std::endl;
					return 0;
				}

#ifdef LPR_EDITOR_USE_CUDA
	 }
			else {
				CheckStatus(status);
					std::cout << "cuda error "<< std::endl;
					return 0;
				}
#endif //LPR_EDITOR_USE_CUDA
		
		}
		else {
			std::cout << "error while creating SessionOptions" << std::endl;
			return 0;
		}
	}
	else {
		std::cout << "error while creating session environment (Ort::Env)" << std::endl;
		return 0;
	}
}

extern "C"
#ifdef _WINDOWS
__declspec(dllexport)
#endif //_WINDOWS
bool detect(H9::DefectData& m_defectdata, size_t step, size_t id, size_t lpn_len, char* lpn)
//bool detect(H9::DefectData& m_defectdata, size_t step, size_t id, size_t lpn_len, char* lpn,std::map<int, std::string>& defStr, std::set<int>& normalSet)
{

	const int width = m_defectdata.m_srcImg->m_data.cols;
	const int height = m_defectdata.m_srcImg->m_data.rows;
	const int pixOpt = m_defectdata.m_srcImg->m_data.channels();
	void* pbData = m_defectdata.m_srcImg->m_data.data;

	const int def_width = m_defectdata.m_defImg->m_data.cols;
	const int def_height = m_defectdata.m_defImg->m_data.rows;
	const int def_pixOpt = m_defectdata.m_defImg->m_data.channels();
	void* def_pbData = m_defectdata.m_defImg->m_data.data;

	if ((pixOpt != 1) && (pixOpt != 3) && (pixOpt != 4) || height <= 0 || width <= 0 || pbData == nullptr) return false;
	else {
		cv::Mat destMat;
		if (pixOpt == 1)
		{
			destMat = cv::Mat(height, width, CV_8UC1, pbData, step);
		}
		if (pixOpt == 3)
		{
			destMat = cv::Mat(height, width, CV_8UC3, pbData, step);
		}
		if (pixOpt == 4)
		{
			destMat = cv::Mat(height, width, CV_8UC4, pbData, step);
		}
		cv::Mat defMat;
		if (def_pixOpt == 1)
		{
			defMat = cv::Mat(def_height, def_width, CV_8UC1, def_pbData, step);
		}
		if (def_pixOpt == 3)
		{
			defMat = cv::Mat(def_height, def_width, CV_8UC3, def_pbData, step);
		}
		if (def_pixOpt == 4)
		{
			defMat = cv::Mat(def_height, def_width, CV_8UC4, def_pbData, step);
		}

		std::list<Yolov5_anpr_onxx_detector*>::const_iterator it = get_detector(id, detectors, detectors_ids);
		std::string lpn_str;
		std::unique_lock<std::mutex> lck(mtx, std::defer_lock);
		lck.lock();
		//for normal plates
		std::list<cv::Rect> boxes;
		std::list<float> confidences;
		std::list<int> classes;

		//H9_INFO("begin evaluate_lpn_with_yolov5,id:%d",id);
		std::cout << "lpn_detection" << std::endl;

		// 真正进行检测的函数
		(*it)->evaluate_lpn_with_lpn_detection(destMat, defMat, lpn_str, confidences, classes, boxes);
		//H9_INFO("finish evaluate_lpn_with_yolov5,id:%d", id);
		std::ostringstream oss;

		// 2023/9/15/19:07 zjj
		//auto boxes_iter = begin(boxes);
		//auto confidences_iter = begin(confidences);
		//auto classes_iter = begin(classes);
		std::list<cv::Rect>::iterator boxes_iter = boxes.begin();
		std::list<float>::iterator confidences_iter = confidences.begin();
		std::list<int>::iterator classes_iter = classes.begin();
		// 2023/9/15/19:07 zjj
		int boxes_size = boxes.size();
		// 改boxes.size() 成了boxes_size
		std::cout << "detected object num is " << boxes_size << std::endl;

		if(boxes_size > 0)
		{
			//oss << ":" << boxes.size()<<",";
			//int totalNum = defStr.size() + normalSet.size();
			for(int i = 0; i < boxes_size; i++)
			{
				
				//wjl
				int def_class = *classes_iter;
				float quexian_confidence = *confidences_iter;
				cv::Rect quexian_box = *boxes_iter;

				std::cout << "def_class is " << def_class;
				std::cout << " Conf = " << quexian_confidence 
						  << ",\tx = " << quexian_box.x << ",\ty= " << quexian_box.y 
						  << ",\twidth = " << quexian_box.width << ",\theight = " << quexian_box.height
						  << ";" << std::endl;

				oss << def_class << " " << quexian_confidence 
					<< " " << quexian_box.x << " " << quexian_box.y 
					<< " " << quexian_box.width << " " << quexian_box.height << " #";

				//if ( totalNum> 0 && normalSet.find(def_class) == normalSet.end() && def_class < defStr.size()) {
					//oss << "C" << def_class << ",";
				//	oss << defStr[def_class] << ",";
					//oss << "Confidence = " << quexian_confidence << ", [" << quexian_box.x << ", " << quexian_box.y << "] from [" << quexian_box.width << ", " << quexian_box.height << "]" << std::endl;
					//std::cout<< "Conf=" << quexian_confidence << ", [" << quexian_box.x << ", " << quexian_box.y << "] from [" << quexian_box.width << ", " << quexian_box.height << "] \n";
					//oss << "Conf = " << quexian_confidence << ", [" << quexian_box.x << ", " << quexian_box.y << "] from [" << quexian_box.width << ", " << quexian_box.height << "] \n" ;
				//}
				//else if(totalNum == 0)
				
				//	oss << "C" << def_class << ",";
				classes_iter++;	
				confidences_iter++;
				boxes_iter++;
			}
			oss<<"@";
		}
		else
		{
			oss<<"Nothing";
		}
		std::string deftype(oss.str());
		m_defectdata.setDefectType(deftype);
		lck.unlock();
		std::string::const_iterator it_lpn(lpn_str.begin());
		int i = 0;
		while (it_lpn != lpn_str.end() && i < lpn_len) {
			lpn[i] = *it_lpn;
			i++; it_lpn++;
		}
		return (boxes.size() > 0);
	}
}

extern "C"
#ifdef _WINDOWS
__declspec(dllexport)
#endif //_WINDOWS
bool close_session(size_t id)
{
	assert(detectors_ids.size() == detectors.size());
	std::unique_lock<std::mutex> lck(mtx, std::defer_lock);
	lck.lock();
	bool session_closed = close_session(id, envs, lsessionOptions, detectors, detectors_ids);
	lck.unlock();
	return session_closed;
}
