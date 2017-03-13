// RUN: %hc %s -o %t.out -lhc_am -DRUNMASK=0xFF && HCC_SERIALIZE_KERNEL=0x3 HCC_SERIALIZE_COPY=0x3 %t.out
#include<stdlib.h>
#include<iostream>

#include<hc.hpp>
#include<hc_am.hpp>

#include"common.h"

#define N 1024*1024

const size_t size = sizeof(float) * N;

#include "common2.h"

#if not defined (RUNMASK)
#define RUNMASK 0xFF
#endif

#if not defined (ITERS)
#define ITERS 2000
#endif


int main(){
    std::vector<hc::accelerator> accs = hc::accelerator::get_all();
    hc::accelerator gpu_acc;
    for(auto& it:accs){
        if(!it.get_is_emulated()){
            gpu_acc = it;
            break;
        }
    }

    Init(gpu_acc);
    hc::accelerator_view av = gpu_acc.get_default_view();


    // RUNMASK should be #defined as input to compilation:
    unsigned testsToRun = RUNMASK;
    int testIters = ITERS;


    if (testsToRun & HostToDeviceCopyTest) {
        for(uint32_t i=0;i<testIters;i++){
            if ((i%1000 == 0)) {
                printf ("info: running Test1 %5d/%5d\n", i, testIters);
            }
            Test1(av);
        }
    }

    if (testsToRun & DeviceToDeviceCopyTest) {
        for(uint32_t i=0;i<testIters;i++){
            if ((i%1000 == 0)) {
                printf ("info: running Test2 %5d/%5d\n", i, testIters);
            }
            Test2(av);
        }
    }

    if (testsToRun & DeviceToHostCopyTest) {
        for(uint32_t i=0;i<testIters;i++){
            if ((i%1000 == 0)) {
                printf ("info: running Test3 %5d/%5d\n", i, testIters);
            }
            Test3(av);
        }
    }

// Create a vector of hc::completion_future for async copy synconization
    std::vector<hc::completion_future> cfs;
    
    if (testsToRun & HostToDeviceAsyncCopyTest) {
        for(uint32_t i=0;i<testIters;i++){
            if ((i%1000 == 0)) {
                printf ("info: running Test4 %5d/%5d\n", i, testIters);
            }
            cfs.push_back(Test4(av));
        }
        for(uint32_t i=0;i<testIters;i++)
	    cfs[i].wait();
    }

    cfs.clear(); 
    if (testsToRun & DeviceToHostAsyncCopyTest) {
        for(uint32_t i=0;i<testIters;i++){
            if ((i%1000 == 0)) {
                printf ("info: running Test5 %5d/%5d\n", i, testIters);
            }
            cfs.push_back(Test5(av));
        }
        for(uint32_t i=0;i<testIters;i++)
	    cfs[i].wait();
    }

 // To release all allocated resources from Init
     Destroy();

}

