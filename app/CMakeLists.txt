# Sets the minimum version of CMake required to build the native
# library. You should either keep the default value or only pass a
# value of 3.4.0 or lower.

set(PROJECT_NAME native-lib)
cmake_minimum_required(VERSION 3.4.1)
set(CMAKE_VERBOSE_MAKEFILE on)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")

set(PROJECT_PATH /home/lab213/AndroidStudioProjects/MarklessAR)
set(OPENCV_PATH /home/lab213/Documents/libraries/OpenCV-android-sdk)

list(APPEND LINK_LIBS "-lEGL;-lGLESv1_CM")


# add opencv library
add_library(lib_opencv SHARED IMPORTED)
set_target_properties(lib_opencv PROPERTIES IMPORTED_LOCATION ${PROJECT_PATH}/app/src/main/jniLibs/${ANDROID_ABI}/libopencv_java.so)

find_library( log-lib log )

# add ml-ar library
SET(ML_CPP_ROOT ${PROJECT_PATH}/app/src/main/cpp)
list(APPEND ML_SOURCES
     ${ML_CPP_ROOT}/src/ARDrawingContext.cpp
     ${ML_CPP_ROOT}/src/CameraCalibration.cpp
     ${ML_CPP_ROOT}/src/GeometryTypes.cpp
     ${ML_CPP_ROOT}/src/ARPipeline.cpp
     ${ML_CPP_ROOT}/src/Pattern.cpp
     ${ML_CPP_ROOT}/src/PatternDetector.cpp)

list(APPEND ML_INCLUDES
    ${ML_CPP_ROOT}/include
    ${OPENCV_PATH}/sdk/native/jni/include)

include_directories(${ML_INCLUDES})
add_library(ml_ar SHARED ${ML_SOURCES})
target_link_libraries(ml_ar ${log-lib} lib_opencv ${LINK_LIBS})

# include_directories(${OPENCV_PATH}/sdk/native/jni/include)
add_library(${PROJECT_NAME} SHARED src/main/cpp/ml_ar.cpp )
target_link_libraries(${PROJECT_NAME} ${log-lib} ml_ar ${LINK_LIBS})
