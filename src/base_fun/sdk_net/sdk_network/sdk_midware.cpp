/**
 * @file sdk_midware.cpp
 * @author 余王亮 (wotsen@outlook.com)
 * @brief 
 * @version 0.1
 * @date 2019-11-10
 * 
 * @copyright Copyright (c) 2019
 * 
 */
#define LOG_TAG "SDK_MIDWARE"

#include <vector>
#include <easylogger/easylogger_setup.h>
#include "sdk_midware.h"

// 中间件是否使能
bool ISdkMidWare::is_enable(void) const
{
	return enable_;
}

// 中间件是否匹配
bool ISdkMidWare::is_match(const std::string &name) const
{
	return name == name_;
}

// 中间件名称
const std::string &ISdkMidWare::name(void) const
{
	return name_;
}

// 仿函数，用于中间件实际处理
bool ISdkMidWare::operator () 
		(struct sdk_net_interface &sdk_interface, const insider::sdk::Sdk &sdk_req, insider::sdk::Sdk &sdk_res) const
{
	// 如果使用函数注册，则执行函数接口
	if (fn_) { return fn_(sdk_interface, sdk_req, sdk_res); }
	return true;
}
