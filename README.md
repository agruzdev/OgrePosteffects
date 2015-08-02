# OgrePosteffects
A simple OGRE application with few posteffects implementation. Just for fun

## How to build

**Required tools**
* Cmake version 2.6 or above
* Microsoft Visual Studio 2013 or 2015 (tested only 2013 for now; 2015 - soon)

**Dependencies**
* OGRE v 1.9.0 - Can be downloaded as sources on the official OGRE website: http://www.ogre3d.org/download/source . Generate Visual Studio solution with the help of CMake and add the OGRE source directory to the environment variable "OGRE_HOME" (for example OGRE_HOME="C:/ogre_src_1_9_1/"). The builded VS solution should be in the "build" subdirectory (for example "C:/ogre_src_1_9_1/build/").
  If you dont want to build the engine, you can use prebuilt SDK from https://bitbucket.org/lezo/ogre_build/downloads Extract it to any folder and set the environment variable OGRE_HOME to root of the extracted folder
* Boost - required by OGRE. Set the environment variable "BOOST_ROOT" to the boost directory. Can be downloaded on the official site http://www.boost.org/users/download/ To buld boost use the command b2.exe toolset=msvc-12.0 variant=debug,release link=static
* Loki - C++ designes and patterns library by A. Alexandrescu. Can be found here http://loki-lib.sourceforge.net/ . Set the environment variable "LOKI_ROOT" to the Loki library directory. The Loki library has only headers, so it is not needed to be built

**Building**
* Open the created VS solution and build the project OgrePoseEffets or ALL_BUILD
* The project INSTALL will copy all dll's into the build directory
 
**Running**
* Run the application and choose OpenGL render system
* In the properties of the render system  is recommended to disable full screen
 
**Using**
* You can rotate the ogre head with nouse
* All posteffects can be enabled/disabled on the panel in the top left corner
* Press ESC to close the application

**License**

The MIT License (MIT)

Copyright (c) 2015 Alexey Gruzdev

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
