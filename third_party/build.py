#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import re
from optparse import OptionParser
from collections import namedtuple


parser = OptionParser(usage="usage: %prog [options]")

parser.add_option("-d", "--debug", dest="debug", action="store_true", default=False,
                    help="debug模式,默认非debug模式,如果是debug版本：-d or --debug")

parser.add_option("-l", "--list", dest="list_module", action="store_true", default=False,
                    help="列出当前可用模块")

parser.add_option("-m", "--modules",
                  action="store", dest="modules", type="string", default="all",
                  help="选择编译安装模块，多个模块使用,隔开，默认编译安装所有模块，可先使用-l查看当前可用模块，选择模块时可以使用数字或名称")

parser.add_option("-i", "--install",
                  action="store", dest="install", type="string", default="./",
                  help="模块的安装路径，默认模块当前路径，请使用绝对路径")

(options, args) = parser.parse_args()


ThirdPartyModule = namedtuple("ThirdPartyModules", ['build', 'module_path', 'description'])


def SQLiteCpp_build(source_dir, debug, install_path):
    cur_dir = os.getcwd()

    os.chdir(source_dir)
    os.system("mkdir build -p")
    os.chdir('build')
    os.system("cmake ../ -DCMAKE_INSTALL_PREFIX=%s" % install_path)
    os.system("make")
    os.system("make install")

    os.system("cp sqlite3/libsqlite3.a %s/lib/" % install_path)
    os.system("cp ../sqlite3/sqlite3.h %s/include/SQLiteCpp/" % install_path)

    os.chdir(cur_dir)


def c_log_build(source_dir, debug, install_path):
    cur_dir = os.getcwd()

    os.chdir(source_dir)
    os.system("make")
    os.system("make MAKE_INSTALL_PREFIX=%s install" % install_path)

    os.chdir(cur_dir)


def c_time_build(source_dir, debug, install_path):
    cur_dir = os.getcwd()

    os.chdir(source_dir)
    os.system("make %s" % ("DEBUG=true" if debug else ""))
    os.system("make MAKE_INSTALL_PREFIX=%s install" % install_path)

    os.chdir(cur_dir)


def json_build(source_dir, debug, install_path):
    cur_dir = os.getcwd()

    os.chdir(source_dir)
    os.system("mkdir %s/include/ -p" % install_path)
    os.system("cp single_include/* %s/include -rf" % install_path)

    os.chdir(cur_dir)


# def libuv_build(source_dir, debug, install_path):
#     cur_dir = os.getcwd()

#     os.chdir(source_dir)
#     os.system("./autogen.sh")
#     os.system("./configure --prefix=%s --enable-shared=no" % install_path)
#     os.system("make")
#     os.system("make install")

#     os.chdir(cur_dir)


def protobuf_build(source_dir, debug, install_path):
    cur_dir = os.getcwd()

    os.chdir(source_dir)
    os.system("./autogen.sh")
    os.system("./configure --prefix=%s --enable-shared=no" % install_path)
    os.system("make")
    os.system("make install")

    os.chdir("python")
    os.system("python3 setup.py build")
    os.system("sudo python3 setup.py install")

    os.chdir(cur_dir)


def sole_build(source_dir, debug, install_path):
    cur_dir = os.getcwd()

    os.chdir(source_dir)
    os.system("mkdir %s/include/sole/ -p" % install_path)
    os.system("cp sole/sole.hpp %s/include/sole/ -f" % install_path)

    os.chdir(cur_dir)


def util_pthread_build(source_dir, debug, install_path):
    cur_dir = os.getcwd()

    os.chdir(source_dir)
    os.system("make")
    os.system("make MAKE_INSTALL_PREFIX=%s install" % install_path)

    os.chdir(cur_dir)


def linux_systemcmd_proxy_build(source_dir, debug, install_path):
    cur_dir = os.getcwd()

    os.chdir(source_dir)
    os.system("make %s" % ("DEBUG=true" if debug else ""))
    os.system("make MAKE_INSTALL_PREFIX=%s install" % install_path)

    os.chdir(cur_dir)


def fmt_build(source_dir, debug, install_path):
    cur_dir = os.getcwd()

    os.chdir(source_dir)
    os.system("mkdir build -p")
    os.chdir('build')
    os.system("cmake ../ -DCMAKE_INSTALL_PREFIX=%s" % install_path)
    os.system("make")
    os.system("make install")

    os.chdir(cur_dir)


def handy_build(source_dir, debug, install_path):
    cur_dir = os.getcwd()

    os.chdir(source_dir)
    os.system("make")
    os.system("make MAKE_INSTALL_PREFIX=%s install" % install_path)

    os.chdir(cur_dir)


# 模块信息
third_party_modules = [
    ThirdPartyModule(SQLiteCpp_build, "SQLiteCpp", "c++封装的sqlite3"),
    ThirdPartyModule(c_log_build, "c-log", "c语言实现的日志,easyLogger"),
    ThirdPartyModule(c_time_build, "c-time", "c语言封装的时间函数接口"),
    ThirdPartyModule(json_build, "json", "c++很直观的json库"),
    # ThirdPartyModule(libuv_build, "libuv", "libuv库"),
    ThirdPartyModule(protobuf_build, "protobuf", "google protobuf协议开源库"),
    ThirdPartyModule(sole_build, "sole", "简单uuid库"),
    ThirdPartyModule(util_pthread_build, "util_pthread", "pthread线程简单接口封装"),
    ThirdPartyModule(linux_systemcmd_proxy_build, "linux-systemcmd-proxy", "linux下system接口的代理库"),
    ThirdPartyModule(fmt_build, "fmt", "c++更好的格式话输出，类似python format"),
    ThirdPartyModule(handy_build, "handy", "c++异步网络库"),
]

# 模块索引列表
third_party_modules_no = [str(i) for i in range(len(third_party_modules))]
# 资源目录列表
third_party_modules_source = [item.module_path for item in third_party_modules]


def list_modules():
    """列出可用模块"""
    for i, item in enumerate(third_party_modules):
        print("%d : %s" % (i, item.module_path + " --> " + item.description))


def build_third_party_moduels(build_modules, debug, install_path):
    """开始构建"""
    for item in build_modules:
        print("###############start build : %s########################" % item)
        third_party_modules[third_party_modules_source.index(item)].build(item, debug, install_path)
        print("###############build success : %s########################" % item)
        print()


def parse_build_modules(inputs):
    """解析需要构建的模块"""
    # 编译所有模块
    if inputs == "all":
        build_modules = [item.module_path for item in third_party_modules]
    else:
        build_modules = list()
        # 去除空白字符
        input_modules = re.sub(r'\s', r'', inputs)
        # 分割模块名
        input_modules = input_modules.split(',')

        # 数字，模块名转换到模块目录名
        for item in input_modules:
            # 名称列表
            if item in third_party_modules_source:
                build_modules.append(item)
            # 编号列表
            elif item in third_party_modules_no:
                build_modules.append(third_party_modules[third_party_modules_no.index(item)].module_path)
            # 无效模块
            else:
                print("no such module : ", item)
                exit(-1)

    # 检查模块资源有效性
    for item in build_modules:
        if not os.path.exists(item) or not os.path.isdir(item):
            print("module %s source error" % item)
            exit(-1)

    return build_modules


def main():
    # 列出可用模块，是单独的执行路径
    if options.list_module:
        list_modules()
        return

    # 检查安装路径
    if '/' != options.install[0] or not os.path.exists(options.install) or not os.path.isdir(options.install):
        print("install path : %s not exit, use absolute path" % options.install)
        return 

    # 构建
    build_third_party_moduels(parse_build_modules(options.modules), options.debug, options.install)

    print("build third party over!")


if __name__ == "__main__":
    main()
