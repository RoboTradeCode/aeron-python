from setuptools import Extension, setup

aeron = Extension(
    name="aeron",
    sources=["src/aeron/aeronmodule.c"],
)

if __name__ == "__main__":
    setup(ext_modules=[aeron])
