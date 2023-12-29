.. README.rst

xcp-revisited
=============

Exercises from Peter van der Linden's *Expert C Programming*.

A revisiting of a book I'd purchased some time ago but had never gotten around
to really reading through.

About
-----

TBD. Although the book is somewhat dated, it's fun to read and still useful. In
fact, I actually started reading and doing the book's programming exercises
during my 2023 year-end vacation, so at least to me there is both educational
and entertainment value to be found. Can't say no to having fun while learning.

Building from source
--------------------

Make
~~~~

For Make_, one can build a debugging, i.e. no optimization build, with

.. code:: bash

   make

To build with optimization, one can specify ``CONFIG=Release``, e.g.

.. code:: bash

   make CONFIG=Release

To enable AddressSanitizer_ and specify a different compiler, in this case
Clang_, one can use

.. code:: bash

   make CC=clang ENABLE_ASAN=1

.. _Make: https://www.gnu.org/software/make/

.. _CMake: https://cmake.org/cmake/help/latest/

.. _AddressSanitizer: https://github.com/google/sanitizers/wiki/AddressSanitizer

.. _Clang: https://clang.llvm.org/

CMake
~~~~~

TBD. Currently one can build a debugging build with

.. code:: bash

   cmake -S . -B build && cmake --build build -j

To build a release build, use

.. code:: bash

   cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build -j

To enable AddressSanitizer and specify a different compiler, one can use

.. code:: bash

   cmake -S . -B build -DCMAKE_C_COMPILER=clang -DENABLE_ASAN=1 &&
   cmake --build build -j
