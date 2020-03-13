/**
 * @file sdk_protocol_do.h
 * @author 余王亮 (wotsen@outlook.com)
 * @brief 
 * @version 0.1
 * @date 2019-09-22
 * 
 * @copyright Copyright (c) 2019
 * 
 */
#pragma once

#include <string>
#include "sdk_interface.h"

// sdk协议处理
void sdk_protocol_do(struct sdk_net_interface &sdk_interface, const insider::sdk::Sdk &sdk_req, insider::sdk::Sdk &sdk_res);
