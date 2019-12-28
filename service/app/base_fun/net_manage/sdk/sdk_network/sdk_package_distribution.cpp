/**
 * @file sdk_package_distribution.cpp
 * @author 余王亮 (wotsen@outlook.com)
 * @brief 
 * @version 0.1
 * @date 2019-09-21
 * 
 * @copyright Copyright (c) 2019
 * 
 */
#define LOG_TAG "SDK_PACKAGE_DISTRIBUTION"

#include <iostream>
#include <list>
#include <cstring>
#include <easylogger/easylogger_setup.h>
#include "util_time/util_time.h"
#include "../../../task_manage/task_manage.h"
#include "sdk_protocol_do.h"
#include "sdk_package_distribution.h"

using namespace wotsen;

// tcp消息处理接口
static void *task_sdk_stream_do(void *name) noexcept;

// TCP消息队列最大消息数量
static const int MAX_STREAM_LIST_LEN = 1024;

// tcp消息队列
static std::list<sdk_package<uv_stream_t>*> stream_list;

/**
 * @brief tcp消息入队
 * 
 * @tparam  libuv数据类型
 * @param package sdk数据包
 * @return true 入队成功
 * @return false 入队失败
 */
template <> bool push_sdk_package<uv_stream_t>(sdk_package<uv_stream_t> *package)
{
	if (!package)
	{
		log_w("package is null\n");
		return false;
	}

	// 队列已满
	if (stream_list.size() >= MAX_STREAM_LIST_LEN)
	{
		log_w("stream list full!\n");
		return false;
	}

	stream_list.push_back(package);

	return true;
}

/**
 * @brief tcp消息处理任务
 * 
 * @param name 任务名称
 * @return void* 
 */
static void *task_sdk_stream_do(void *name) noexcept
{
    log_i("sdk stream 数据处理初始化完成...\n");

	char *res_buf = new char[MAX_SDK_MSG_LEN];

	// 收到的消息
	sdk_package<uv_stream_t> *package = nullptr;

	// 响应消息
	uv_buf_t res;

    // 消息缓存
    struct sdk_data_buf req_data = {0, NULL};
    struct sdk_data_buf res_data = {0, NULL};

	res.len = 0;
	res.base = res_buf;

    res_data.data = res_buf;

	for (;;)
	{
		if (!stream_list.empty())
		{
			res.len = 0;
			memset(res_buf, 0, MAX_SDK_MSG_LEN);

			// 获取队列首部消息
			package = stream_list.front();

            // 请求消息
            req_data.len = package->recv_len;
            req_data.data = package->handle->data;

            // 响应消息内存长度
            res_data.len = MAX_SDK_MSG_LEN;

            // 协议处理
			sdk_protocol_do(req_data, res_data);

            // 响应消息实际长度
			res.len = res_data.len;

            // 发送响应消息
			package->write(package->handle, &res);

			// 消息出队
			stream_list.pop_back();
			delete package;
			package = nullptr;
		}
    }
}

/**
 * @brief 初始化sdk消息分发
 * 
 */
void task_sdk_package_distribution_init(void)
{
    task_create(task_sdk_stream_do, STACKSIZE(50*1024), "task_sdk_stream_do", OS_MIN(5), E_TASK_IGNORE);
}
