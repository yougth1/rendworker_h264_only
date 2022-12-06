#include "alisdk.h"


AliSDKHandle *AliSDKHandle::AliSDKHandleInstance = NULL;

string AliSDKHandle::AliSDKUploadHandle(const string& UploadBucketName, const string& LocalFileAbsolutePath, const string& RemoteObjectName)
{
    std::shared_ptr<std::iostream> content = std::make_shared<std::fstream>(LocalFileAbsolutePath.c_str(), std::ios::in | std::ios::binary);
    PutObjectRequest request(UploadBucketName, RemoteObjectName, content);

    auto outcome = client->PutObject(request);
    if (!outcome.isSuccess()) {
        string errInfo = "PutObject failed with code :[" + outcome.error().Code() + "], error message : [" + outcome.error().Message() + "], requestId : [" + outcome.error().RequestId() + "]";
        return errInfo;
    }
    return "";
}

string AliSDKHandle::AliSDKDownloadHandle(const string& DownloadBucketName, const string& RemoteObjectName, const string& FileNametoSave)
{
    GetObjectRequest request(DownloadBucketName, RemoteObjectName);
    request.setResponseStreamFactory([=]() {
        return std::make_shared<std::fstream>(FileNametoSave, std::ios_base::out | std::ios_base::in | std::ios_base::trunc| std::ios_base::binary);
    });

    auto outcome = client->GetObject(request);
    if (outcome.isSuccess())
        std::cout << "GetObjectToFile success" << outcome.result().Metadata().ContentLength() << std::endl;
    else {
        string errInfo = "GetObjectToFile failed with code :[" + outcome.error().Code() + "], error message : [" + outcome.error().Message() + "], requestId : [" + outcome.error().RequestId() + "]";
        return errInfo;
    }
    return "";
}