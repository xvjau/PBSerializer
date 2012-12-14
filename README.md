PBSerializer
============

Protobuf serializer using boost's property tree to use JSON, XML, etc as alternate serialization methods for PB.

Dependencies
------------

* Protobuf (http://code.google.com/p/protobuf/)
* Boost Property Tree (http://www.boost.org/)

Build
-----

You can build PBSerializer using CMake following these steps:

1. Create a build dir in the root dir of the project
    `mkdir build`
2. Inside the build dir, run CMake:
    `cmake ..`
3. Finally, run make:
    `make`

This process will build both library and test suite.
It is always a good idea to run the test suite before installing.

To run the test suie, inside the build dir, go to the test dir:
    `cd tests`
And execute the *tests* program:
    `./tests`

You can finally install the library running `make install`
inside the build dir, *not the tests dir*
