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
and entertainment value to be found.

Building from source
--------------------

TBD. Currently requires Make_ although adding a CMake_ config is a must.

For Make, one can build a debugging, i.e. no optimization build, with

.. code:: bash

   make

To build with optimization, one can specify ``CONFIG=Release``, e.g.

.. code:: bash

   make CONFIG=Release

Other variables can be used to control the build, e.g. to enable
AddressSanitizer_ and specify a different compiler, in this case Clang_, one
can use

.. code:: bash

   make CC=clang ENABLE_ASAN=1


.. _Make: https://www.gnu.org/software/make/

.. _CMake: https://cmake.org/cmake/help/latest/

.. _AddressSanitizer: https://github.com/google/sanitizers/wiki/AddressSanitizer

.. _Clang: https://clang.llvm.org/
