centralpark
===========

A C large array memory manager library

This memory manager is intended for applications which 
use large amounts of equally-sized memory chunks with high performance,
for example, board game AIs which use state trees with a lot of nodes.
(The original use case for this library was, in fact, a contest-winning AI).

Valgrind memory checking can be enabled via -DWITH_VG_MEMCHECK.

Usage documentation for all functions can be found within the header.
