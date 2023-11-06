#include "InspectProcessor.h"
#include <iostream>
#include <numeric>
#include <xmmintrin.h>
#include <omp.h>

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <boost/filesystem.hpp>
#include "h9Log.h"
#include "objectwrapper.h"
#include "image.h"
#include "inspect_util.h"
#include <fstream>
#include "h9Timer.h"
#include "ColorPredictor.h"
#include "str_util.h"

using namespace std;

namespace H9 {

#define GET_ENGINE(T, x) dynamic_cast<T*>(GetEngine((x)))
    
bool InspectProcessor::findKey(YAML::Node const &node, string const &key) const
{
    return node[key] || (m_params && m_params[key]);
}

template<typename T>
T InspectProcessor::parse(YAML::Node const &node, string const &key, T const &defval) const
{
    if (node[key]) {
        if (node[key].IsScalar())
            return node[key].as<T>();
        else
            return defval;
    }

    if (m_params && m_params[key]) {
        if (m_params[key].IsScalar())
            return m_params[key].as<T>();
        else
            return defval;
    }
        
    return defval;
}

template<typename T> vector<T> InspectProcessor::parseList(YAML::Node const &node, string const &key, vector<T> const &defval) const
{
    vector<T> ret;
    if (node[key])
        parseOneList(node[key], ret);
    else if (m_params && m_params[key])
        parseOneList(m_params[key], ret);
    else
        return defval;

    return ret;
}

template<> vector<vector<double> > InspectProcessor::parseList(YAML::Node const &node, string const &key, vector<vector<double> > const &defval) const
{
    YAML::Node val;
    if (node[key])
        val = node[key];
    else if (m_params && m_params[key])
        val = m_params[key];
    else
        return defval;

    if (!val.IsSequence())
        return defval;

    vector<vector<double> > ret(val.size(), vector<double>());
    for (size_t i = 0; i < val.size(); ++i)
        parseOneList(val[i], ret[i]);

    return ret;
}

bool InspectProcessor::Initialize(map<string, string> const &options)
{
    cout << "1" << endl;
    H9_INFO("InspectProcessor initialization1 start");
    string confname("conf.yaml");

    cout << "first" << endl;
    map<string, string>::const_iterator oit = options.find("conf");
    cout << "second" << endl;

    system("Pause");

    if (options.end() != oit)
        confname = oit->second;
    cout << "confname:" << confname << endl;
    H9_INFO("Load YAML config from file %s", confname.c_str());
    try {
        m_config = YAML::LoadFile(confname);
        cout << "conf yaml load " << endl;
    } catch (YAML::ParserException &e) {
	    H9_ERROR("Load config yaml file %s failed.\nexception: %s", confname.c_str(), e.what());
        return false;
    } catch(...) {
        cout << "catch 2" << endl;
	    H9_ERROR("Load config yaml file %s failed.", confname.c_str());
        system("Pause");
	    return false;
    }

    if (!m_config["init"]) {
        H9_ERROR("YAML config does not have init node");
        return false;
    }
    YAML::Node const &initnode = m_config["init"];

    if (initnode["mode"]) {
        string modestr = initnode["mode"].as<string>();
        H9_INFO("Mode specified in YAML: %s", modestr.c_str());
        if ("calibrate" == modestr || "cal" == modestr)
            m_mode = CAL;
        else if ("inspect" == modestr)
            m_mode = INSPECT;
    }
    oit = options.find("mode");
    if (options.end() != oit) {
        InspectMode mode = UNKNOWN;
        if ("calibrate" == oit->second || "cal" == oit->second)
            mode = CAL;
        else if ("inspect" == oit->second)
            mode = INSPECT;

        if (UNKNOWN != mode) {
            if (UNKNOWN != m_mode)
                H9_INFO("Mode is overridden by command line setting: %s", oit->second.c_str());
            m_mode = mode;
        }
    }

    if (UNKNOWN == m_mode) {
        H9_ERROR("Mode is not specified");
        return false;
    }

    if (INSPECT == m_mode) {
        m_refimg = parse(initnode, "reference", string(""));
        if (m_refimg.empty()) {
            H9_WARN("Missing reference img for inspection; assume not used");
        }
    }

    if (initnode["test_mode"] && initnode["test_mode"].as<int>() > 0)
        m_testmode = true;
    if (initnode["missing_return_ng"] && initnode["missing_return_ng"].as<int>() > 0)
        m_missing_return_ng = true;

    if (initnode["parameter"]) {
        string paramname = initnode["parameter"].as<string>();
        if (!LoadParamYaml(paramname))
            return false;
    } else if (m_config["parameter"])
        m_params = YAML::Clone(m_config["parameter"]);

    m_modelpath = parse(initnode, "modelPath", string(""));
    if (m_modelpath.empty()) {
		m_modelpath = "d:\\models\\best_yaodai_0318.onnx";
        H9_WARN("Missing modelpath for inspection; assume not used");
    }
    m_omcpath = parse(initnode, "halconOmcPath", string(""));
    if (m_omcpath.empty()) {
        H9_WARN("Missing modelpath for inspection; assume not used");
    }
    m_modeltype = parse(initnode, "modelType", string(""));
    if (m_modeltype.empty()) {
        H9_WARN("Missing modeltype for inspection; assume not used");
        m_modeltype = "ONNX";
    }

    modelDetPath = parse(initnode, "modelDetPath", string(""));
    if (modelDetPath.empty()) {
        H9_WARN("Missing modelDetPath for inspection; assume not used");
    }

    modelClsPath = parse(initnode, "modelClsPath", string(""));
    if (modelClsPath.empty()) {
        H9_WARN("Missing modelClsPath for inspection; assume not used");
    }

    modelRecPath = parse(initnode, "modelRecPath", string(""));
    if (modelRecPath.empty()) {
        H9_WARN("Missing modelRecPath for inspection; assume not used");
    }

    keysPath = parse(initnode, "modelkeysPath", string(""));
    if (keysPath.empty()) {
        H9_WARN("Missing keysPath for inspection; assume not used");
    }

    if (initnode["detectors"]) {
        string detyaml = initnode["detectors"].as<string>();
        if (!LoadDetectorYaml(detyaml))
            return false;
    } else if (m_config["detectors"])
        m_detectors = YAML::Clone(m_config["detectors"]);

    m_resultfile = "result.yaml";
    if (initnode["result_yaml"] && initnode["result_yaml"].IsScalar())
        m_resultfile = initnode["result_yaml"].as<string>();
    H9_INFO("Save result to yaml file: %s", m_resultfile.c_str());

    if (initnode["runtime_limit"] && initnode["runtime_limit"].as<float>() > 0)
        m_rt_limit = initnode["runtime_limit"].as<float>();
    else
        m_rt_limit = -1;

    if (initnode["total_runtime_limit"] && initnode["total_runtime_limit"].as<float>() > 0)
	    m_total_rt_limit = initnode["total_runtime_limit"].as<float>();
    else
	    m_total_rt_limit = -1;      
	
    if (m_params["num_pills"]) {
        m_npills = m_params["num_pills"].as<int>();
        H9_INFO("The amount of pills/capsules : %d", m_npills);
    }
    // if (m_params["num_rows"]) {
    //     m_nrows = m_params["num_rows"].as<int>();
    //     H9_INFO("The rows of pills/capsules : %d", m_nrows);
    // }    
    // if (m_params["num_cols"]) {
    //     m_ncols = m_params["num_cols"].as<int>();
    //     H9_INFO("The cols of pills/capsules : %d", m_ncols);
    // }
    //cout << "ppocr model cleandata" << endl;
    cleandata();
    //cout << "ppocr model cleandata done" << endl;
    InitEngineSet();
    //cout << "ppocr model initenginset" << endl;

#ifdef	USE_ONNXRuntime
    //init session
    size_t len = m_modelpath.size();
    if (!m_modelpath.empty() && boost::filesystem::exists(boost::filesystem::path(m_modelpath.c_str())) && m_modeltype == "ONNX")
        session_id = init_session(len, m_modelpath.c_str());
    //cout << "init model start" << endl;
    if (!modelDetPath.empty() && !modelClsPath.empty() && !modelRecPath.empty() && !keysPath.empty()) {
        std::cout << "Init ocr lite inside inspectProcessor!" << endl;
        //int numThread = 4;
        //cout << "omp_thread" << endl;
        //omp_set_num_threads(numThread);
        //cout << "omp_thread end" << endl;
        ////OcrLite ocrLite;
        //ocrLite.setNumThread(numThread);
        //cout << "set_thread end" << endl;
        //ocrLite.initLogger(
        //        false,//isOutputConsole
        //        false,//isOutputPartImg
        //        false);//isOutputResultImg
        H9_LOG("%s: Before ocrLite.initModels.", __func__);
        //cout << "initLogger end" << endl;
        
    }
#endif
    //cout << "ppocr model init done" << endl;
	H9_INFO("InspectProcessor initialization done");

    return true;
}

bool InspectProcessor::Initialize(const char* flow, const char* design_template, const char* detectors, const char* options, InspectMode m)
{
	H9_INFO("InspectProcessor initialization2 start");
    cout << "init start" << endl;
	if (flow == NULL || strlen(flow) == 0) {
		H9_ERROR("flow yaml is not configured");
		return false;
	}

	H9_INFO("Load flow YAML config from file %s", flow);

    cout <<"flow is: " << flow << endl;
	try {
		m_config = YAML::LoadFile(flow);
	} catch (YAML::ParserException &e) {
		H9_ERROR("Load conf yaml file %s failed.\nexception: %s", flow, e.what());
		return false;
	} catch(...) {
		H9_ERROR("Load conf yaml file %s failed.", flow);
		return false;
	}

	m_mode = m; // can be overriden by the one in yaml
	if (!m_config["init"]) {
		H9_ERROR("YAML config does not have init node");
		return false;
	}

	YAML::Node const &initnode = m_config["init"];
	if (initnode["mode"]) {
		string modestr = initnode["mode"].as<string>();
		H9_INFO("Mode specified in YAML: %s", modestr.c_str());
		if ("calibrate" == modestr || "cal" == modestr)
			m_mode = CAL;
		else if ("inspect" == modestr)
			m_mode = INSPECT;
	}
#ifdef	USE_ONNXRuntime

    cout << "USE_Runtime" << endl;
    m_normalIdx.clear();
    m_defectLabel.clear();
    m_totalLabelNum = 0;

#endif

	if (options != NULL && strlen(options) != 0) {
		try {
			YAML::Node options_conf = YAML::LoadFile(options);
			if (options_conf["mode"].IsDefined()) {
				std::string mode_str = options_conf["mode"].as<std::string>();
				InspectMode mode = UNKNOWN;
				if ("calibrate" == mode_str || "cal" == mode_str)
					mode = CAL;
				else if ("inspect" == mode_str)
					mode = INSPECT;
				
				
				if (UNKNOWN != mode) {
					if (UNKNOWN != m_mode)
						H9_INFO("Mode is overridden by options: %s", options);
					m_mode = mode;
				}
			}
		} catch (YAML::ParserException &e) {
			H9_ERROR("Load options yaml file %s failed.\nexception: %s", options, e.what());
			//return false;
		} catch(...) {
			H9_ERROR("Load options yaml file %s failed.", options);
			return false;
		}
	}
	
	
	if (UNKNOWN == m_mode) {
		H9_ERROR("Mode is not specified");
		return false;
	}
		
	if (INSPECT == m_mode) {
		m_refimg = parse(initnode, "reference", string(""));
		if (m_refimg.empty()) {
			H9_WARN("Missing reference img for inspection; assume not used");
		}
	}
	
	if (initnode["test_mode"] && initnode["test_mode"].as<int>() > 0)
		m_testmode = true;
    if (initnode["missing_return_ng"] && initnode["missing_return_ng"].as<int>() > 0)
        m_missing_return_ng = true;
	if (design_template != NULL && strlen(design_template) != 0) {
		if (!LoadParamYaml(std::string(design_template)))
			return false;
	} else if (m_config["parameter"])
		m_params = YAML::Clone(m_config["parameter"]);

    m_modelpath = parse(initnode, "modelPath", string(""));
    cout << " model path: " << m_modelpath << endl;
    if (m_modelpath.empty()) {
        //m_modelpath = "d:\\models\\best_yaodai_0318.onnx";
        H9_WARN("Missing modelpath for inspection; assume not used");
    }
    // << "m_omcpath start" << endl;
    m_omcpath = parse(initnode, "halconOmcPath", string(""));
    if (m_omcpath.empty()) {
        H9_WARN("Missing modelpath for inspection; assume not used");
    }
    //cout << "m_omcpath end" << endl;
    m_modeltype = parse(initnode, "modelType", string(""));
    if (m_modeltype.empty()) {
        H9_WARN("Missing modeltype for inspection; assume not used");
        m_modeltype = "ONNX";
    }
    modelDetPath = parse(initnode, "modelDetPath", string(""));
    if (modelDetPath.empty()) {
        H9_WARN("Missing modelDetPath for inspection; assume not used");
    }
    modelClsPath = parse(initnode, "modelClsPath", string(""));
    if (modelClsPath.empty()) {
        H9_WARN("Missing modelClsPath for inspection; assume not used");
    }

    modelRecPath = parse(initnode, "modelRecPath", string(""));
    if (modelRecPath.empty()) {
        H9_WARN("Missing modelRecPath for inspection; assume not used");
    }

    keysPath = parse(initnode, "modelkeysPath", string(""));
    if (keysPath.empty()) {
        H9_WARN("Missing keysPath for inspection; assume not used");
    }  
	
	if (detectors != NULL && strlen(detectors) != 0) {
		if (!LoadDetectorYaml(std::string(detectors)))
			return false;
	} else if (m_config["detectors"])
		m_detectors = YAML::Clone(m_config["detectors"]);

	m_resultfile = "result.yaml";
	if (initnode["result_yaml"] && initnode["result_yaml"].IsScalar())
		m_resultfile = initnode["result_yaml"].as<string>();
        
	if (initnode["runtime_limit"] && initnode["runtime_limit"].as<float>() > 0)
            m_rt_limit = initnode["runtime_limit"].as<float>();
	else
	    m_rt_limit = -1;

	if (initnode["total_runtime_limit"] && initnode["total_runtime_limit"].as<float>() > 0)
	    m_total_rt_limit = initnode["total_runtime_limit"].as<float>();
	else
	    m_total_rt_limit = -1;
	
    if (m_params["num_pills"]) {
        m_npills = m_params["num_pills"].as<int>();
        H9_INFO("The amount of pills/capsules : %d", m_npills);
    }
    /*
    if (m_params["ocr"])
    {
        YAML::Node const& ocrnode = m_params["ocr"];

        if (ocrnode["ocr_fuzzymatch"])
        {
            int sw = ocrnode["ocr_fuzzymatch"].as<int>();
            m_bOcrFuzzyMatch = sw != 0 ? true : false;
        }
        else m_bOcrFuzzyMatch = false;

        if (ocrnode["ocr_template"])
        {
            string str = ocrnode["ocr_template"].as<string>();
            m_strOcrTemplate = str;

        }
        else m_strOcrTemplate = "";
    }*/
    // if (m_params["num_rows"]) {
    //     m_nrows = m_params["num_rows"].as<int>();
    //     H9_INFO("The rows of pills/capsules : %d", m_nrows);
    // }    
    // if (m_params["num_cols"]) {
    //     m_ncols = m_params["num_cols"].as<int>();
    //     H9_INFO("The cols of pills/capsules : %d", m_ncols);
    // }
    cout << "cleandata start" << endl;
    cleandata();
    cout << "initengineset start" << endl;
    //cout << "initengineset start" << endl;
    InitEngineSet();
    cout << "initengineset end" << endl;
#ifdef	USE_ONNXRuntime
    //init session
    cout << "model path: " << m_modelpath << endl; 
    cout << "model type: " << m_modeltype << endl;
    size_t len = m_modelpath.size();
    if (!m_modelpath.empty() && boost::filesystem::exists(boost::filesystem::path(m_modelpath.c_str())) && m_modeltype == "ONNX") {
        cout << "init model start" << endl;
        session_id = init_session(len, m_modelpath.c_str());
        
    }
        
    //cout << "init model start" << endl;
    if (!modelDetPath.empty() && !modelClsPath.empty() && !modelRecPath.empty() && !keysPath.empty()) {
        std::cout << "Init ocr lite inside inspectProcessor!" << endl;
        //int numThread = 4;
        //cout << "omp_thread" << endl;
        //omp_set_num_threads(numThread);
        //cout << "omp_thread end" << endl;
        ////OcrLite ocrLite;
        //ocrLite.setNumThread(numThread);
        //cout << "set_thread end" << endl;
        //ocrLite.initLogger(
        //        false,//isOutputConsole
        //        false,//isOutputPartImg
        //        false);//isOutputResultImg
        H9_LOG("%s: Before ocrLite.initModels.", __func__);
        //cout << "initLogger end" << endl;
        
    }
#endif
    //cout << "init model end" << endl;
	H9_INFO("InspectProcessor initialization done");
	return true;
}

	
bool InspectProcessor::LoadParamYaml(string const &paramname)
{ 
    try {
        // 只能用绝对路径？？？？？ 看起来是这样的
        m_params = YAML::LoadFile(paramname);
        cout << "params.yaml loaded successfully" << endl;
    } catch (YAML::ParserException &e) {
	    H9_ERROR("Load parameter yaml file %s failed.\nexception: %s", paramname.c_str(), e.what());
        return false;
    } catch(...) {
        cout << "there" << endl;
	    H9_ERROR("Load parameter yaml file %s failed.", paramname.c_str());
	    return false;
    }

    return true;
}

bool InspectProcessor::LoadDetectorYaml(string const &detyaml)
{
    try {
        m_detectors = YAML::LoadFile(detyaml);
    } catch (YAML::ParserException &e) {
	    H9_ERROR("Load detector yaml file %s failed. \nexception: %s", detyaml.c_str(), e.what());
        return false;
    } catch(...) {
	    H9_ERROR("Load detector yaml file %s failed.", detyaml.c_str());
	    return false;
    }
    return true;
}

bool InspectProcessor::Execute(string const &imgname)
{

    m_foundDefect = false;
    m_flipped = false;
    m_defect_images.clear();
    m_imgname = imgname;
    H9_INFO("InspectProcessor execute start for image %s", imgname.c_str());
    size_t pos = imgname.rfind('.');
    m_basename = m_imgname.substr(0, pos);
    pos = imgname.rfind('/');
    if (pos == std::string::npos)
	    pos = imgname.rfind('\\');

    if (string::npos != pos)
        m_basename = m_basename.substr(pos + 1);
 	    
    H9_INFO("image has base name %s", m_basename.c_str());


    return executeFlow();
    
}

bool InspectProcessor::Execute(const ImageObject& in)
{
    // 限制检测输入图片的尺寸
    if (in.m_data.rows != 1440 || in.m_data.cols != 2560)
    {
        cout << "\nimage error !\n" << endl;;
        return false;
    }


    m_imgname = ""; // image will be read from in
    m_basename = "inspection";
    m_result = YAML::Node();

    ObjectWrapper* wrapper = m_container.addWrapper("FROM_UPSTREAM__");
    wrapper->getImageObject() = in; 

    return executeFlow();
}

bool InspectProcessor::executeFlow()
{
    // conf_inspect_1.yaml 中指定flow
    if (!m_config["flow"]) {
        H9_ERROR("ERROR: flow node is missing");
        return false;
    }

    const YAML::Node& flow_node = m_config["flow"];

    if (!flow_node["commands"] || !flow_node["commands"].IsSequence()) {
        H9_ERROR("ERROR: flow node has wrong format");
        return false;
    }
    
    if(m_rt_limit>0 && m_total_rt_limit <= 0)
	    m_rt_limit_timer.reset();

    if (m_rt_limit > 0 || m_total_rt_limit > 0)                                                                                                          
        m_current_process_start = m_rt_limit_timer.getDuration();
    
    YAML::Node const &commands = flow_node["commands"];
    for (size_t i = 0; i < commands.size(); ++i) {
        if (inspectionDone()) {
            H9_INFO("Inspection is done; return");
            return true;
        }
        YAML::Node const &command = commands[i];
        string key = command.begin()->first.as<string>();
        if ("break" == key) {
            int break_val = 0;
            YAML::Node const &bp_node = command[key];
            if (bp_node.IsMap()) {
                if (bp_node["tag"] && bp_node["tag"].IsScalar()) {
                    string bp_key = bp_node["tag"].as<string>();
                    break_val = parse(command[key], bp_key, break_val);
                }
            } else if (bp_node.IsScalar())
                break_val = bp_node.as<int>();
            if (0 == break_val) {
                continue;
            } else {
                H9_INFO("break is encountered; flow is ended");
                break;
            }
        }
        // 这里开始确定用啥command
        if (!ExecuteCommand(key, command[key])) {
            H9_ERROR("Failed to execute command %s", key.c_str());
            return false;
        }
	/*
        if((i+1 == commands.size()) && (m_rt_limit > 0) && (m_rt_limit_timer.getDuration() > m_rt_limit)) {
            H9_LOG("runtime limit reached, flow is ended, duration=%f limit=%f", m_rt_limit_timer.getDuration(), m_rt_limit);
            break;
	    }*/


        if (i+1 < commands.size()) {
            float duration = m_rt_limit_timer.getDuration();
            //H9_LOG("runtime so far %d - %f", i, duration);
            if ((m_total_rt_limit > 0 && duration > m_total_rt_limit)) {
            H9_LOG("runtime limit reached for this sample, flow is ended, duration=%f limit=%f",
                duration, m_total_rt_limit);
            break;
            }
            if (m_rt_limit > 0 && duration - m_current_process_start > m_rt_limit) {
            H9_LOG("runtime limit reached for this process, flow is ended, duration=%f limit=%f",
                duration - m_current_process_start, m_rt_limit);
            break;
            }
        }
    }
    H9_INFO("InspectProcessor execute done");
    return true;
}


bool InspectProcessor::Finalize() const
{
    H9_INFO("InspectProcessor finalize start");
    // if (CAL == m_mode) {
    //     m_result["num_pills"] = m_npills;
    //     m_result["num_rows"] = m_nrows;
    //     m_result["num_cols"] = m_ncols;
    // }
    H9_INFO("save inspection result to %s", m_resultfile.c_str());
    ofstream ofs(m_resultfile.c_str());
    ofs << m_result;
    ofs.close();

    H9_INFO("InspectProcessor finalize done");
    return true;
} 

bool InspectProcessor::OutputResult(std::string& result_str) const
{
	ostringstream out_s;
	out_s << m_result; 
	result_str = out_s.str();

	if (result_str == "~") result_str = "";
	
	if (m_result["inspection"].IsDefined()) {
		if (m_result["inspection"].IsScalar()) 
			H9_TRACE("res:%s", m_result["inspection"].as<std::string>().c_str());
		if (m_result["inspection"].IsSequence())
			H9_TRACE("Inspection result contains %d samples", m_result["inspection"].size());
	}
	return true;
}  
/*
bool InspectProcessor::GetDefectImage(std::vector<std::pair<std::string, ImageObject> >& out, std::string& tag) const
{
	tag = "";

	if (m_mode == CAL) {
		std::vector<std::string> images;

		if (m_params && m_params["param_groups"] && m_params["param_groups"].IsSequence()) {

			for (size_t i = 0; i < m_params["param_groups"].size(); ++i)
				if (m_params["param_groups"][i].IsMap())
					images.push_back(m_params["param_groups"][i].begin()->first.as<std::string>());
			
		} else {
			//images = std::vector<std::string>{"contour_img", "image_aligned", "inrange", "descript", "descript_text"};
			images = std::vector<std::string>{"contour_img", "image_aligned", "inrange_a","inrange","inrange_close",
                                        "descript", "blister_dilate_mask","xor_mask","aligned_text","texture_result"};
            //, "texture_result"};
			// all names are hardcoded for now - have to match with the conf.yaml
			// maybe later we can change it to more generic
		}
		
		int requested = images.size() - 1;
		YAML::Node dummy;
		for (int im = images.size() - 1; im >= 0; --im)
			if (parse(dummy, images[im], 0) > 0) {
				requested = im;
				break;
			}
		H9_TRACE("Requested image is %d = %s", requested, images[requested].c_str());
		
		for (int i = 0; i <= requested; ++i) {
			H9_TRACE("Checking calibrate image %s", images[i].c_str());
			if (m_container.hasWrapper(images[i])) {
				ObjectWrapper * wrapper = m_container.getWrapper(images[i]);
				ImageObject out_i = wrapper->getImageObject();

				if (out_i.m_data.cols == 0 || out_i.m_data.rows == 0) {
					H9_TRACE("Failed to grab CAL image %s", images[i].c_str());
					continue;
				}
				
				if (out_i.colorType() == ImageObject::GRAY) {
					cv::cvtColor(out_i.m_data, out_i.m_data, cv::COLOR_GRAY2BGR);     
					for (int j = i - 1; j >= 0; --j) {

						if (m_container.hasWrapper(images[j])) {
							wrapper = m_container.getWrapper(images[j]);
							ImageObject p = wrapper->getImageObject();
							if (p.m_data.cols > 0 && p.m_data.rows > 0 &&
							    p.colorType() == ImageObject::BGR) {
								cv::addWeighted(p.m_data, 0.7, out_i.m_data, 0.3, 0,
										out_i.m_data);
								break;
							}
						}
					}
					out_i.setColorType(ImageObject::BGR);
				}


				
				if (images[i] == "descript_text" && m_container.hasWrapper("texture_result")) {
					wrapper = m_container.getWrapper("texture_result");
					ImageObject p = wrapper->getImageObject();
					if (p.m_data.cols > 0 && p.m_data.rows > 0) {
						cv::cvtColor(p.m_data, p.m_data, cv::COLOR_GRAY2BGR);
					
						cv::addWeighted(out_i.m_data, 0.7, p.m_data, 0.3, 0,
								out_i.m_data);
					}
				}
				
				tag = images[i] + "/" + images[requested]; 
				H9_TRACE("Calibrate image %s grabbed,tag:%s", images[i].c_str(),tag);

				out.push_back(std::make_pair(images[i], out_i));
				
			}
		}
		return tag != "";
	}
	
	if (m_result["inspection"] && m_result["inspection"].IsSequence()) {
        H9_TRACE("Inspection IsSequence!");
		bool is_good = true;
		for (size_t i = 0; i < m_result["inspection"].size(); ++i)
			if (m_result["inspection"][i]["type"].IsScalar() &&
			    m_result["inspection"][i]["type"].as<std::string>() != "Good")
				is_good = false;
		if (is_good)
        {
            H9_TRACE("Inspection is_good!");
			return false;
        }
	} else if (m_result["inspection"] && m_result["inspection"].IsScalar() &&
		   m_result["inspection"].as<std::string>() == "Good")
    {
        H9_TRACE("inspection IsScalar!");
        return false;
    }
		

	if (!m_result["inspection"] || m_result["inspection"].IsNull())
    {
        H9_TRACE("inspection IsNull!");
        return false;
    }
	
    H9_TRACE("Before check m_defect_images[0]");
	if (m_defect_images.size() < 1 || !m_container.hasWrapper(m_defect_images.at(0)))
    {
        H9_TRACE("m_container has no m_defect_images[0]");
		return false;
    }
    H9_TRACE("Check m_defect_images[0] Success!");

	ObjectWrapper *wrapper = m_container.getWrapper(m_defect_images.at(0));
    H9_TRACE("Get m_defect_images[0] Success!");
	out.push_back(std::make_pair("DEFECT", wrapper->getImageObject()));
    H9_TRACE("GetDefectImage Finish!");
	return true;
}*/

bool InspectProcessor::GetDefectImage(std::vector<std::string>& vnames, std::map<std::string, ImageObject>& out, std::string& tag) const
//bool InspectProcessor::GetDefectImage(std::vector<std::pair<std::string, ImageObject> >& out, std::string& tag) const
{
    tag = "";

    if (m_mode == CAL) {
        std::vector<std::string> images;

        if (m_params && m_params["param_groups"] && m_params["param_groups"].IsSequence()) {

            for (size_t i = 0; i < m_params["param_groups"].size(); ++i)
                if (m_params["param_groups"][i].IsMap())
                    images.push_back(m_params["param_groups"][i].begin()->first.as<std::string>());
            vnames = images;
        }
        else {
            //images = std::vector<std::string>{"contour_img", "image_aligned", "inrange", "descript", "descript_text"};
            images = std::vector<std::string>{ "contour_img", "image_aligned", "inrange_a","inrange","inrange_close",
                                        "descript", "blister_dilate_mask","xor_mask","aligned_text","texture_result" };
            //, "texture_result"};
            // all names are hardcoded for now - have to match with the conf.yaml
            // maybe later we can change it to more generic
        }

        int requested = images.size() - 1;
        YAML::Node dummy;
        for (int im = images.size() - 1; im >= 0; --im)
            if (parse(dummy, images[im], 0) > 0) {
                requested = im;
                break;
            }
        H9_TRACE("Requested image is %d = %s", requested, images[requested].c_str());

        for (int i = 0; i <= requested; ++i) {
            H9_TRACE("Checking calibrate image %s", images[i].c_str());
            if (m_container.hasWrapper(images[i])) {
                ObjectWrapper* wrapper = m_container.getWrapper(images[i]);
                ImageObject out_i = wrapper->getImageObject();

                if (out_i.m_data.cols == 0 || out_i.m_data.rows == 0) {
                    H9_TRACE("Failed to grab CAL image %s", images[i].c_str());
                    continue;
                }

                if (out_i.colorType() == ImageObject::GRAY) {
                    cv::cvtColor(out_i.m_data, out_i.m_data, cv::COLOR_GRAY2BGR);
                    for (int j = i - 1; j >= 0; --j) {

                        if (m_container.hasWrapper(images[j])) {
                            wrapper = m_container.getWrapper(images[j]);
                            ImageObject p = wrapper->getImageObject();
                            if (p.m_data.cols > 0 && p.m_data.rows > 0 &&
                                p.colorType() == ImageObject::BGR) {
                                cv::addWeighted(p.m_data, 0.7, out_i.m_data, 0.3, 0,
                                        out_i.m_data);
                                break;
                            }
                        }
                    }
                    out_i.setColorType(ImageObject::BGR);
                }


                /*
                if (images[i] == "descript_text" && m_container.hasWrapper("texture_result")) {
                    wrapper = m_container.getWrapper("texture_result");
                    ImageObject p = wrapper->getImageObject();
                    if (p.m_data.cols > 0 && p.m_data.rows > 0) {
                        cv::cvtColor(p.m_data, p.m_data, cv::COLOR_GRAY2BGR);

                        cv::addWeighted(out_i.m_data, 0.7, p.m_data, 0.3, 0,
                                out_i.m_data);
                    }
                }
                */
                //lijun
                tag = images[i] + "/" + images[requested];
                H9_TRACE("Calibrate image %s grabbed,tag:%s", images[i].c_str(), tag.c_str());
                out[images[i]] = out_i;
                //if(i == requested) cv::imwrite("d://cal.bmp", out_i.m_data);

            }
        }
        return tag != "";
    }

    if (m_result["inspection"] && m_result["inspection"].IsSequence()) {
        bool is_good = true;
        for (size_t i = 0; i < m_result["inspection"].size(); ++i)
            if (m_result["inspection"][i]["type"].IsScalar() &&
                m_result["inspection"][i]["type"].as<std::string>() != "Good")
                is_good = false;
        if (is_good)
            return false;
    }
    else if (m_result["inspection"] && m_result["inspection"].IsScalar() &&
        m_result["inspection"].as<std::string>() == "Good")
        return false;

    if (!m_result["inspection"] || m_result["inspection"].IsNull())
        return false;

    if (!m_container.hasWrapper(m_defect_images.at(0)))
        return false;
    ObjectWrapper* wrapper = m_container.getWrapper(m_defect_images.at(0));
    //out.push_back(std::make_pair("DEFECT", wrapper->getImageObject()));
    out["DEFECT"] = wrapper->getImageObject();

    //std::cout << "defect_Object num::   " << m_defect_images.size() << std::endl;

    return true;
}

void InspectProcessor::newSample()
{
	if (m_total_rt_limit > 0)
		m_rt_limit_timer.reset();
}


YAML::Node InspectProcessor::calcNewParams(std::vector<YAML::Node> const& vec, std::string const& str)
{
    if (vec.empty()) {
        H9_ERROR("Empty input vector, vec.size(): %d", vec.size());
        return YAML::Node();
    }
    YAML::Node res_node;
    YAML::Node node = vec[0];
    ostringstream oss;
    oss << node;
    H9_INFO("calcNewParams input (element 0 in vec): \n%s", oss.str().c_str());
    if (node.IsMap()) {
        for (auto it = node.begin(); it != node.end(); it++) {
            string key = it->first.as<string>();
            if (it->second.IsScalar()) {
                float mean = 0;
                int num_node = vec.size();
                for (int i = 0; i < num_node; i++) {
                    mean += vec[i][key].as<int>();
                }
                mean /= num_node;
                res_node[key] = round(mean);
            }
            else if (it->second.IsSequence()) {
                YAML::Node seq_node = it->second;
                assert(seq_node.size() > 0);
                if (seq_node[0].IsScalar()) {
                    int seq_size = seq_node.size();
                    vector<float> means(seq_size, 0);
                    //float means[seq_size] = {0};
                    int num_node = vec.size();
                    for (int i = 0; i < num_node; i++) {
                        for (int j = 0; j < seq_size; j++) {
                            means[j] += vec[i][key][j].as<int>();
                        }
                    }
                    for (int j = 0; j < seq_size; j++) {
                        means[j] /= num_node;
                        res_node[key].push_back(round(means[j]));
                    }
                }
                else if (seq_node[0].IsSequence()) {
                    int num_node = vec.size();
                    int seq_size = seq_node.size();
                    int subseq_size = seq_node[0].size();
                    vector<vector<float>> means(seq_size, vector<float>(subseq_size, 0));
                    for (int i = 0; i < num_node; i++) {
                        for (int j = 0; j < seq_size; j++) {
                            for (int k = 0; k < subseq_size; k++) {
                                means[j][k] += vec[i][key][j][k].as<int>();
                            }
                        }
                    }
                    for (int j = 0; j < seq_size; j++) {
                        YAML::Node tmpnode;
                        for (int k = 0; k < subseq_size; k++) {
                            means[j][k] /= num_node;
                            tmpnode.push_back(round(means[j][k]));
                        }
                        res_node[key].push_back(tmpnode);
                    }
                }
            }
        }
    }
    else {
        H9_ERROR("node is not map! not supported yet.");
        return YAML::Node();
    }
    ostringstream oss1;
    oss1 << res_node;

    H9_INFO("m_params %d", m_params);
    H9_INFO("the calculated nodes: \n%s", oss1.str().c_str());

    // update m_params copy
    H9_INFO("m_params is null ?  %d, ptr=%lld", m_params, &m_params);
    H9_INFO("m_params.IsDefined(): %d", m_params.IsDefined());
    H9_INFO("m_params.size(): %d", m_params.size());
    YAML::Node params = YAML::Clone(m_params);
    H9_INFO("clone m_params success");
    for (auto it = res_node.begin(); it != res_node.end(); ++it) {
        string key = it->first.as<string>();
        if (params[str][key]) {
            H9_INFO("set %s", key.c_str());
            params[str][key] = res_node[key];
        }
    }
    H9_INFO("before return");
    return params;
}


bool InspectProcessor::ExecuteCommand(string const &key, YAML::Node const &node)
{
    H9_INFO("execute command: %s", key.c_str());

    H9_TIMER(m_name + ":" + key);
    
    if (node["skip"] && node["skip"].as<int>() == 1) {
        H9_INFO("Skip command %s", key.c_str());
        return true;
    }

    map<string, CommandType>::const_iterator cit = m_commands.find(key);
    if (m_commands.end() == cit) {
        H9_ERROR("Unknown command %s", key.c_str());
        return false;
    }
    CommandType cmd = cit->second;
    if (!(this->*cmd)(node)) {
        H9_ERROR("Failed to execute command %s", key.c_str());
        return false;
    }

    H9_INFO("Successfully execute command %s", key.c_str());
    return true;
}

bool InspectProcessor::LoadImageCommand(YAML::Node const &node)
{
    H9_PROFILER("InspectProcessor::LoadImageCommand");
    //int start = clock();
    if (!node["image"]) {
        H9_ERROR("load_image: image name missing");
        return false;
    }
    ObjectWrapper *wrapper = m_container.addWrapper(node["image"].as<string>());
    ImageObject &imgobj = wrapper->getImageObject();
    string path = "";
    if (node["path"] && node["path"].IsScalar()) {
        path = node["path"].as<string>();
    }
    //edit by lijun for path specical condition we should firstly read path not from upstream
    if (path.empty() && m_imgname.empty() && m_container.hasWrapper("FROM_UPSTREAM__")) {
	    imgobj.clone(m_container.getWrapper("FROM_UPSTREAM__")->getImageObject());
    } else if (!path.empty()) {
        if (!imgobj.readFromFile(path))
            return false;
    } else {
	    if (!imgobj.readFromFile(m_imgname))
		    return false;
    }
    
    wrapper->setValidType(ObjectWrapper::IMAGE);
    H9_INFO("Successfully loaded image");

    if (CAL == m_mode)
        return true;
    //int end = clock();
    //cout << "load image command time is: " << end - start << endl;
    if (!node["reference"]) {
        H9_INFO("load_image: reference name missing; assume not used");
        return true;
    }
    if (m_refimg.empty()) {
        H9_INFO("reference image not given; assume not used.");
        return true;
    }
    wrapper = m_container.addWrapper(node["reference"].as<string>());
    ImageObject &refobj = wrapper->getImageObject();
    if (!refobj.readFromFile(m_refimg)) {
        H9_ERROR("Failed to load reference image %s", m_refimg.c_str());
        return false;
    }
    wrapper->setValidType(ObjectWrapper::IMAGE);
    return true;
}

bool InspectProcessor::SaveImageCommand(YAML::Node const &node)
{
    if (m_disableSaveImage) {
	H9_TRACE("save_image command ignored");
	return true;
    }
	
    H9_PROFILER("InspectProcessor::SaveImageCommand");
    if (!node["image"]) {
        H9_ERROR("save_image: image name is missing");
        return false;
    }
    string name = node["image"].as<string>();
    ObjectWrapper *wrapper = m_container.getWrapper(name);
    if (!wrapper || !wrapper->hasValidType(ObjectWrapper::IMAGE)) {
        H9_ERROR("save_image: cannot save image with name %s", name.c_str());
        return false;
    }

    ImageObject &imgobj = wrapper->getImageObject();
    string out_file;
    if (node["out_file"])
        out_file = node["out_file"].as<string>();
    else if (node["tag"])
        {
            out_file = m_basename + "-" + node["tag"].as<string>() + ".png";
            H9_TRACE("save_image: out_file:%s,m_basename:%s",out_file.c_str(),m_basename.c_str());
        }
    else {
        H9_ERROR("save_image: image file name is not specified for saving");
        return false;
    }
    if (!imgobj.writeToFile(out_file))
        return false;

    return true;
}


#ifdef	USE_ONNXRuntime
bool InspectProcessor::YOLODetectCommand(YAML::Node const &node)
{
    //wjl
    cout << "YOLODetectCommand" << endl;
    H9_PROFILER(__func__);
    string command_str = "dd_yolo";
    if (!node["src"]) {
        H9_ERROR("%s: incorrect syntax", command_str.c_str());
        return false;
    }
    string srcname = node["src"].as<string>();
    ObjectWrapper *srcwrapper = m_container.getWrapper(srcname);
    if (!srcwrapper || !srcwrapper->hasValidType(ObjectWrapper::IMAGE)) {
        H9_ERROR("%s: src image %s does not exist", command_str.c_str(), srcname.c_str());
        return false;
    }
    ImageObject &srcobj = srcwrapper->getImageObject();

    if (!node["def_src"]) {
        H9_ERROR("%s: incorrect syntax", command_str.c_str());
        return false;
    }
    string defsrcname = node["def_src"].as<string>();
    ObjectWrapper *defsrcwrapper = m_container.getWrapper(defsrcname);
    if (!defsrcwrapper || !defsrcwrapper->hasValidType(ObjectWrapper::IMAGE)) {
        H9_ERROR("%s: def_src image %s does not exist", command_str.c_str(), defsrcname.c_str());
        return false;
    }
    ImageObject *defsrcobj = &defsrcwrapper->getImageObject();

    if (!node["def_img"]) {
        H9_ERROR("%s: incorrect syntax", command_str.c_str());
        return false;
    }
    //wjl
    cout << "YOLODetectCommand end" << endl;

    string defimgname = node["def_img"] ? node["def_img"].as<string>() : "$$DEFECT_CAPSULE";
    ObjectWrapper *defwrapper = NULL;
    
    bool isDefect = false;
    DefectData m_defectdata;

    H9_INFO("capsule_pack_detect: save defect image to %s", defimgname.c_str());
    defwrapper = m_container.addWrapper(defimgname);
    ImageObject *p_defimgobj = &defwrapper->getImageObject();
    p_defimgobj->clone(srcobj);
    defwrapper->setValidType(ObjectWrapper::IMAGE);

    //wjl
    cout << "YOLODetectCommand2" << endl;
    cv::Mat frame = defsrcobj->m_data;
    int channels_ = defsrcobj->m_data.channels();
    m_defectdata.m_basename = m_basename;

    m_defectdata.m_srcImg = defsrcobj;
    m_defectdata.m_defImg = p_defimgobj;
    if (frame.size().width &&
		frame.size().height && ((channels_ == 1) || (channels_ == 3) || (channels_ == 4))
		&& (frame.type() == CV_8UC1 || frame.type() == CV_8UC3 || frame.type() == CV_8UC4)) 
        {
		    size_t step = 0;
            const size_t lpn_len = 15;
            char lpn[lpn_len] = "\0";
            //wjl
            //cout << "defectlabel size:" << m_defectLabel.size() << endl;
            //cout << "normalIdx size:" << m_normalIdx.size() << endl;
            isDefect = detect(m_defectdata, step, session_id,lpn_len, lpn/*, m_defectLabel, m_normalIdx*/);
            //cout << "YOLODetectCommand2-detectend" << endl;
    }
    H9_INFO("YOLODetect has defect:", isDefect);   
    cout << "YOLODetectCommand2-frame" << endl;
    if (isDefect && defwrapper) {
        if (std::find(m_defect_images.begin(), m_defect_images.end(), defimgname) == m_defect_images.end())
        {
            //wjl
            //cout << "isDefect && defwrapper" << endl;
            m_defect_images.push_back(defimgname);
        }
        defwrapper->setValidType(ObjectWrapper::IMAGE);
        m_foundDefect = true;
        //cout << "YOLODetectCommand2-isDefect && defwrapper" << endl;

        // 缺陷图输出 outfile为文件名，defwrapper->...这段函数填充图片内容
        if(m_debug)
        {
            H9_INFO("pill_detect: save defect image to %s", defimgname.c_str());
            string outfile = m_basename + "_" + defimgname + ".png";
            H9_INFO("save capsule defect to file %s", outfile.c_str());
            defwrapper->getImageObject().writeToFile(outfile);
        }
    }
    cout << "YOLODetectCommand2-getDefectType" << endl;
    m_result[m_basename] = m_defectdata.getDefectType();
    //wjl
    //cout << m_result[m_basename] << endl;
    //cout << "YOLODetectCommand2 end" << endl;
    
    return true;
}

#endif

EngineBase *InspectProcessor::GetEngine(YAML::Node const &node)
{
    map<string, EngineBase*>::iterator it = m_engineset.find(node["INTERNAL_ENGINE_KEY"].Scalar());
    if(it == m_engineset.end()) {
        return NULL;
    }
    return it->second;
}

void InspectProcessor::SetEngine(YAML::Node const &node, EngineBase *ptr)
{
    map<string, EngineBase*>::iterator it = m_engineset.find(node["INTERNAL_ENGINE_KEY"].Scalar());
    if(it != m_engineset.end()) {
        delete it->second;
        m_engineset.erase(it);        
    }
    m_engineset.insert(pair<string, EngineBase*>(node["INTERNAL_ENGINE_KEY"].Scalar(), ptr));
}

void InspectProcessor::InitEngineSet()
{
    //cout << "cleanup start" << endl;
    CleanupEngineSet();
    //cout << "cleanup end" << endl;
    YAML::Node commands = m_config["flow"]["commands"];
    char buf[64];
    //cout << "for start" << endl;
    for (size_t i = 0; i < commands.size(); ++i) {
        YAML::Node command = commands[i];
        string key = command.begin()->first.as<string>();
        if (key == "break")
            continue;
        snprintf(buf, sizeof(buf), "%d", (int)i); 
        command[key]["INTERNAL_ENGINE_KEY"] = buf;
    }
    //cout << "for end" << endl;
}

void InspectProcessor::CleanupEngineSet()
{
         for(std::map<string, EngineBase*>::iterator it = m_engineset.begin();  it != m_engineset.end(); ++it)
         {
             delete it->second;
         }

         map<string, EngineBase*>().swap(m_engineset);

#ifdef	USE_ONNXRuntime
    bool session_closed = close_session(session_id);
#endif
}
    
void InspectProcessor::RegisterCommands()
{
    map<string, CommandType>().swap(m_commands); //把 m_commands 置空
    m_commands["load_image"] = &InspectProcessor::LoadImageCommand;
    m_commands["save_image"] = &InspectProcessor::SaveImageCommand;
#ifdef	USE_ONNXRuntime
    //onnxruntime
    m_commands["dd_yolo"] = &InspectProcessor::YOLODetectCommand;
#endif
}

}//H9
