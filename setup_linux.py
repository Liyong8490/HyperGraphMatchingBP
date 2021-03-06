#!/usr/bin/env python

from distutils.core import setup, Extension
import numpy


FactorGraph_module = Extension('FactorBP._FactorGraph',
                            sources=['FactorBP/FactorGraph.i','FactorBP/FactorGraph.cpp', 'FactorBP/FactorCreator.cpp', 'FactorBP/Factor.cpp', 'FactorBP/Auction.cpp', 'FactorBP/SubTourFactor.cpp', 'FactorBP/MDiverseSolver.cpp'],
                            swig_opts=['-modern','-c++','-I./FactorBP'],
                            extra_compile_args=['-O3','-DPERFECT_MATCHING_DOUBLE', '-std=c++11','-DPERFECT_MATCHING_DOUBLE', '-ffast-math', '-pipe', '-fomit-frame-pointer', '-std=c++11'],
                            include_dirs=[numpy.get_include()],
                           )
                            #extra_compile_args=['-I./FactorBP','-O3', '-I/usr/local/include','-DPERFECT_MATCHING_DOUBLE', '-ffast-math', '-pipe', '-fomit-frame-pointer', '-std=c++11', '-stdlib=libc++'],

setup (name = 'FactorBP',
       version = '0.1',
       author      = "Zhen Zhang",
       author_email='zhen@zzhang.org',
       description = """Python module for Factor based belief propagation""",
       packages=['FactorBP'],
       package_dir={'FactorBP':'FactorBP/'},
       ext_modules = [FactorGraph_module],
       py_modules = ["FactorGraph"],
       )
