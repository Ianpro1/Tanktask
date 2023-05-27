from glob import glob
import os, sys
from setuptools import setup

try:
    from pybind11.setup_helpers import Pybind11Extension
except ImportError:
    from setuptools import Extension as Pybind11Extension


cpp_files = glob("TankTrouble/*.cpp")
src_files = sorted(cpp_files)


ext_modules = [
    Pybind11Extension(
        "Tanktask",
        src_files,  # Sort source files for reproducibility
        include_dirs=['TankTrouble', 'extern/Box2D/include/box2d', 'extern/SDL2/SDL2_image-2.6.3/include', 'extern/SDL2/SDL2-2.26.5/include', 'pybind11/include/pybind11'],
        library_dirs=['extern/Box2D/lib', 'extern/SDL2/SDL2_image-2.6.3/lib/x64', 'extern/SDL2/SDL2-2.26.5/lib/x64'],
        libraries=['SDL2', 'SDL2main', 'SDL2_image', 'box2d'],
        extra_link_args = ['/NODEFAULTLIB:library', '/LIBPATH:TankTrouble/runtime'],
        depends = ['TankTrouble/assets']
    ),
]

extra_link_args = ['/NODEFAULTLIB:library']

setup(
    name="Tanktask",
    version="0.0.1",
    author="_ianmi",
    description="",
    long_description="",
    ext_modules=ext_modules,
    python_requires=">=3.7",
    setup_requires=["pybind11"],
    install_requires=['numpy'],
    extra_link_args = extra_link_args,
)