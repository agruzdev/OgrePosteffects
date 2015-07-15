/**
* @file PostEffectManager.cpp
*
* Copyright (c) 2015 by Gruzdev Alexey
*
* Code covered by the MIT License
* The authors make no representations about the suitability of this software
* for any purpose. It is provided "as is" without express or implied warranty.
*/

#include <assert.h>

#include <OgreCompositorChain.h>
#include <OgreCompositorManager.h>

#include "PostEffect.h"
#include "PostEffectManager.h"
#include "PostEffectFactory.h"

namespace OgreEffect
{

    DECLARE_REGISTRATION_FUNCTION(EffectNull)
    DECLARE_REGISTRATION_FUNCTION(EffectFading)
    DECLARE_REGISTRATION_FUNCTION(EffectBlur)

    //-------------------------------------------------------
    void PostEffectManager::RegisterDefaultFactories()
    {
        INVOKE_REGISTRATION_FUNCTION(EffectNull);
        INVOKE_REGISTRATION_FUNCTION(EffectFading);
        INVOKE_REGISTRATION_FUNCTION(EffectBlur);
    }
    //-------------------------------------------------------
    const Ogre::String PostEffectManager::PE_NULL = "PostEffect/Null";
    const Ogre::String PostEffectManager::PE_FADING = "PostEffect/Fading";
    const Ogre::String PostEffectManager::PE_BLUR = "PostEffect/Blur";
    //-------------------------------------------------------
    PostEffectManager* PostEffectManager::getSingletonPtr(void)
    {
        return &sPostEffectsManager::Instance();
    }
    //-------------------------------------------------------
    PostEffectManager& PostEffectManager::getSingleton(void)
    {
        return sPostEffectsManager::Instance();
    }
    //-------------------------------------------------------
    PostEffectManager::PostEffectManager()
    {
        if (true == mFactories.empty())
        {
            RegisterDefaultFactories();
        }
    }
    //-------------------------------------------------------
    PostEffectManager::~PostEffectManager()
    {

    }
    //-------------------------------------------------------
    void PostEffectManager::RegisterPostEffectFactory(Ogre::SharedPtr<PostEffectFactory> factory)
    {
        assert(nullptr != factory.get());
        Ogre::String name = factory->GetEffectName();
        auto factIt = mFactories.find(name);
        if (factIt == mFactories.cend())
        {
            mFactories.emplace(std::make_pair(name, factory));
        }
    }
    //-------------------------------------------------------
    void PostEffectManager::UnregisterPostEffectFactory(Ogre::SharedPtr<PostEffectFactory> factory)
    {
        assert(nullptr != factory.get());
        Ogre::String name = factory->GetEffectName();
        auto factIt = mFactories.find(name);
        if (factIt != mFactories.end())
        {
            mFactories.erase(factIt);
        }
    }
    //-------------------------------------------------------
    PostEffect* PostEffectManager::CreatePostEffect(const Ogre::String & effectType, Ogre::RenderWindow* window, Ogre::Viewport* viewport)
    {
        PostEffect* effect = nullptr;
        auto factIt = mFactories.find(effectType);
        if (factIt != mFactories.cend())
        {
            assert(nullptr != factIt->second.get());
            effect = factIt->second->Create();

            Ogre::CompositorChain* chain = Ogre::CompositorManager::getSingleton().getCompositorChain(viewport);
            if (chain->getNumCompositors() > 0)
            {
                throw std::logic_error("PostEffectManager[CreatePostEffect]: The viewport has already compositors");
            }

            effect->Prepare(window, chain);
        }
        return effect;
    }

}//namespace OgreEffect