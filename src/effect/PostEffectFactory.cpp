/**
* @file PostEffectFactory.cpp
*
* Copyright (c) 2015 by Gruzdev Alexey
*
* Code covered by the MIT License
* The authors make no representations about the suitability of this software
* for any purpose. It is provided "as is" without express or implied warranty.
*/

#include "PostEffectFactory.h"

namespace OgreEffect
{

    PostEffectFactory::PostEffectFactory(const Ogre::String& name) :
        mName(name)
    {
    }
    //-------------------------------------------------------
    PostEffectFactory::~PostEffectFactory()
    {
    }
    //-------------------------------------------------------

}//namespace OgreEffect