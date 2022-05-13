from setuptools import Extension, setup

aeron = Extension(
    name="aeron",
    sources=["src/aeron/aeronmodule.c"],
    libraries=["aeron"],
)

if __name__ == "__main__":
    setup(ext_modules=[aeron])
