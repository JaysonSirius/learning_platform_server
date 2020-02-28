/**
 * @file sdk_midware_manage.cpp
 * @author yuwangliang (astralrovers@outlook.com)
 * @brief 
 * @version 0.1
 * @date 2020-02-16
 * 
 * @copyright Copyright (c) 2020
 * 
 */
#define LOG_TAG "SDK_MIDWARE_MANAGE"

#include <easylogger/easylogger_setup.h>
#include "_sdk_midware_manage.h"
#include "sdk_midware_manage.h"

// 中间件管理器
static SdkMidWareManager midware_mannager;

// 获取中间件列表
sdk_midware_list &SdkMidWareManager::midwares(void)
{
    return midwares_;
}

// 注册中间件
void SdkMidWareManager::register_midware(const std::shared_ptr<ISdkMidWare> &mid)
{
	// 不能重复注册
	if (is_midware_exist(mid->name())) { return; }

	midwares_.push_back(mid);
}

// 中间件是否存在
bool SdkMidWareManager::is_midware_exist(const std::string &name)
{
	for (auto &item : midwares_) {
		if (item->is_match(name)) { return true; }
	}

	return false;
}

SdkMidWareManager &get_sdk_midware_manager(void)
{
    return midware_mannager;
}

// 中间件注册
void register_sdk_midware(const std::shared_ptr<ISdkMidWare> &mid)
{
	midware_mannager.register_midware(mid);
}
