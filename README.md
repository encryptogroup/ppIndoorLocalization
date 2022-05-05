# Privacy-Preserving Indoor Localization

C++ Library implementing a Client-Server Privacy Preserving Indoor Localisation Protocol and a simple Android Application which uses it. 

## Folder Structure description

* app: This folder contains the mobile application, which will trigger the evaluation part
* PPILLib: Ths folder contains the Library which is used by the mobile application. This Library is base on the Mobile Private Contact Discovery Library. Essentially here are all the parts of the protocol implemented.
* PPILLib/droidCrypto/Server: This folder contains the part of the Server, which will trigger the garbling part 
* Database: This folder contains a few examples of how the Database-files should look like and can be used for testing.


## Requirements

* JAVA JNI libaries
* C++ compiler supporting C++14
* Phone with arm64-v8a ABI
* Android NDK 21+
* Android SDK 28+


## Build instructions

* Before Building: fill in the Config-File (PPILLib/Config.h) with your Path to the Server Database and the IP and Port of the Host of the connection (e.g. of your Router)
* Server:
```bash
cd PPILLib
mkdir build && cd build
cmake ..
make -j
```
You may will receive the error "Could NOT find JNI". In this case you will have to got to /PPILLib/build/CMakeCache.txt and set the following variables manually:
JAVA_INCLUDE_PATH (path to your jni.h), JAVA_INCLUDE_PATH2 (path to your jni_md.h), JAVA_AWT_INCLUDE_PATH(path to your jawt.h)
* Mobile App: Open gradle project in Android Studio, build and deploy. (Debugging and Datatransfer must be enaled on connected device)
## Run Instructions

* run the main function with the number of Accesspoints: e.g. '''PPILLib/droidCrypto/Server/main 20'''
* Start the mobile application 

## Test programs


## Disclaimer

This code is provided as a experimental implementation for testing purposes and should not be used in a productive environment. We cannot guarantee security and correctness.

## Acknowledgements

This project is based on the Mobile Private Contact Discovery Library [droidCrypto](https://github.com/contact-discovery/mobile_psi_cpp) by Daniel Kales.

