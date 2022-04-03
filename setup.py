import subprocess
from pathlib import Path
from setuptools import Command, Extension, setup, find_packages
from setuptools.command.build_ext import build_ext


class CMakeBuild(build_ext, Command):
    def run(self):
        Path("build").mkdir(exist_ok=True)
        source_dir = Path.cwd()
        build_dir = source_dir / "build"

        subprocess.check_call(["cmake", source_dir], cwd=build_dir)
        subprocess.check_call(["cmake", "--build", "."], cwd=build_dir)
        subprocess.check_call(["cmake", "--install", "."], cwd=build_dir)

        super().run()


aeron = Extension(
    name="aeron",
    sources=["src/aeron/aeronmodule.c"],
    include_dirs=["build/include/aeron/c"],
    extra_objects=["build/lib/libaeron_static.a"],
)

if __name__ == "__main__":
    setup(ext_modules=[aeron], cmdclass={"build_ext": CMakeBuild})
