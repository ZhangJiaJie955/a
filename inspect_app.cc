#include <iostream>
#include "InspectProcessor.h"
#include <time.h>
#include <string>
#include <map>
#include <vector>
#include <fstream>
#include "h9Log.h"
#include "h9Timer.h"

// 我加的
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include "image.h"
// 我加的


using namespace std;

namespace {

void printhelp(string const &appname)
{
    cout << appname << " <--img <imagename> | --list <imagelistfile> > [-c <config> [-l <logfile> [-o <options>]]]" << endl;
}

bool parseOptions(string const &optstr, map<string, string> &options)
{
    return true;
}

bool parseParams(int argc, char *argv[], map<string, string> &params)
{
    if (argc % 2 == 0) {
        cout << "ERROR: incorrect command line parameter count" << endl;
        printhelp(argv[0]);
        return false;
    }

    map<string, string> tmp_params;

    int nparams = argc / 2;
    int cnt = 0;
    string config;
    string log;
    for (int i = 0; i != nparams; ++i, cnt += 2) {
        if (string("-c") == argv[cnt]) {
            tmp_params["conf"] = argv[cnt + 1];
        } else if (string("-l") == argv[cnt]) {
            tmp_params["log"] = argv[cnt + 1];
        } else if (string("-o") == argv[cnt]) {
            if (!parseOptions(argv[cnt + 1], tmp_params)) {
                cout << "Failed to parse command line options: " << argv[cnt + 1] << endl;
                return false;
            }
            cout << "Successfully parsing command line options" << endl;
        } else if (string("--img") == argv[cnt]) {
            tmp_params["image"] = argv[cnt + 1];
        } else if (string("--list") == argv[cnt]) {
            tmp_params["img_list"] = argv[cnt + 1];
        } else if (string("--loglevel") == argv[cnt]) {
            tmp_params["loglevel"] = argv[cnt + 1];
        } else if (string("--debugflag") == argv[cnt]) {
            tmp_params["debugflag"] = argv[cnt + 1];
        } else {
            cout << "Unknown command line parameter " << argv[cnt] << endl;
            return false;
        }
    }
    params.swap(tmp_params);
    return true;
}

// imglist 是一个文件，如imglist.txt，里面记录了待检测所有图片的路径，每一行为一张图片的路径
// 文件的最后一行 必须以 # 结尾
bool parseImageNames(string const &imglist, vector<string> &imgnames)
{
    ifstream ifs(imglist.c_str());
    if (!ifs) {
        cout << "Failed to open image list file " << imglist << endl;
        return false;
    }

    string line;
    while (getline(ifs, line)) {
        if ('#' == line[0])
            continue;
        imgnames.push_back(line);
    }

    if (!ifs.eof()) {
        cout << "Failed to parse image list file " << imglist << endl;
        return false;
    }

    return true;
}

}//namespace

class FileHandler
{
public:
    FileHandler(string fname) : m_fp(fopen(fname.c_str(), "w")) {}
    ~FileHandler() {fclose(m_fp);}

    FILE *file() {return m_fp;}

private:
    FILE *m_fp;
};

int main(int argc, char** argv)
{
    cout << "start" << endl;
    int start2 = clock();
    H9::init_timer();
    H9::InspectProcessor proc;

    /*if (1 == argc) {
        printhelp(argv[0]);
        return 0;
    }*/

    proc.setDebugFlag(true);
    
    map<string, string> params;
    //parseParams(argc, argvCopy, params);
    //parseParams(2, argvCopy, params);
    
    // 
    //params["conf"] = "D:\\OCR\\inspect-pouch-master\\out\\RelWithDebInfo\\conf_cal.yaml";
    params["conf"] = "C:\\zjj\\Project\\shiyanshi\\inspect_algo\\asset\\conf_inspect_1.yaml";
    //params["mode"] = "inspect";
    //params["conf"] = "D:/inspect/x64/Release/conf_inspect.yaml";
    //params["img_list"] = "D:/inspect/x64/Release/square_ring_descript.txt";
    //params["img_list"] = "D:/inspect/x64/Release/square_ring_detect.txt";
    
    //params["image"] = "D:/inspect/x64/Release/1227.bmp";
    //params["image"] = "D:/inspect/x64/Release/ok.bmp";
    //params["image"] = "D:/inspect/x64/Release/Green-descript.png";
    //params["image"] = "D:/inspect/x64/Release/bl.bmp";
    //params["image"] = "D:/inspect/x64/Release/dl.bmp";
    //params["image"] = "D:/inspect/x64/Release/ql.bmp";
    //params["image"] = "D:/inspect/x64/Release/ys.bmp";
    //params["image"] = "D:/inspect/x64/Release/descript-default.bmp";
    //params["image"] = "D:/inspect/x64/Release/packdefect1227.bmp";
    //params["image"] = "D:/inspect/x64/Release/pilldefect1227.bmp";
    //在对象params中索引"image"键对应的值是原图片的路径地址8
    params["image"] = "C:\\zjj\\Project\\shiyanshi\\inspect_algo\\demo_img\\B11.bmp";
    
    //解引用下行代码可以开始多张检测
    //params["img_list"] = "C:\\shiyanshiproject\\inspectdpbase\\build\\RelWithDebInfo\\img_list.txt";
    //params["image"] = "C:/shiyanshiproject/data/softtrigger.bmp";
    //params["image"] = "D:/Release/data/1.bmp"; 
    //params["image"] = "D:\\OCR\\inspect-pouch-master\\out\\RelWithDebInfo\\gaosu-test\\0119caltest.bmp";
    //params["image"] = "D:\\OCR\\inspect-pouch-master\\out\\Release\\ok.bmp";
    //params["img_list"] = "D:/OCR/inspect-pouch-master/out/RelWithDebInfo/hlw-16-test/test-03.txt";
    //params["image"] = "D:/OCR/inspect-pouch-master/out/RelWithDebInfo/hlw-xianchang/OUTPUT_478_original_NG.bmp";
    //params["image"] = "E:\\Data_yaodai\\pouch-hlw\\20230330\\RESULT_20230330T15_white\\OUTPUT_399_original_NG.bmp";

    //params["image"] = "D:/inspect/x64/Release/quejiao-2.bmp";
    //params["image"] = "D:/inspect/x64/Release/duanpian-1.bmp";
    //params["image"] = "D:/inspect/x64/Release/xianwen-1.bmp";
    //params["image"] = "D:/inspect/x64/Release/liewen-2.bmp";
    
    //params["image"] = "D:/inspect/x64/Release/heidian09.bmp";
    //params["image"] = "D:/inspect/x64/Release/liangdu2.bmp";

    
    string logname("inspect.log");

    map<string, string>::iterator pit = params.find("log");
    if (params.end() != pit) {
        logname = pit->second;
        params.erase(pit);
    }
    cout << "Log is redirected to " << logname << endl;
    FileHandler fhandler(logname);

    int log_level = LOG_TRACE;
    pit = params.find("loglevel");
    if (params.end() != pit) {
        log_level = atoi(pit->second.c_str());
        params.erase(pit);
    }
    if (log_level < 0 || log_level > LOG_FATAL) {
        cout << "ERROR: invalid loglevel, should be in the range [0, 5]" << endl;
        log_level = LOG_INFO;
    }
    cout << "Log level is set to " << log_level << endl;
    H9_LOG_INIT_FP(fhandler.file(), log_level);

    int debugflag = 1;
    pit = params.find("debugflag");
    if (params.end() != pit) {
        debugflag = atoi(pit->second.c_str());
        params.erase(pit);
    }
    cout << "debugflag is set to " << debugflag << endl;
    proc.setDebugFlag(debugflag!=0);
    
    vector<string> imgnames;
    pit = params.find("image");
    if (params.end() == pit) {
        cout << "fail" << endl;
        pit = params.find("img_list");
        if (params.end() == pit) {
            cout << "ERROR: image is not specified" << endl;
            printhelp(argv[0]);
            return 1;
        }
        if (!parseImageNames(pit->second, imgnames)) {
            cout << "parseImageNames failed" << endl;
            return 1;
        }
        params.erase(pit);
    } else {
        imgnames.push_back(pit->second);
        params.erase(pit);

        pit = params.find("img_list");
        if (params.end() != pit) {
            cout << "Both image and image list are specified; will handle both" << endl;
            H9_WARN("Both image and image list are specified; will handle both");
            if (!parseImageNames(pit->second, imgnames))
                return 1;
        }
    }
    H9_LOG("Number of images to be inspected %d:", (int)imgnames.size());
    cout << "Number of images to be inspected " << imgnames.size() << endl;
    for (vector<string>::const_iterator iit = imgnames.begin(); iit != imgnames.end(); ++iit) {
        H9_LOG(iit->c_str());
        cout << iit->c_str()<< std::endl;
    }
    cout << endl;

    // 检查YAML文件是否可以导入 （暂未涉及YAML内容）
     bool success = proc.Initialize(params);

    if(!success){
        cout<<"Initialization failed"<<endl;
        H9_ERROR("Initialization Failed");
        return 1;
    }
    else {
        cout << "Initialization OK" << endl;
    }

    //int start = clock();
    int i = 0;
    // 原来的调用代码，传入 imagenams 调用exec
    // 
    //for (vector<string>::const_iterator iit = imgnames.begin(); iit != imgnames.end(); ++iit) {
    //    std::string str;
    //    int start = clock();
    //    H9_PROFILER("Run Inspection");
    //    cout << "Run Inspection " << *iit<<endl;
	   // proc.newSample();
    //    cout << "newsample" << endl;

    //    // 下面这个 proc.Execute()该是执行检测的函数。输入是待检测图片的绝对路径
    //    if (!proc.Execute(*iit)) {
    //        cout << "Failed to execute YAML flow for image " << iit->c_str() << endl;
    //        H9_ERROR("Failed to execute YAML flow for image %s", iit->c_str());
    //        return 1;
    //    }
    //    cout << "Execute" << endl;
    //    proc.OutputResult(str);
    //    std::cout << str << endl;
    //    
    //    proc.cleandata();
    //    //int end = clock();3
    //    //std::cout << "Execute time is : " << end - start << std::endl;
    //    std::cout << endl << "the" << i << "times" << endl;
    //    //int end = clock();
    //    //
    //    i++;
    //}

    // 我加的传入 ImageObject 对象作为参数，调用execute，单张图
    
    // 不符合尺寸的
    //cv::Mat image = cv::imread("C:\\shiyanshiproject\\data\\Pictures\\img_size_test.png");
    cv::Mat image = cv::imread("C:\\zjj\\Project\\shiyanshi\\Detector\\Detector\\demo_img\\B11.bmp");
    // 添加判断语句做保护

    // 将 RGB 转为 BGR  因为 ImageObject 默认是 BGR 的（构造函数设置的）
    // OpenCV 默认的颜色制式排列就是 BGR，所以就不用转换了？
    //cv::imshow("image", image);
    //cv::waitKey(0);

    H9::ImageObject inputImage = H9::ImageObject(image);

    proc.Execute(inputImage);
    cout << "Execute" << endl;

    // str 包括所有检测结果输出信息。
    std::string str;
    proc.OutputResult(str);

   // 这里不做输出
   std::cout << str << endl;
        
    proc.cleandata();
    // 到这里为止

    if (!proc.Finalize()) {
        H9_ERROR("Flow Finalization Failed");
        return 1;
    }

    H9::Timer::summarize();


    /*std::cout << std::endl << "hello,world";*/

    //int end2 = clock();
    //std::cout << "整体用时  ：" << end2 - start2 << std::endl;


    return 0;
}