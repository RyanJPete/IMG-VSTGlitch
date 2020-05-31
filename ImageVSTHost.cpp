/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "ImageVSTHost.h"
#include <cstdlib>
#include <windows.h>
#include <iostream>
#include <fstream>

const int bsize = 512;
const float sRate = 44100.0f;

extern "C" {
    VstIntPtr VSTCALLBACK hostCallback(AEffect *effect, VstInt32 opCode, VstInt32 index, VstIntPtr value, void *ptr, float opt){
        switch(opCode){
            case audioMasterVersion:
                printf("Plugin asking value of OpCode: %d (audioMasterVersion), returning 2400\n", opCode);
                return 2400;
            case audioMasterGetCurrentProcessLevel:
                printf("Plugin asking value of OpCode: %d (audioMasterGetCurrentProcessLevel), returning kVstProcessLevelUser\n", opCode);
                printf("opcode val %d\n", audioMasterGetCurrentProcessLevel);
                return kVstProcessLevelRealtime;
            case audioMasterGetBlockSize:
                printf("Plugin asking value of OpCode: %d(audioMasterGetBlockSize), returning %d\n", opCode, bsize);
                return bsize;
            case audioMasterGetSampleRate:
                printf("Plugin asking value of OpCode: %d (audioMasterGetSampleRate), returning %f\n", opCode, sRate);
                return sRate;
            case audioMasterGetTime:
                printf("Plugin asking value of OpCode: %d (audioMasterGetTime), returning null\n", opCode);
                return NULL;
            default:
                printf("Plugin wants value of OpCode: %d\n", opCode);
                break;
        }
        return -1;
    }
}

Host::Host(){
    numChannels = 2;
    blocksize = bsize;
}

AEffect* Host::loadPlugin(char* vstPath){
    AEffect *plugin = NULL;
        
    HMODULE modulePtr = LoadLibraryA(vstPath);
    if(modulePtr == NULL){      //check that load was successful
        printf("failed loading '%s' error code %d\n", vstPath, GetLastError());
        return NULL;
    }
    
    vstPluginFuncPtr mainEntryPoint = (vstPluginFuncPtr)GetProcAddress(modulePtr, "VSTPluginMain");
    //Create plugin instance
    plugin = mainEntryPoint(hostCallback);
    std::cout << "plugin loaded" << std::endl;
    return plugin;
}

long Host::configurePluginCallbacks(AEffect *plugin){
    //check magic number
    if(plugin->magic != kEffectMagic){
        printf("Plugin is invalid or corrupt");
        return -1;
    }
    
    //create dispatcher handle, maybe return this instead of plugin, I don't know how else I'll use it
    dispatcher = (dispatcherFuncPtr)(plugin->dispatcher);
    
    //Set up plugin callback functions
    plugin->getParameter = (getParameterFuncPtr)plugin->getParameter;
    plugin->processReplacing = (processFuncPtr)plugin->processReplacing;
    plugin->setParameter = (setParameterFuncPtr)plugin->setParameter;
    
    printf("plugin callbacks configured\n");
    return 1;
}

void Host::startPlugin(AEffect *plugin){
    dispatcherFuncPtr dispatcher = (dispatcherFuncPtr)(plugin->dispatcher);
    dispatcher(plugin, effOpen, 0, 0, NULL, 0.0f);
    
    //Set defaults
    float sampleRate = sRate;
    dispatcher(plugin, effSetSampleRate, 0, 0, NULL, sampleRate);
    int blocksize = 512;
    dispatcher(plugin, effSetBlockSize, 0, blocksize, NULL, 0.0f);
    printf("plugin started\n");
}

void Host::resumePlugin(AEffect *plugin){
    dispatcher(plugin, effMainsChanged, 0, 1, NULL, 0.0f);
}

void Host::suspendPlugin(AEffect *plugin){
    dispatcher(plugin, effMainsChanged, 0, 0, NULL, 0.0f);
}

//bool Host::canPluginDo(char *canDoString, AEffect *plugin){
  //  return (dispatcher(plugin, effCanDo, 0, 0, (void)*canDoString, 0.0f) > 0);
//}

void Host::initializeIO(){
    //inputs and outputs are float** (pointer arrays) and declared somewhere
    //probably owned by the class (what class tho)
    //numChannels and blocksize are size_t
    inputs = (float**)malloc(sizeof(float**) * numChannels);
    outputs = (float**)malloc(sizeof(float**)* numChannels);
    for(int channel = 0; channel < numChannels; channel++){
        inputs[channel] = (float*)malloc(sizeof(float*)*blocksize);
        outputs[channel] = (float*)malloc(sizeof(float*)*blocksize);
    }
    printf("I/O initialized\n");
}

void silenceChannel(float **channelData, int numChannels, long numFrames){
    for(int channel = 0; channel < numChannels; ++channel){
        for(long frame = 0; frame < numFrames; ++frame){
            channelData[channel][frame] = 0.0f;
        }
    }
}


void Host::printOutputs(){
    float it = outputs[0][0];
    int cnt = 0;
    for (int x = 0; x < inputSize; ++x){
        //printf("%g, ", it);
        it = outputs[0][cnt++];
    }
    printf("\n");
}

void Host::printInputs(){
    float it = inputs[0][0];
    int cnt = 1;
    for (int x = 0; x < inputSize; ++x){
        std::cout << it << ", ";
        it = inputs[0][cnt++];
    }
    //printf("\n");
}

void  Host::copyin2out(){
    for(int x = 0; x <= inputSize; ++x){
        outputs[0][x] = inputs[0][x];
    }
}
/* old reading method
void Host::copyHeader(){
    for(int x = 0; x <= 54/sizeof(float); ++x){
        outputs[0][x] = inputs[0][x];
    }
}

void Host::writeOutputs(std::string fName){
    std::ofstream image(fName, std::ios::out | std::ios::binary);
    char *holder = new char[iSize];
    
   // for(int x = 0; x < 10000; ++x){
     //   char mov = (char)inputs[0][x];
       // holder[x] = mov;
        //printf("%c, ", mov);
    //}
    
    int x = 0;
    int fcount = 0;
    while(x < iSize){
        //char mov = (char)outputs[0][x];
        //printf("\\ %g: ", outputs[0][x]);
        
        
        //turn floats into writeable chars
        unsigned char const * mov = reinterpret_cast<unsigned char const *>(&(outputs[0][fcount++]));
        for(std::size_t i = 0; i != sizeof(float); ++i){
            //printf(" %02X -", mov[i]);
            holder[x++] = mov[i];
        }
        //printf("/\n");
        //holder[x] = mov;
        
    }
    //printf("\n");
    image.write(holder, iSize);
    image.close();
}

int Host::loadImage(std::string fName){
    std::ifstream image(fName, std::ios::in | std::ios::binary | std::ios::ate);    //open image file
    if(!image.is_open()){
        return -1;
    }
    
    float mov;
    iSize = image.tellg();          //get image size
    char *holder = new char[iSize]; //array to hold image contents
    image.seekg(0, std::ios::beg);
    image.read(holder, iSize);      //read the image into holder
    
    inputSize = iSize/sizeof(mov);
    if(iSize %  sizeof(mov) != 0){
        inputSize++;
    }
    
    inputs[0] = (float*)malloc(sizeof(float*)*inputSize);
    inputs[1] = (float*)malloc(sizeof(float*)*inputSize);
    outputs[0] = (float*)malloc(sizeof(float*)*inputSize);
    outputs[1] = (float*)malloc(sizeof(float*)*inputSize);
    int inputcnt = 0;
    
    
        
    for(int x = 0; x < iSize; x = x + sizeof(mov)){     //change input chars into floats with 4 chars to a float
        
        
        u_char charray[sizeof(mov)];
        //put 4 bytes into a float
        for(int count = 0; count < sizeof(mov); ++count){   //create chararray of sizeof(float) chars
            if(x + count < iSize){
               //printf("%c:", holder[x + count]);
                charray[count] = holder[x + count];
            }
            
        }
                        //printf("\n");

        //std::cout << std::endl;
        memcpy(&mov, &charray, sizeof(mov)); //turn chararray into a float
        //std::cout << mov << "/";
        inputs[0][inputcnt] = mov;     //put the float into inputs
        inputs[1][inputcnt] = mov;
        //std::cout << inputs[0][inputcnt] << std::endl;
        inputcnt++;
    }
    image.close();
    return 1;
}*/


//change reading method
void Host::copyHeader(){
    for(int x = 0; x <= 54; ++x){
        outputs[0][x] = inputs[0][x];
    }
}

void Host::writeOutputs(std::string fName){
    std::ofstream image(fName, std::ios::out | std::ios::binary);
    char *holder = new char[iSize];
    
   // for(int x = 0; x < 10000; ++x){
     //   char mov = (char)inputs[0][x];
       // holder[x] = mov;
        //printf("%c, ", mov);
    //}
    
    int x = 0;
    int fcount = 0;
    while(x < iSize){
        //char mov = (char)outputs[0][x];
        //printf("\\ %g: ", outputs[0][x]);
        
        
        //turn floats into writeable chars
        unsigned char const * mov = reinterpret_cast<unsigned char const *>(&(outputs[0][fcount++]));
        for(std::size_t i = 0; i != sizeof(float); ++i){
            //printf(" %02X -", mov[i]);
            if(i == 0){
                holder[x++] = mov[i];
            }
        }
        //printf("/\n");
        //holder[x] = mov;
        
    }
    //printf("\n");
    image.write(holder, iSize);
    image.close();
}

int Host::loadImage(std::string fName){
    std::ifstream image(fName, std::ios::in | std::ios::binary | std::ios::ate);    //open image file
    if(!image.is_open()){
        return -1;
    }
    
    float mov;
    iSize = image.tellg();          //get image size
    char *holder = new char[iSize]; //array to hold image contents
    image.seekg(0, std::ios::beg);
    image.read(holder, iSize);      //read the image into holder
    
    inputSize = iSize;
    //if(iSize %  sizeof(mov) != 0){
      //  inputSize++;
    //}
    
    inputs[0] = (float*)malloc(sizeof(float*)*inputSize);
    inputs[1] = (float*)malloc(sizeof(float*)*inputSize);
    outputs[0] = (float*)malloc(sizeof(float*)*inputSize);
    outputs[1] = (float*)malloc(sizeof(float*)*inputSize);
    int inputcnt = 0;
    
    
        
    for(int x = 0; x < iSize; x = x + 1){     //change input chars into floats with 4 chars to a float
        
        
        u_char charray[sizeof(mov)];
        //put 1 bytes into a float
        for(int count = 0; count < sizeof(mov); ++count){   //create chararray of sizeof(float) chars
            if(count == 0){
               //printf("%c:", holder[x + count]);
                charray[count] = holder[x];
            } else {
                charray[count] = ' ';
            }
            
        }
                        //printf("\n");

        //std::cout << std::endl;
        memcpy(&mov, &charray, sizeof(mov)); //turn chararray into a float
        //std::cout << mov << "/";
        inputs[0][inputcnt] = mov;     //put the float into inputs
        inputs[1][inputcnt] = mov;
        //std::cout << inputs[0][inputcnt] << std::endl;
        inputcnt++;
    }
    image.close();
    return 1;
}
std::string Host::getParamName(AEffect *plugin, int index){
    char name[] = "";
    dispatcher(plugin, effGetParamName, index, 0, (void*)name, 0.0f);
    std::string ret = name;
    return ret;
}

std::string Host::getParamLabel(AEffect *plugin, int index){
    char label[] = "";
    dispatcher(plugin, effGetParamLabel, index, 0, (void*)label, 0.0f);
    std::string ret = label;
    return ret;
}

std::string Host::getParamDisplay(AEffect *plugin, int index){
    char display[] = "";
    dispatcher(plugin, effGetParamDisplay, index, 0, (void*)display, 0.0f);
    std::string ret = display;
    return ret;
}

void Host::processAudio(AEffect *plugin){
    //reset output array before processing
    //silenceChannel(outputs, numChannels, numFrames);
    //printf("outputs silenced\n");
    //When processing instrument clear input channel
    //When processing effects clear output channel
    //When inputing data from files not neccasary, I'm leaving it in for now but will probably remove it later
    //silenceChannel(inputs, numChannels, numFrames);
    //printf("inputs silenced\n");
    //std::cout << iSize << std::endl;
    plugin->processReplacing(plugin, inputs, outputs, iSize); //inputs are channels (as in right and left, array arrays dummy
    //printf("channel processed\n");
}

