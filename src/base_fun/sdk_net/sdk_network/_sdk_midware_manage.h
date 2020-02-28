/**
 * @file _sdk_midware_manage.h
 * @author yuwangliang (astralrovers@outlook.com)
 * @brief 
 * @version 0.1
 * @date 2020-02-16
 * 
 * @copyright Copyright (c) 2020
 * 
 */
#pragma once

#include <list>
#include <string>
#include <memory>

class ISdkMidWare;

using sdk_midware_list = std::list<std::shared_ptr<ISdkMidWare>>;

// 中间件管理
class SdkMidWareManager {
public:
	SdkMidWareManager () {}
	~SdkMidWareManager() { midwares_.clear(); }

	// 注册中间件
	void register_midware(const std::shared_ptr<ISdkMidWare> &mid);

	// 中间件是否存在
	bool is_midware_exist(const std::string &name);

    sdk_midware_list &midwares(void);

private:
	// 中间件列表
	sdk_midware_list midwares_;
};

// 获取中间件管理器
SdkMidWareManager &get_sdk_midware_manager(void);
