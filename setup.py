from distutils.core import setup, Extension
# import pkgconfig

# ela = pkgconfig.libs('opencv') + ' ' + pkgconfig.cflags('opencv')
# ela = ela.split()

module_name = "v4l2"

setup(name=module_name,
      version="1.0.0",
      description=module_name+" python driver",
      author="ssjhs",
      author_email="wangxiaonb@126.com",
      ext_modules=[Extension(
          module_name,  # 注意这个地方要和模块初始化的函数名对应
          sources=["pyv4l2.cpp", "v4l2.cpp"],
          # extra_link_args=ela
          #   include_dirs=["头文件目录，这里填当前目录即可"],
          # libraries=['pthread'],
          # extra_objects = ['`pkg-config --cflags --libs opencv`'],
          # library_dirs = ['/usr/local/include/opencv2/contrib','/usr/local/include/opencv2/core','/usr/local/include/opencv2/highgui','/usr/local/include/opencv2/imgproc','/usr/local/include'],
          #   library_dirs=['库文件目录，这里填当前目录即可'],
          #   language='c++',
          #   extra_compile_args=['-std=c++11']
      )])
