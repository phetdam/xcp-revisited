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

.. code:: shell

   make

To build with optimization, one can specify ``CONFIG=Release``, e.g.

.. code:: shell

   make CONFIG=Release

To enable AddressSanitizer_ and specify a different compiler, in this case
Clang_, one can use

.. code:: shell

   make CC=clang ENABLE_ASAN=1

To build the project's libraries as static instead of shared, one can use

.. code:: shell

   make BUILD_SHARED=

``BUILD_SHARED`` is set by default and results in shared libraries being built.

By default, if a C++ compiler is available and if `Google Test`_ >=1.10.0 is
locatable via pkg-config_, unit tests will also be built. If one of these
components is missing, no tests will be built. One can also disable test
building with

.. code:: shell

   make BUILD_TESTS=

.. _Make: https://www.gnu.org/software/make/

.. _CMake: https://cmake.org/cmake/help/latest/

.. _AddressSanitizer: https://github.com/google/sanitizers/wiki/AddressSanitizer

.. _Clang: https://clang.llvm.org/

.. _Google Test: https://github.com/google/googletest

.. _pkg-config: https://www.freedesktop.org/wiki/Software/pkg-config/

CMake
~~~~~

Using the provided ``build.sh`` script, one can build a debugging build with

.. code:: shell

   ./build.sh

To build a release build, use

.. code:: shell

   ./build.sh -c Release

To enable AddressSanitizer and specify a different C compiler, one can use

.. code:: shell

   ./build.sh -Ca -DCMAKE_C_COMPILER=clang -DENABLE_ASAN=ON

To build the project's libraries as static instead of shared, one can use

.. code:: shell

   ./build.sh -Ca -DBUILD_SHARED_LIBS=ON

``BUILD_SHARED_LIBS`` is set by default and results in shared libraries being
built.

If a C++ compiler and Google Test >=1.10.0 are available, unit tests will be
built by default. If one of these components is missing, no tests will be built.
One can also explicitly choose to disable test building with

.. code:: shell

   ./build.sh -Ca -DBUILD_TESTS=OFF
