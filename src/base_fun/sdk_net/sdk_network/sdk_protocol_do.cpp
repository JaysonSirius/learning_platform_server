/**
 * @file sdk_protocol_do.cpp
 * @author 余王亮 (wotsen@outlook.com)
 * @brief 
 * @version 0.1
 * @date 2019-09-22
 * 
 * @copyright Copyright (c) 2019
 * 
 */
#define LOG_TAG "SDK_PROTOCOL_DO"

#include <cstdio>
#include <cstring>
#include <time.h>
#include <string>
#include <inttypes.h>
#include <easylogger/easylogger_setup.h>
#include "tools_func/calculate_crc16.h"
#include "../sdk_tree/sdk_tree_do.h"
#include "sdk_protocol_do.h"
#include "sdk_midware_do.h"

using namespace insider::sdk;

// header数据处理
static bool sdk_header_check(struct sdk_net_interface &sdk_interface, const Sdk &sdk_req);
// 业务内容内容处理
static bool sdk_body_do(struct sdk_net_interface &sdk_interface, const Sdk &sdk_req, Sdk &sdk_res);

// 中间件处理，中间件处理失败时直接响应
static bool sdk_midware_proc(struct sdk_net_interface &sdk_interface, const Sdk &sdk_req, Sdk &sdk_res);
// body处理
static bool sdk_body_proc(struct sdk_net_interface &sdk_interface, const Sdk &sdk_req, Sdk &sdk_res);

// 打包响应消息头
static void sdk_pack_res_msg_header(struct sdk_net_interface sdk_interface, const Sdk &sdk_req, Sdk &sdk_res);

// 魔术数字校验
static bool sdk_header_check_magic(const Header &header);
// 版本检查
static bool sdk_header_check_version(const Header &header);
// 网络接口检查
static bool sdk_header_check_net(const Header &header, const struct sdk_net_interface &sdk_interface);
// 数据时间检查
static bool sdk_header_check_time(const Header &header);
// 数据方向检查
static bool sdk_header_check_data_dir(const Header &header);

/**
 * @brief 业务内容内容处理
 * 
 * @param sdk_interface 网络接口
 * @param sdk_req 请求
 * @param sdk_res 响应
 * @return true 成功
 * @return false 失败
 */
static bool sdk_body_do(struct sdk_net_interface &sdk_interface, const Sdk &sdk_req, Sdk &sdk_res)
{
	const Body &body = sdk_req.body();
	OperationType op_type = body.method();

	if (op_type > OperationType::DELETE)
	{
		log_e("sdk method not match\n");
		return false;
	}

	log_i("requset url : %s\n", body.url().c_str());

	// 根据处理结果组包数据
	return sdk_tree_do(sdk_interface, sdk_req, sdk_res);
}

// 魔术数字校验
static bool sdk_header_check_magic(const Header &header)
{
	// 魔术数字检查
	if (SdkMagic::SDK_MAGIC != header.msg_magic())
	{
		log_e("msg magic is %x\n", header.msg_magic());
		return false;
	}

	return true;
}

// 版本检查
static bool sdk_header_check_version(const Header &header)
{
	// 版本检查，不同版本不兼容，后续可去除
	if (SdkVersion::SDK_CUR_VERSION != header.version())
	{
		log_e("msg version is %x\n", header.version());
		return false;
	}

	return true;
}

// 网络接口检查
// FIXME:存在代理、网关等跨网段时是否可能回有问题
static bool sdk_header_check_net(const Header &header, const struct sdk_net_interface &sdk_interface)
{
	// 网络类型匹配
	if (sdk_interface.trans_protocol != header.trans_proto() || 
		sdk_interface.ip_version != header.host().ip_version())
	{
		log_e("trans proto not match\n");
		return false;
	}

	// 源地址检查
	if (sdk_interface.src_ip != header.host().ip()
		|| (int)sdk_interface.src_port != header.host().port())
	{
		log_i("client ip = %s, port = %d, sdk ip = %s, sdk port = %d\n",
				sdk_interface.src_ip.c_str(), sdk_interface.src_port,
				header.host().ip().c_str(), header.host().port());
		return false;
	}

	// 目的地址检查
	if (sdk_interface.des_ip != header.dest().ip()
		|| (int)sdk_interface.des_port != header.dest().port())
	{
		log_i("server ip = %s, port = %d, sdk ip = %s, sdk port = %d\n",
				sdk_interface.des_ip.c_str(), sdk_interface.des_port,
				header.dest().ip().c_str(), header.dest().port());
		return false;
	}

	return true;
}

// 数据时间检查
static bool sdk_header_check_time(const Header &header)
{
	// 时间错误，两边的时间不同步
	if (time(NULL) < header.time().in_time())
	{
		log_e("time error\n");
		return false;
	}
	return true;
}

// 数据方向检查
static bool sdk_header_check_data_dir(const Header &header)
{
	// 数据方向
	if (DataFlow::DATA_IN != header.data_dir())
	{
		log_e("data dir error\n");
		return false;
	}
	return true;
}

// 打包响应消息头
static void sdk_pack_res_msg_header(struct sdk_net_interface sdk_interface, const Sdk &sdk_req, Sdk &sdk_res)
{
	Header *header = sdk_res.mutable_header();

	// 魔术数字
	header->set_msg_magic(SdkMagic::SDK_MAGIC);
	// 版本
	header->set_version(SdkVersion::SDK_CUR_VERSION);
	// 包序号，使用请求的
	header->set_pack_id(sdk_req.header().pack_id());
	// 时间
	header->mutable_time()->set_out_time(time(NULL));
	// 方向
	header->set_data_dir(DataFlow::DATA_OUT);

	// 响应时，地址反过来
    header->mutable_host()->set_ip_version(sdk_interface.ip_version);
	header->mutable_host()->set_port(sdk_interface.des_port);
	header->mutable_host()->set_ip(sdk_interface.des_ip);

    header->mutable_dest()->set_ip_version(sdk_interface.ip_version);
	header->mutable_dest()->set_port(sdk_interface.src_port);
	header->mutable_dest()->set_ip(sdk_interface.src_ip);

	// 传输协议
	header->set_trans_proto(sdk_interface.trans_protocol);

	return ;
}

// header数据处理
static bool sdk_header_check(struct sdk_net_interface &sdk_interface, const Sdk &req_sdk)
{
	const Header &header = req_sdk.header();

	return sdk_header_check_magic(header)
		   && sdk_header_check_version(header)
		   && sdk_header_check_net(header, sdk_interface)
		   && sdk_header_check_time(header)
		   && sdk_header_check_data_dir(header);
}

// 中间件处理，中间件处理失败时直接响应
static bool sdk_midware_proc(struct sdk_net_interface &sdk_interface, const Sdk &sdk_req, Sdk &sdk_res)
{
	return sdk_midware_do(sdk_interface, sdk_req, sdk_res);
}

// body内容处理
static bool sdk_body_proc(struct sdk_net_interface &sdk_interface, const Sdk &sdk_req, Sdk &sdk_res)
{
	return sdk_body_do(sdk_interface, sdk_req, sdk_res);
}

/**
 * @brief sdk协议处理
 * 
 * @param sdk_interface 网络接口
 * @param sdk_req 请求
 * @param sdk_res 应答
 */
void sdk_protocol_do(struct sdk_net_interface &sdk_interface, const Sdk &sdk_req, Sdk &sdk_res)
{
	// 头部检查
	if (!sdk_header_check(sdk_interface, sdk_req))
	{
		return ;
	}

	// 需要填写回去
	sdk_res.mutable_body()->set_url(sdk_req.body().url());
	sdk_res.mutable_body()->set_method(sdk_req.body().method());

	/* NOTE:如果是中间件出错，其中的sdk错误码由中间件填写 */
	if (!sdk_midware_proc(sdk_interface, sdk_req, sdk_res))
	{
		// 中间件响应消息不走正常流程，单独存在错误码，会填写具体中间件
		sdk_pack_res_msg_header(sdk_interface, sdk_req, sdk_res);
		return ;
	}
	
	// 业务部分错误码由业务本身填写，原因在于业务功能相对中间件更为复杂，不能够统一处理
	if (sdk_body_proc(sdk_interface, sdk_req, sdk_res))
	{
		sdk_set_result(ResponseResult::OK, "ok", sdk_res);
	}

	// 响应消息
	sdk_pack_res_msg_header(sdk_interface, sdk_req, sdk_res);

	return ;
}
