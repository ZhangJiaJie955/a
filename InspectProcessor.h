#ifndef _INSPECTPROCESSOR_H
#define _INSPECTPROCESSOR_H
#include <string>
#include <map>
#include <vector>
#include <yaml-cpp/yaml.h>
#include "objectcontainer.h"
#include "EngineBase.h"
#include "h9Log.h"
#include "h9Timer.h"

#ifdef	USE_ONNXRuntime
#include "Open_LPReditor_Lib.h"
#include "utils_image_file.h"
#endif
namespace H9 {

class ImageObject;
	
class InspectProcessor
{
public:
    enum InspectMode {CAL = 0,
                      INSPECT,
                      UNKNOWN
    };

public:
    explicit InspectProcessor(InspectMode m_ = UNKNOWN, const std::string& name_ = "")
	    : m_mode(m_), m_name(name_), m_testmode(false),  m_foundDefect(false), /*m_bOcrFuzzyMatch(false),m_strOcrTemplate(""),*/
          m_flipped(false), m_missing_return_ng(false), m_debug(false), m_disableSaveImage(false),
          m_npills(0), m_nrows(0), m_ncols(0),
	      m_rt_limit_timer(""), m_rt_limit(-1), m_total_rt_limit(-1), m_current_process_start(0)
    {
            RegisterCommands();
         }
    ~InspectProcessor() {
          cleandata(1);
          CleanupEngineSet();
		  }

    bool Initialize(std::map<std::string, std::string> const &);
    bool Initialize(const char* flow, const char* design_template, const char* detectors,
		    const char* options, InspectMode m = INSPECT);
    bool LoadParamYaml(std::string const &);
    bool LoadDetectorYaml(std::string const &);
    bool Execute(std::string const &);
    bool Execute(const ImageObject&);
    void cleandata(int cleanup=1)
    {
        if(cleanup) {m_container.cleanup();}
        m_foundDefect = false;
        m_flipped = false;
	    m_defect_images.clear();
        m_result.reset();
    }
    bool Finalize() const;
    bool OutputResult(std::string&) const;
    //bool GetDefectImage(std::vector<std::pair<std::string, ImageObject> >&, std::string&) const;
    bool GetDefectImage(std::vector<std::string>&, std::map<std::string, ImageObject>&, std::string&) const;
    void setDebugFlag(bool d) { m_debug = d; }
    void disableSaveImage(bool d = true) {m_disableSaveImage = d; }

    void newSample();
    YAML::Node getUpdatedParams() { return m_params_to;}
    YAML::Node calcNewParams(std::vector<YAML::Node> const& vec, std::string const& str = "pouch");
#ifdef	USE_ONNXRuntime
    bool needTrigger(std::string& triggerImgPath) { triggerImgPath = m_triggerImg; return (m_needTrigger && !m_triggerImg.empty()); };
#endif
    void updateParams(YAML::Node const &params) {m_params = YAML::Clone(params);}
 private:
    void RegisterCommands();
    bool inspectionDone() const {return m_foundDefect;}
    bool executeFlow();
    bool ExecuteCommand(std::string const &, YAML::Node const &);

    bool findKey(YAML::Node const &, std::string const &) const;
    template<typename T>
    T parse(YAML::Node const &, std::string const &, T const&) const;
    template<typename T>
    std::vector<T> parseList(YAML::Node const &, std::string const &, std::vector<T> const&) const;

    bool LoadImageCommand(YAML::Node const &);
    bool SaveImageCommand(YAML::Node const &);
 


    //

#ifdef	USE_ONNXRuntime
    //onnxruntime
    bool YOLODetectCommand(YAML::Node const &);
    
#endif
private:
    typedef bool (InspectProcessor::*CommandType)(YAML::Node const &);
    std::map<std::string, CommandType> m_commands;

    InspectMode m_mode;
    std::string m_name;
    bool m_testmode;
    bool m_foundDefect;
    bool m_flipped;
    bool m_missing_return_ng;
    /*
    bool m_bOcrFuzzyMatch;
    std::string m_strOcrTemplate;
    */
#ifdef	USE_ONNXRuntime
    bool m_needTrigger;
    std::string m_triggerImg;
    std::set<int> m_normalIdx;
    std::map<int, std::string> m_defectLabel;
    int m_totalLabelNum;
#endif

    YAML::Node m_config;
    YAML::Node m_params;
    YAML::Node m_detectors;
    YAML::Node m_params_to;
    mutable YAML::Node m_result;

    std::string m_imgname;
    std::string m_basename;
    std::string m_refimg;
    std::string m_resultfile;
    std::vector<std::string> m_defect_images;
    
    ObjectContainer m_container;

    bool m_debug;
    bool m_disableSaveImage;
    
    std::vector<int> m_lens_center;
    int m_npills;
    int m_nrows;
    int m_ncols;
    std::string m_modelpath;
    std::string m_modeltype;
    std::string m_omcpath;
    //std::string m_model_det_path;
    //std::string m_model_cls_path;
    //std::string m_model_rec_path;
    //std::string m_keys_path;

    std::vector<std::vector<int> > m_corners;//golal cornner shared between differnt engines
    std::string modelDetPath;
    std::string modelClsPath;
    std::string modelRecPath;
    std::string keysPath;
private:
    std::map<std::string, EngineBase*> m_engineset;
    EngineBase *GetEngine(YAML::Node const &node);
    void SetEngine(YAML::Node const &node, EngineBase *ptr);
    void InitEngineSet();
    void CleanupEngineSet();

private:
    H9::Timer m_rt_limit_timer;
    float m_rt_limit, m_total_rt_limit;
    float m_current_process_start;
    // m_rt_limit is the runtime limit per process; m_total_rt the limit per
    // sample - each sample may come with multiple exposures - thus multi-processes
    // in sequence; m_current_process_start is the reading of the total process
    // time at the beginning of current process - reset for each sample
private:
    size_t session_id;
};
}//H9

#endif
