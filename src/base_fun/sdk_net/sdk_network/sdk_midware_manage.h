/**
 * @file sdk_midware_manage.h
 * @author yuwangliang (astralrovers@outlook.com)
 * @brief 
 * @version 0.1
 * @date 2020-02-16
 * 
 * @copyright Copyright (c) 2020
 * 
 */
#pragma once

#include "sdk_midware.h"

// 中间件注册
void register_sdk_midware(const std::shared_ptr<ISdkMidWare> &mid);

/****************************************************************************************************
 * 两种方式注册中间件：
 * 1.扩展ISdkMidWare子类，重载()方法
 * 2.使用类似函数接口注册
 * 无论哪种方式都会生成ISdkMidWare对象中间件
****************************************************************************************************/

// 使用中间件子类创建中间件对象
#define SDK_CREATE_MIDWARE(midware, name, enable) \
	register_sdk_midware(std::make_shared<mideware>(std::string(name), enable))

// 导入中间件接口用于创建中间件默认对象
#define SDK_IMPORT_MIDWARE(midware, name, enable) \
	extern bool midware(struct sdk_net_interface &sdk_interface, const Sdk &sdk_req, Sdk &sdk_res);\
	register_sdk_midware(std::make_shared<ISdkMidWare>(std::string(name), enable, midware))
