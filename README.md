MarklessAR_Android is a demo project for a markless augmented reality application on Android. It's constructed under Android Studio with NDK and CMake. This project can be a good demonstration for how to use OpenCV with Android Studio and how to transplant c/c++ project into Android.

# Env
* IDE: Android Studio 2.2+
* OpenCV4Android: 2.4.11(other version will be fine)
* OpenGL ES 1/ ES 2

# How to use
1.Configure Android Studio for NDK and CMake, both of which can be downloaded via Android Studio SDK menu(Tools->Android->SDK Manager->SDK Tools);

2.Download this repository to local directory:
```
git clone https://github.com/FangGet/MarklessAR_Android.git
```

3.Download OpenCV4Android and decompressed it:
```
wget http://jaist.dl.sourceforge.net/project/opencvlibrary/opencv-android/2.4.11/OpenCV-2.4.11-android-sdk.zip
unzip OpenCV-2.4.11-android-sdk.zip
```

4.Import project into Android Studio(File->New->Import Project), find your project and click ok.

5.Revise some configurations:
(1) import OpenCV java SDK(File->New->Import Module), find OpenCV4Android->sdk->java and import it, dependency files will be generated automaticly;

(2) revise following variables:
``` 
compileSdkVersion 24
buildToolsVersion "25.0.2"
```
both OpenCV library and app's **build.gradle** should be revised.

(3)revise OpenCV destination:
for app's build.gradle:
```
sourceSets {
        main {
            jniLibs.srcDirs = ['/home/***/AndroidStudioProjects/MarklessAR/app/src/main/jniLibs']
        }
    }
```
for app's CMakeLists.txt:
```
set(PROJECT_PATH /home/lab213/AndroidStudioProjects/MarklessAR)
set(OPENCV_PATH /home/lab213/Documents/libraries/OpenCV-android-sdk)
```

Then it can be built and run properly.

# Further Development
In this demo project, only a coordinate system and simple cube is demonstrated, if you'd like to load some 3D models like .mqo file, refer to this project:
[https://github.com/takmin/OpenCV-Marker-less-AR](https://github.com/takmin/OpenCV-Marker-less-AR)

# My blog (Chinese)
[http://fangrenziwo.com/2016/12/30/markless-ar-mobile-as/](http://fangrenziwo.com/2016/12/30/markless-ar-mobile-as/)



Any question, contact with fangasfrank_at_gmail.com






