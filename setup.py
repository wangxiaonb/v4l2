from distutils.core import setup, Extension
import pkgconfig


# MOD = 'Exten' #模块名
# setup(name=MOD,ext_modules=[Extension(MOD,sources=['./Exten.cpp'])])

ela = pkgconfig.libs('opencv') + ' ' + pkgconfig.cflags('opencv')
ela = ela.split()

setup(name="v4l2",
      version="1.0.0",
      description="v4l2 python driver",
      author="ssjhs",
      author_email="wangxiaonb@126.com",
      ext_modules=[Extension(
          "v4l2",  # 注意这个地方要和模块初始化的函数名对应
          sources=["pyv4l2.cpp", "v4l2.cpp"],
          extra_link_args=ela
          #   include_dirs=["头文件目录，这里填当前目录即可"],
          # libraries=['pthread'],
          # extra_objects = ['`pkg-config --cflags --libs opencv`'],
          # library_dirs = ['/usr/local/include/opencv2/contrib','/usr/local/include/opencv2/core','/usr/local/include/opencv2/highgui','/usr/local/include/opencv2/imgproc','/usr/local/include'],
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
