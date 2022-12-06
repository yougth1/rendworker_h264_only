#ifndef __ALISDK_HPP__
#define __ALISDK_HPP__
#include <iostream>
#include <string>
#include <alibabacloud\\oss\\OssClient.h>
#include <memory>
#include <fstream>

using std::cout;
using std::endl;
using std::string;
using namespace AlibabaCloud::OSS;

class AliSDKHandle
{
private:
    OssClient *client = NULL;

    AliSDKHandle()
    {
        InitializeSdk();
    }

    ~AliSDKHandle()
    {
        ShutdownSdk();
    }

    static AliSDKHandle *AliSDKHandleInstance;

public:
    string AliSDKUploadHandle(const string& UploadBucketName, const string& LocalFileAbsolutePath, const string& RemoteObjectName);
    string AliSDKDownloadHandle(const string& DownloadBucketName, const string& RemoteObjectName, const string& FileNametoSave);
    static AliSDKHandle *GetInstance()
    {
        if (NULL == AliSDKHandleInstance)
            AliSDKHandleInstance = new AliSDKHandle();
        return AliSDKHandleInstance;
    }
    bool AliClientInit(const string& Endpoint, const string& AccessKeyId, const string& AccessKeySecret)
    {
        ClientConfiguration conf;
        if (NULL == client)
            client = new OssClient(Endpoint, AccessKeyId, AccessKeySecret, conf);
        return true;
    }

    bool AliClientClose()
    {
        if (nullptr != client)
        {
            delete client;
            client = nullptr;
        }
        return true;
    }
};

#endif