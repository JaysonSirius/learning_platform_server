/**
 * @file sdk_interface.cpp
 * @author yuwangliang (astralrovers@outlook.com)
 * @brief 
 * @version 0.1
 * @date 2020-01-29
 * 
 * @copyright Copyright (c) 2020
 * 
 */

#include "sdk_interface.h"

using namespace insider::sdk;

void sdk_set_result(const ResponseResult &status_code, const std::string &code, Sdk &sdk_res)
{
	sdk_res.mutable_footer()->mutable_result()->mutable_sdk_result()->set_status_code(status_code);
	sdk_res.mutable_footer()->mutable_result()->mutable_sdk_result()->set_code(code);
}

void sdk_set_result(const ResponseResult &status_code, const char *code, Sdk &sdk_res)
{
	sdk_set_result(status_code, std::string(code), sdk_res);
}

void sdk_content_set_result(const ContentResultE &status_code, const std::string &code, Sdk &sdk_res)
{
	sdk_res.mutable_footer()->mutable_result()->mutable_content_result()->set_status_code(status_code);
	sdk_res.mutable_footer()->mutable_result()->mutable_content_result()->set_code(code);
}

void sdk_content_set_result(const ContentResultE &status_code, const char *code, Sdk &sdk_res)
{
	sdk_content_set_result(status_code, std::string(code), sdk_res);
}

void sdk_content_set_result(const ContentResultE &status_code, Sdk &sdk_res)
{
	sdk_res.mutable_footer()->mutable_result()->mutable_content_result()->set_status_code(status_code);
}

void sdk_content_set_result(const std::string &code, Sdk &sdk_res)
{
	sdk_res.mutable_footer()->mutable_result()->mutable_content_result()->set_code(code);
}

void sdk_content_set_result(const char *code, Sdk &sdk_res)
{
	sdk_content_set_result(std::string(code), sdk_res);
}