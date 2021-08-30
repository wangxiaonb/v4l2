from distutils.core import setup, Extension

# MOD = 'Exten' #模块名
# setup(name=MOD,ext_modules=[Extension(MOD,sources=['./Exten.cpp'])])

setup(name="v4l2",
      version="1.0.0",
      description="v4l2 python driver",
      author="ssjhs",
      author_email="wangxiaonb@126.com",
      ext_modules=[Extension(
          "v4l2",  # 注意这个地方要和模块初始化的函数名对应
          sources=["pyv4l2.cpp", "v4l2.cpp"],
          #   include_dirs=["头文件目录，这里填当前目录即可"],
          #   libraries=['v4l2'],
          #   library_dirs=['库文件目录，这里填当前目录即可'],
        #   language='c++',
        #   extra_compile_args=['-std=c++11']
      )])


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