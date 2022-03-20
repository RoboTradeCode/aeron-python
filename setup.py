from distutils.core import setup, Extension

aeron = Extension('aeron',
                  include_dirs=['libs/aeron/aeron-client/src/main/c'],
                  libraries=['aeron'],
                  library_dirs=['/usr/local/lib'],
                  sources=['aeronmodule.c'])

setup(name='aeron',
      version='0.1',
      description='Efficient reliable UDP unicast, UDP multicast, and IPC message transport',
      author='nomnoms12',
      author_email='alexander.ign0918@gmail.com',
      url='https://github.com/real-logic/aeron',
      ext_modules=[aeron])
