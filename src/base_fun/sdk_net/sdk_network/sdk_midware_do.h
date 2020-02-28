/**
 * @file sdk_midware_do.h
 * @author yuwangliang (astralrovers@outlook.com)
 * @brief 
 * @version 0.1
 * @date 2020-02-16
 * 
 * @copyright Copyright (c) 2020
 * 
 */
#pragma once

#include "sdk_interface.h"

using namespace insider::sdk;

/**
 * @brief 
 * 
 * @param sdk_interface 网络接口
 * @param sdk_req 请求
 * @param sdk_res 应答
 * @return true 成功
 * @return false 失败
 */
bool sdk_midware_do(struct sdk_net_interface &sdk_interface, const Sdk &sdk_req, Sdk &sdk_res);
