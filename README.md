# OgrePosteffects
A simple OGRE application with few posteffects implementation. Just for fun

## How to build

Use CMake version above 2.6 to build the project
(Tested only on Visual Studio 2013 for now)

**Dependencies**
* OGRE v 1.9.1 - Can be downloaded on the official OGRE website: http://www.ogre3d.org/download/source . Generate Visual Studio solution with the help of CMake and add the OGRE source directory to the environment variable "OGRE_HOME" (for example OGRE_HOME="C:/ogre_src_1_9_1/"). The builded VS solution should be in the "build" subdirectory (for example "C:/ogre_src_1_9_1/build/").
* Boost - required by OGRE. Set the environment variable "BOOST_ROOT" to the boost directory
* Loki - C++ designes and patterns library by A. Alexandrescu. Can be found here http://loki-lib.sourceforge.net/ . Set the environment variable "LOKI_ROOT" to the Loki library directory. The Loki library has only headers, so it is not needed to be built
