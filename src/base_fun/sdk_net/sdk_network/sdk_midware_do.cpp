/**
 * @file sdk_midware_do.cpp
 * @author yuwangliang (astralrovers@outlook.com)
 * @brief 
 * @version 0.1
 * @date 2020-02-16
 * 
 * @copyright Copyright (c) 2020
 * 
 */
#define LOG_TAG "SDK_MIDWARE_DO"
#include <easylogger/easylogger_setup.h>
#include "sdk_midware.h"
#include "_sdk_midware_manage.h"
#include "sdk_midware_do.h"

/**
 * @brief 
 * 
 * @param sdk_interface 网络接口
 * @param sdk_req 请求
 * @param sdk_res 应答
 * @return true 成功
 * @return false 失败
 */
bool sdk_midware_do(struct sdk_net_interface &sdk_interface, const Sdk &sdk_req, Sdk &sdk_res)
{
    auto &midwares = get_sdk_midware_manager().midwares();

    for (auto &item : midwares) {
		if (!item->is_enable()) { continue; }

		if (!(*item)(sdk_interface, sdk_req, sdk_res)) {
			log_d("proc midware %s failed\n", item->name().c_str());

			sdk_set_result(ResponseResult::ERROR, "error module : " + item->name(), sdk_res);

			// 一个中间件处理失败不再向后传递
			return false;
		}
		
		log_d("proc midware %s success\n", item->name().c_str());
	}

	return true;
}
