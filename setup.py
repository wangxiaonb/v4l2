from distutils.core import setup,Extension

# MOD = 'Exten' #模块名
# setup(name=MOD,ext_modules=[Extension(MOD,sources=['./Exten.cpp'])])

setup(name="Exten",
        version="1.0.0",
        description="abc",
        author="abc",
        author_email="abc",
        ext_modules=[Extension(
            "Exten", # 注意这个地方要和模块初始化的函数名对应
            sources=["Exten.cpp"],
        #   include_dirs=["头文件目录，这里填当前目录即可"],
        #   libraries=['mathfunlib'],
        #   library_dirs=['库文件目录，这里填当前目录即可'],
            language='c++',
            extra_compile_args=['-std=c++11']
        )])


# MOD = 'Extest' #模块名
# setup(name=MOD,ext_modules=[Extension(MOD,sources=['my_extend.cpp'])]) #源文件名
# setup(name="Extest",
#         version="1.0.0",
#         description="abc",
#         author="abc",
#         author_email="abc",
#         ext_modules=[Extension(
#             "Extest", # 注意这个地方要和模块初始化的函数名对应
#             sources=["my_extend.cpp"],
#         #   include_dirs=["头文件目录，这里填当前目录即可"],
#         #   libraries=['mathfunlib'],
#         #   library_dirs=['库文件目录，这里填当前目录即可'],
#             language='c++',
#             extra_compile_args=['-std=c++11']
#         )])



# def main():
#     setup(name="fastaddition",
#           version="1.0.0",
#           description="abc",
#           author="abc",
#           author_email="abc",
#           ext_modules=[Extension(
#               "fastaddition", # 注意这个地方要和模块初始化的函数名对应
#               sources=["pyInterface.cpp"],
#             #   include_dirs=["头文件目录，这里填当前目录即可"],
#             #   libraries=['mathfunlib'],
#             #   library_dirs=['库文件目录，这里填当前目录即可'],
#               language='c++',
#               extra_compile_args=['-std=c++11']
#           )])


# if __name__ == "__main__":
#     main()