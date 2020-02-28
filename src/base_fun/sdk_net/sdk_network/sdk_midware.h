/**
 * @file sdk_midware.h
 * @author 余王亮 (wotsen@outlook.com)
 * @brief 
 * @version 0.1
 * @date 2019-11-10
 * 
 * @copyright Copyright (c) 2019
 * 
 */
#ifndef __SDK_MIDWARE_H__
#define __SDK_MIDWARE_H__

#include "sdk_interface.h"

using namespace insider::sdk;

// sdk中间件处理
using sdk_midware_fn = bool (*)(struct sdk_net_interface &sdk_interface, const Sdk &sdk_req, Sdk &sdk_res);

class ISdkMidWare
{
private:
	std::string name_;			///< 中间件名称
	bool enable_;				///< 使能
	sdk_midware_fn fn_;			///< 处理接口(该接口优先级更高)

public:
	ISdkMidWare(const std::string &name, const bool &enable, sdk_midware_fn fn = nullptr)
		: name_(name), enable_(enable), fn_(fn) {}
	virtual ~ISdkMidWare() {}

	// 中间件是否使能
	virtual bool is_enable(void) const;
	// 中间件是否匹配
	virtual bool is_match(const std::string &name) const;
	// 中间件名称
	virtual const std::string &name(void) const;

	// 仿函数，子类可重写该方法用于处理自身逻辑，
	// 如果不重写该方法则不会发生任何事情
	virtual bool operator () 
		(struct sdk_net_interface &sdk_interface, const Sdk &sdk_req, Sdk &sdk_res) const;
};

#endif // !__SDK_MIDWARE_H__
