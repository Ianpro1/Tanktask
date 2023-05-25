from glob import glob
from setuptools import setup
from pybind11.setup_helpers import Pybind11Extension

ext_modules = [
    Pybind11Extension(
        "Tanktask",
        sorted(glob("src/*.cpp")),  # Sort source files for reproducibility
        include_dirs=['include']
    ),
]

setup(
    name="Tanktask",
    version="0.0.1",
    author="_ianmi",
    description="",
    long_description="",
    ext_modules=ext_modules,
    python_requires=">=3.7",
    install_requires=['numpy'],
)