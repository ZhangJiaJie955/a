cmake_minimum_required(VERSION 3.17)

# 设置宏定义开关 USE_ONNXRuntime
# define options in outside level's CMakelists.txt
option(USE_ONNXRuntime    "Set to switch to build use ONNXRuntime" ON)
option(USE_CUDA    "Set to switch to build use CUDA" ON)

project(algo_inspect)

# ONNXRuntime关联的CUDA库
message(STATUS "before USE_ONNXRuntime=${USE_ONNXRuntime}, USE_CUDA=${USE_CUDA}")
# 提示WIN32平台
message(STATUS "!!!!  _WIN32=${_WIN32}  !!!! ")

#set(config_lib_root "C:\\zjj\\Project\\shiyanshi\\inspect_algo\\cfg\\lib")

# YAML库配置
set(YAML_CPP_INCLUDE_DIR "C:\\zjj\\Project\\shiyanshi\\inspect_algo\\cfg\\include")
set(YAML_CPP_LIBRARIES "C:\\zjj\\Project\\shiyanshi\\inspect_algo\\cfg\\lib\\yaml-cpp.lib")

# OpenCV 455库配置
# set(OpenCV_INCLUDE_DIRS "C:\\zjj\\Project\\shiyanshi\\inspect_algo\\cfg\\include\\opencv2")
# set(OpenCV_LIBS "C:\\zjj\\Project\\shiyanshi\\inspect_algo\\cfg\\lib\\opencv_world455.lib")

# OpenCV 3416库配置
set(OpenCV_INCLUDE_DIRS "C:\\zjj\\Project\\shiyanshi\\inspect_algo\\cfg\\include")
set(OpenCV_LIBS "C:\\zjj\\Project\\shiyanshi\\inspect_algo\\cfg\\lib\\opencv_world3416.lib")

# Boost库配置
set(Boost_INCLUDE_DIR "C:\\zjj\\Project\\shiyanshi\\inspect_algo\\cfg\\include\\boost-1_66")
set(Boost_LIBRARY_DIR_1 "C:\\zjj\\Project\\shiyanshi\\inspect_algo\\cfg\\lib\\boost_filesystem-vc141-mt-x64-1_66.lib")
set(Boost_LIBRARY_DIR_2 "C:\\zjj\\Project\\shiyanshi\\inspect_algo\\cfg\\lib\\boost_system-vc141-mt-x64-1_66.lib")

# ONNX库配置
SET(ONNXRT_INC_DIR "C:\\zjj\\Project\\shiyanshi\\inspect_algo\\cfg\\include\\onnxruntime")
SET(ONNXRT_LIB_DIR "C:\\zjj\\Project\\shiyanshi\\inspect_algo\\cfg\\lib\\onnxruntime.lib")

include_directories(
  ${OpenCV_INCLUDE_DIRS}
  ${YAML_CPP_INCLUDE_DIR}
  ${Boost_INCLUDE_DIR}
  ${ONNXRT_INC_DIR}
  )

link_directories(
  ${config_lib_root}
  )

if (USE_ONNXRuntime)
	ADD_DEFINITIONS(-DUSE_ONNXRuntime)
	message(STATUS "Use ONNXRuntime (CPU) !!!")
elseif (USE_CUDA)
	ADD_DEFINITIONS(-DUSE_ONNXRuntime)
	ADD_DEFINITIONS(-DLPR_EDITOR_USE_CUDA)
	message(STATUS "Use ONNXRuntime (CUDA) !!! define LPR_EDITOR_USE_CUDA")
endif()

message(STATUS "after USE_ONNXRuntime=${USE_ONNXRuntime}, USE_CUDA=${USE_CUDA}")


if	(USE_ONNXRuntime OR  USE_CUDA)
  message("USE_ONNXRuntime OR  USE_CUDA")
SET(SOURCES
  Open_LPReditor_Lib.cc
  InspectProcessor.cc
  image.cc
  strobj.cc
  objectwrapper.cc
  objectcontainer.cc
  inspect_util.cc
  defect_data.cc
  ColorPredictor.cc
  Levenshtein.cc
  h9Log.cc
  h9Timer.cc
  Line.cc
  ONNX_detector.cc
  utils_anpr_detect.cc
  utils_image_file.cc
  yolov5_anpr_onnx_detector.cc 
  getopt.cc
  str_util.cpp

)

else()
message("DO NOT USE_ONNXRuntime OR  USE_CUDA")
SET(SOURCES
  InspectProcessor.cc
  image.cc
  strobj.cc
  h9Log.cc
  h9Timer.cc
  objectwrapper.cc
  objectcontainer.cc
  inspect_util.cc
  defect_data.cc
  ColorPredictor.cc
  Levenshtein.cc
  getopt.cc
  str_util.cpp
)
endif()  

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

ADD_LIBRARY(inspect_algo SHARED ${SOURCES})
include(GenerateExportHeader)
generate_export_header(inspect_algo)

TARGET_LINK_LIBRARIES(inspect_algo
 ${OpenCV_LIBS}
 ${YAML_CPP_LIBRARIES}
 ${ONNXRT_LIB_DIR}
${Boost_LIBRARY_DIR_1}
${Boost_LIBRARY_DIR_2}
)

#SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
#LINK_DIRECTORIES(inspect_app ${PROJECT_BINARY_DIR}/lib)
add_executable (inspect_app inspect_app.cc inspect_util.cc)

# 这还必须写在后面
TARGET_LINK_LIBRARIES(inspect_app inspect_algo
${OpenCV_LIBS}
${YAML_CPP_LIBRARIES}
${ONNXRT_LIB_DIR}
${Boost_LIBRARY_DIR_1}
${Boost_LIBRARY_DIR_2}
)

