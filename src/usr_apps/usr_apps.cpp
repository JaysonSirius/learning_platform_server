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
#include "task_manage/task_manage.h"
#include "uv_event/uv_event.h"
#include "sdk_net/sdk_network/sdk_network.h"

// 应用模块导入
/**************************************************************************************/

/* 私有模块 */
#include "upgrade/upgrade.h"
#include "user_manage/user_manage.h"

/* 三方模块 */

/**************************************************************************************/

#include "os_param.h"
#include "usr_apps.h"

class AppModuleManager;
struct app_module_config;

using app_module_fun = void (*)(void);
using app_module_status_fun = bool (*)(void);

/**
 * @brief 模块配置信息
 * 
 */
struct app_module_config {
    struct app_module_base_info base_info;  ///< 模块基本配置信息
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
        static AppModuleManager *app_module_manager;
        static struct app_module_config app_modules[OS_SYS_MAX_APP_MODULES];
        uint32_t app_module_num;
        pthread_mutex_t mutex;
    public:
        AppModuleManager() : app_module_num(0) {
            pthread_mutex_init(&mutex, NULL);
            for (int i = 0; i < OS_SYS_MAX_APP_MODULES; i++)
            {
                if (app_modules[i].init)
                {
                    app_module_num++;
                }
            }

            log_i("init app modules [%d]\n", app_module_num);
        }

        ~AppModuleManager() {
            pthread_mutex_destroy(&mutex);
        }

        static AppModuleManager *get_app_module_manager(void)
        {
            if (!app_module_manager)
            {
                app_module_manager = new AppModuleManager();
            }

            return app_module_manager;
        }

        // 模块初始化
        void app_modules_init(void) noexcept {
            pthread_mutex_lock(&mutex);

            struct app_module_config *item = app_modules;

            for (int i = 0; i < (int)app_module_num; i++)
            {
                if (item[i].base_info.enable && E_APP_MODULE_IDLE == item[i].base_info.state && item[i].init)
                {
                    log_i("init app modules : %s\n", item[i].base_info.name.c_str());
                    item[i].init();
                    item[i].base_info.state = E_APP_MODULE_INSTALLED;
                }
            }
            pthread_mutex_unlock(&mutex);
        }

        // 单个模块初始化
        void init_single_app_module(const uint32_t id, const std::string &name) noexcept
        {
            if (id >= app_module_num) { return ; }

            pthread_mutex_lock(&mutex);
            struct app_module_config *item = &app_modules[id];

            if (name == item->base_info.name
                && item->base_info.enable
                && E_APP_MODULE_IDLE == item->base_info.state
                && item->init
                && E_APP_MODULE_CFG_PERMISSION_ENABLE == item->base_info.permission)
            {
                log_i("init app modules : %s\n", item[id].base_info.name.c_str());
                item->init();
                item->base_info.state = E_APP_MODULE_INSTALLED;
            }
            pthread_mutex_unlock(&mutex);
        }

        // 单个模块卸载
        void finit_single_app_module(const uint32_t id, const std::string &name) noexcept
        {
            if (id >= app_module_num) { return ; }

            pthread_mutex_lock(&mutex);
            struct app_module_config *item = &app_modules[id];

            if (name == item->base_info.name
                && item->base_info.enable
                && E_APP_MODULE_INSTALLED == item->base_info.state
                && item->exit
                && E_APP_MODULE_CFG_PERMISSION_ENABLE == item->base_info.permission)
            {
                log_i("finit app modules : %s\n", item[id].base_info.name.c_str());
                item->exit();
                item->base_info.state = E_APP_MODULE_IDLE;
            }
            pthread_mutex_unlock(&mutex);
        }

        // 获取当前所有模块状态
        std::vector<struct app_module_cout_info> &&app_modules_current_status(void) noexcept {
            std::vector<struct app_module_cout_info> module_list;

            pthread_mutex_lock(&mutex);
            struct app_module_config *item = app_modules;

            for (uint32_t i = 0; i < app_module_num; i++)
            {
                struct app_module_cout_info info = {
                    i,
                    item[i].status ? (item[i].status() ? E_APP_MODULE_RUN_ST_OK : E_APP_MODULE_RUN_ST_ERR) : E_APP_MODULE_RUN_ST_UNKNOWN,
                    {
                        item[i].base_info.name,
                        item[i].base_info.enable,
                        item[i].base_info.state,
                        item[i].base_info.permission
                    }
                };

                module_list.push_back(info);
            }
            pthread_mutex_unlock(&mutex);

            return std::move(module_list);
        }
};

// 获取模块信息
std::vector<struct app_module_cout_info> &&app_modules_current_status(void) noexcept
{
    AppModuleManager *app_module_manager = AppModuleManager::get_app_module_manager();

    return app_module_manager->app_modules_current_status();
}

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

static bool enable = true;
static bool disable = false;

AppModuleManager *AppModuleManager::app_module_manager = nullptr;

#define END_OF_APP_MODULE { {"", disable, E_APP_MODULE_BAD, E_APP_MODULE_CFG_PERMISSION_DISENABLE}, NULL, NULL, NULL }

// 应用模块配置表
struct app_module_config AppModuleManager::app_modules[OS_SYS_MAX_APP_MODULES] = {
    // 升级模块
    { { "system upgrade",      enable,  E_APP_MODULE_IDLE,     E_APP_MODULE_CFG_PERMISSION_DISENABLE },     system_upgrade_task_init,      NULL, system_upgrade_task_state},
    // 用户管理模块
    { { "user manage",         enable,  E_APP_MODULE_IDLE,     E_APP_MODULE_CFG_PERMISSION_DISENABLE },     user_manager_init,      NULL, user_manager_state},

    /*******************************************************************************************************************************************************/ 

    // more app modules

    /*******************************************************************************************************************************************************/ 

    END_OF_APP_MODULE
};

// sdk中间件
static void register_sdk_midwares(void)
{
	SDK_IMPORT_MIDWARE(user_manange_midware_do, true);
	// TODO:记录访问者，ip，端口，时间(),json格式
}

// TODO:移到告警模块
void except_task_alarm(const struct except_task_info &except_task)
{
    // TODO:记录到异常任务日志，告警上报
}

static void base_function_module_init(void)
{
    wotsen::task_manage_init(OS_SYS_TASK_NUM, reinterpret_cast<wotsen::abnormal_task_do>(except_task_alarm));

	// 初始化uv任务
	task_uv_event_init();

	// 初始化sdk网络
	sdk_uv_net_init();

	// 注册sdk中间件
	register_sdk_midwares();
}

static void applications_module_init(void)
{
    AppModuleManager *app_module_manager = AppModuleManager::get_app_module_manager();

    // 私有功能模块与第三方模块初始化
    app_module_manager->app_modules_init();

    // 告警
    // 升级，文件传输校验以及如何安装，设计扩展
    // 图像上传下载、图像信息、搜索
    // ai模型加载及运行，级联python
}

void usr_apps_init(void)
{
    // 基础模块初始化
    base_function_module_init();

    // 应用模块初始化
    applications_module_init();
}
