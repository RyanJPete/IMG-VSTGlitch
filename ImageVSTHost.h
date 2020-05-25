/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   LeanrVSTHosting.h
 * Author: Jerome
 *
 * Created on April 18, 2019, 4:34 PM
 */

#ifndef IMAGEVSTHOST_H
#define IMAGEVSTHOST_H

#include <iostream>
#include "aeffectx.h"

extern "C"{
        //Main host callback this is a declaration, probably obvious but now I know. I guess this makes "C" a function variable for this callback function (not sure what callback means here)
        //actually still confused on what VSTCALLBACK is and how it works, update, it's an alias for __cdel which is a calling convention where the caller has to clean up the stack and probably some other stuff, read into it sometime
        VstIntPtr VSTCALLBACK hostCallback(AEffect *effect, VstInt32 opCode, VstInt32 index, VstIntPtr value, void *ptr, float opt);
    }

//Plugin entry point, function pointer returning AEffect taking audioMasterCallback called vstPluginFuncPtr 
typedef AEffect *(*vstPluginFuncPtr)(audioMasterCallback host);
//Plugin dispatcher function created function pointer dispatcherFuncPtr with return type VstIntPtr
typedef VstIntPtr (*dispatcherFuncPtr)(AEffect *effect, VstInt32 opCode, VstInt32 index, VstInt32 value, void *ptr, float opt);
//plugin getparameter() method
typedef float (*getParameterFuncPtr)(AEffect *effect, VstInt32 index);
//plugin setParameter() method
typedef void (*setParameterFuncPtr)(AEffect *effect, VstInt32 index, float value);
//Plugin processEvents() method
typedef VstInt32 (*processEventsFuncPtr)(VstEvents *events);
//Plugin process() metho
typedef void (*processFuncPtr)(AEffect *effect, float **inputs, float **outputs, VstInt32 sampleFrames);

class Host{
    float** inputs;
    float** outputs;
    size_t numChannels; 
    size_t blocksize;
    dispatcherFuncPtr dispatcher;
    
public: 
    Host();
    AEffect* loadPlugin(char* vstPath);
    int iSize;
    int inputSize;  //derived from iSize, number of floats the chars in iSize translate into
    char header[12];
    
    long configurePluginCallbacks(AEffect *plugin);
    void startPlugin(AEffect *plugin);
    void resumePlugin(AEffect *plugin);
    void suspendPlugin(AEffect *plugin);
    bool canPluginDo(char *canDoString, AEffect *plugin);
    void initializeIO();
    int loadImage(std::string fName);
    void processAudio(AEffect *plugin);
    void printOutputs();        
    void printInputs();
    void writeOutputs(std::string fName);
    void copyin2out();
    void copyHeader();
    std::string getParamName(AEffect *plugin, int index);
    std::string getParamLabel(AEffect *plugin, int index);
    std::string getParamDisplay(AEffect *plugin, int index);
};
#endif /* IMAGEVSTHOST_H */

