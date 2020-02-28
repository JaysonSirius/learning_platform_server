/**
 * @file usr_apps.cpp
 * @author 余王亮 (wotsen@outlook.com)
 * @brief 
 * @version 0.1
 * @date 2019-08-17
 * 
 * @copyright Copyright (c) 2019
 * 
 */
#define LOG_TAG "USR_APPS"

#include <pthread.h>
#include <easylogger/inc/elog.h>
#include "util_time/util_time.h"
#include "sys_capability.h"
#include "task_manage/task_manage.h"
#include "sdk_net/sdk_network/sdk_network.h"
#include "sdk_net/sdk_network/sdk_midware_manage.h"
#ifdef DEBUG
#include "sdk_net/sdk_tree/sdk_tree_do.h"
#endif
#include "handy_loop/handy_loop.h"

// 应用模块导入
/**************************************************************************************/

/* 私有模块 */
#include "upgrade/upgrade.h"
#include "user_manage/user_manage.h"

/* 三方模块 */

/**************************************************************************************/

#include "os_param.h"
#include "usr_apps.h"

using namespace insider::sdk;

using app_module_fun = void (*)(void);
using app_module_status_fun = bool (*)(void);

// 模块列表结束符
#define END_OF_APP_MODULE { create_moudle_base("", false, E_APP_MODULE_BAD, E_APP_MODULE_CFG_PERMISSION_DISENABLE) }

// 生成一个模块的基本信息
static AppModuleBaseInfo create_moudle_base(const char *name, const bool enable,
                                      const AppModuleState state, const AppModuleCfgPermission permission);

// sdk中间件注册
static void register_sdk_midwares(void);
// 基础功能模块初始化
static void base_function_module_init(void);
// 应用业务模块初始化
static void applications_module_init(void);

/**
 * @brief 模块配置信息
 * 
 */
struct app_module_config {

    app_module_config() {}

    app_module_config(const AppModuleBaseInfo &base_info) : base_info(base_info), init(nullptr), exit(nullptr), status(nullptr)
    {
    }

    app_module_config(const AppModuleBaseInfo &base_info, const app_module_fun &init, app_module_fun exit, app_module_status_fun status)
                        : base_info(base_info), init(init), exit(exit), status(status)
    {
        // static_assert(init != nullptr, "app module init is null!");
    }

    AppModuleBaseInfo base_info;    ///< 模块基本配置信息
    app_module_fun init;            ///< 模块初始化接口
    app_module_fun exit;            ///< 模块卸载接口
    app_module_status_fun status;   ///< 模块状态接口
};

/**
 * @brief 模块管理器
 * 
 */
class AppModuleManager {
private:
    // 模块管理句柄
    static AppModuleManager *app_module_manager_;
    // 模块列表
    static struct app_module_config app_modules_[OS_SYS_MAX_APP_MODULES];
    // 模块最大数量
    uint32_t app_module_num_;
    // 互斥锁
    pthread_mutex_t mutex_;
public:
    AppModuleManager();

    ~AppModuleManager();

    // 模块管理句柄
    static AppModuleManager *get_app_module_manager(void);

    // 模块初始化
    void app_modules_init(void) noexcept;
    // 单个模块初始化
    void init_single_app_module(const uint32_t id, const std::string &name) noexcept;
    // 单个模块卸载
    void finit_single_app_module(const uint32_t id, const std::string &name) noexcept;
    // 获取当前所有模块状态
    void app_modules_current_status(AppModuleCoutInfoList &moduel_list) noexcept;
    // 合法模块数量
    uint32_t valid_module_num(void) const noexcept;
};

AppModuleManager::AppModuleManager() : app_module_num_(OS_SYS_MAX_APP_MODULES) {
    pthread_mutex_init(&mutex_, NULL);

    log_i("init app modules [%d]\n", valid_module_num());
}

AppModuleManager::~AppModuleManager() {
    pthread_mutex_destroy(&mutex_);
}

// 合法模块数量
uint32_t AppModuleManager::valid_module_num(void) const noexcept
{
    uint32_t valid_module = 0;

    for (uint32_t i = 0; i < app_module_num_; i++)
    {
        if (app_modules_[i].init)
        {
            valid_module++;
        }
    }

    return valid_module;
}

// 模块管理句柄
AppModuleManager *AppModuleManager::get_app_module_manager(void)
{
    if (!app_module_manager_)
    {
        app_module_manager_ = new AppModuleManager();
    }

    return app_module_manager_;
}

// 模块初始化
void AppModuleManager::app_modules_init(void) noexcept {
    pthread_mutex_lock(&mutex_);

    struct app_module_config *item = app_modules_;

    for (int i = 0; i < (int)app_module_num_; i++)
    {
        if (item[i].base_info.enable() && E_APP_MODULE_IDLE == item[i].base_info.state() && item[i].init)
        {
            log_d("init app modules : %s\n", item[i].base_info.name().c_str());
            item[i].init();
            item[i].base_info.set_state(E_APP_MODULE_INSTALLED);
        }
    }
    pthread_mutex_unlock(&mutex_);
}

// 单个模块初始化
void AppModuleManager::init_single_app_module(const uint32_t id, const std::string &name) noexcept
{
    if (id >= app_module_num_) { return ; }

    pthread_mutex_lock(&mutex_);
    struct app_module_config *item = &app_modules_[id];

    if (name == item->base_info.name()
        && item->base_info.enable()
        && E_APP_MODULE_IDLE == item->base_info.state()
        && item->init
        && E_APP_MODULE_CFG_PERMISSION_ENABLE == item->base_info.permission())
    {
        log_d("init app modules : %s\n", item[id].base_info.name().c_str());
        item->init();
        item->base_info.set_state(E_APP_MODULE_INSTALLED);
    }
    pthread_mutex_unlock(&mutex_);
}

// 单个模块卸载
void AppModuleManager::finit_single_app_module(const uint32_t id, const std::string &name) noexcept
{
    if (id >= app_module_num_) { return ; }

    pthread_mutex_lock(&mutex_);
    struct app_module_config *item = &app_modules_[id];

    if (name == item->base_info.name()
        && item->base_info.enable()
        && E_APP_MODULE_INSTALLED == item->base_info.state()
        && item->exit
        && E_APP_MODULE_CFG_PERMISSION_ENABLE == item->base_info.permission())
    {
        log_d("finit app modules : %s\n", item[id].base_info.name().c_str());
        item->exit();
        item->base_info.set_state(E_APP_MODULE_IDLE);
    }
    pthread_mutex_unlock(&mutex_);
}

// 获取当前所有模块状态
void AppModuleManager::app_modules_current_status(AppModuleCoutInfoList &moduel_list) noexcept
{
    pthread_mutex_lock(&mutex_);
    struct app_module_config *item = app_modules_;

    for (uint32_t i = 0; i < app_module_num_; i++)
    {
        // 没有初始化接口的模块认为是无效的
        if (!item[i].init) { continue; }

        // 添加一个模块
        AppModuleCoutInfo *module = moduel_list.add_app_modules();
        AppModuleBaseInfo *info = module->mutable_base_info();

        module->set_id(i);
        module->set_run_state(item[i].status
                                ? (item[i].status() ? E_APP_MODULE_RUN_ST_OK : E_APP_MODULE_RUN_ST_ERR)
                                : E_APP_MODULE_RUN_ST_UNKNOWN);
        
        info->set_name(item[i].base_info.name());
        info->set_enable(item[i].base_info.enable());
        info->set_state(item[i].base_info.state());
        info->set_permission(item[i].base_info.permission());
    }
    pthread_mutex_unlock(&mutex_);

    return ;
}

// 获取模块信息
void app_modules_current_status(AppModuleCoutInfoList &moduel_list) noexcept
{
    AppModuleManager *app_module_manager = AppModuleManager::get_app_module_manager();

    app_module_manager->app_modules_current_status(moduel_list);
}

#ifdef DEBUG
// 打印模块信息
static void _print_app_module_state()
{
    AppModuleCoutInfoList moduel_list;
    app_modules_current_status(moduel_list);

    int size = moduel_list.app_modules_size();

    using namespace std;

    for (int i = 0; i < size; i++)
    {
        const AppModuleCoutInfo &module = moduel_list.app_modules(i);

        cout << "module id : " << module.id() << endl;
        cout << "run state : " << module.run_state() << endl;

        cout << "modules name : " << module.base_info().name() << endl;
        cout << "modules enable : " << module.base_info().enable() << endl;
        cout << "state : " << module.base_info().state() << endl;
        cout << "permission : " << module.base_info().permission() << endl;

        cout << "########################################" << endl;
    }
}
#endif

// 初始化单个模块
void init_single_app_module(const uint32_t id, const std::string &name) noexcept
{
    AppModuleManager *app_module_manager = AppModuleManager::get_app_module_manager();

    app_module_manager->init_single_app_module(id, name);
}

// 反初始化单个模块
void finit_single_app_module(const uint32_t id, const std::string &name) noexcept
{
    AppModuleManager *app_module_manager = AppModuleManager::get_app_module_manager();

    app_module_manager->finit_single_app_module(id, name);
}

AppModuleManager *AppModuleManager::app_module_manager_ = nullptr;

// 生成一个模块的基本信息
static AppModuleBaseInfo create_moudle_base(const char *name, const bool enable,
                                      const AppModuleState state, const AppModuleCfgPermission permission)
{
    AppModuleBaseInfo _module;

    _module.set_name(name);
    _module.set_enable(enable);
    _module.set_state(state);
    _module.set_permission(permission);

    return _module;
}

// 应用模块配置表
struct app_module_config AppModuleManager::app_modules_[OS_SYS_MAX_APP_MODULES] = {
    // 升级模块
    { create_moudle_base("system upgrade", true, E_APP_MODULE_IDLE, E_APP_MODULE_CFG_PERMISSION_DISENABLE),
        system_upgrade_task_init, nullptr, system_upgrade_task_state },
    
    // 用户管理模块
    { create_moudle_base("user manage", true, E_APP_MODULE_IDLE, E_APP_MODULE_CFG_PERMISSION_DISENABLE),
        user_manager_init, nullptr, user_manager_state },

    /*******************************************************************************************************************************************************/ 

    // more app modules

    /*******************************************************************************************************************************************************/ 

    END_OF_APP_MODULE
};

// sdk中间件注册
static void register_sdk_midwares(void)
{
	SDK_IMPORT_MIDWARE(user_manange_midware_do, "user_manange_midware", true);
	// TODO:记录访问者，ip，端口，时间(),json格式
}

// TODO:移到告警模块
void except_task_alarm(const struct except_task_info &except_task)
{
    // TODO:记录到异常任务日志，告警上报
}

// 基础功能模块初始化
static void base_function_module_init(void)
{
    wotsen::task_manage_init(get_max_task_num_capability(), reinterpret_cast<wotsen::abnormal_task_do>(except_task_alarm));

    task_handy_loop_init();

	// 初始化sdk网络
	sdk_net_init();

#ifdef DEBUG
    sdk_tree_map();
#endif

	// 注册sdk中间件
	register_sdk_midwares();
}

// 应用业务模块初始化
static void applications_module_init(void)
{
    AppModuleManager *app_module_manager = AppModuleManager::get_app_module_manager();

    // 私有功能模块与第三方模块初始化
    app_module_manager->app_modules_init();

#ifdef DEBUG
    _print_app_module_state();
#endif
}

// 用户应用初始化
void usr_apps_init(void)
{
    // 基础模块初始化
    base_function_module_init();

    // 应用模块初始化
    applications_module_init();
}
