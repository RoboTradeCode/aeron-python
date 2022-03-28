import setuptools

aeron = setuptools.Extension(
    name='aeron',
    include_dirs=['libs/aeron/aeron-client/src/main/c'],
    libraries=['aeron'],
    library_dirs=['/usr/local/lib'],
    sources=['aeronmodule.c'],
)

with open('README.md', encoding='utf-8') as fh:
    long_description = fh.read()

setuptools.setup(
    name='aeron',
    version='0.1.0',
    author='nomnoms12',
    author_email='alexander.ign0918@gmail.com',
    description='Efficient reliable UDP unicast, UDP multicast, and IPC message transport',
    long_description=long_description,
    long_description_content_type='text/markdown',
    url='https://github.com/RoboTradeCode/aeron-python',
    project_urls={
        'Bug Tracker': 'https://github.com/RoboTradeCode/aeron-python/issues',
    },
    classifiers=[
        'Operating System :: POSIX :: Linux',
        'Programming Language :: C',
        'Programming Language :: Python :: Implementation :: CPython',
    ],
    ext_modules=[aeron],
)
