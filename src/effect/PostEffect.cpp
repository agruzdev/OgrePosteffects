/**
* @file PostEffect.cpp
*
* Copyright (c) 2015 by Gruzdev Alexey
*
* Code covered by the MIT License
* The authors make no representations about the suitability of this software
* for any purpose. It is provided "as is" without express or implied warranty.
*/

#include <string>

#include "PostEffect.h"

#include <OgreCompositorManager.h>
#include <OgreRenderWindow.h>
#include <OgreCompositionTechnique.h>
#include <OgreCompositionTargetPass.h>
#include <OgreCompositionPass.h>
#include <OgreCompositorChain.h>
#include <OgreTechnique.h>
#include <OgreMaterial.h>
#include <OgrePass.h>
#include <OgreTimer.h>

namespace OgreEffect
{

    const Ogre::String PostEffect::DICTIONARY_NAME = "PostEffect";
    //-------------------------------------------------------
    void PostEffect::CreateParametersDictionary()
    {
        if (createParamDictionary(DICTIONARY_NAME))
        {
            Ogre::ParamDictionary* dictionary = getParamDictionary();
            if (nullptr != dictionary)
            {
                DoCreateParametersDictionary(dictionary);
            }
        }
    }
    //-------------------------------------------------------
    PostEffect::PostEffect(const Ogre::String & name, size_t id) :
        mName(name), mId(id)
    {
        mTimer = Ogre::Root::getSingleton().getTimer();
    }
    //-------------------------------------------------------
    PostEffect::~PostEffect()
    {
        //empty
    }
    //-------------------------------------------------------
    Ogre::Real PostEffect::GetTimeInSeconds() const
    {
        return static_cast<Ogre::Real>(mTimer->getMilliseconds() / 1000.0);
    }
    //-------------------------------------------------------
    Ogre::String PostEffect::GetUniquePostfix() const
    {
        return "PostEffect/" + mName + "/" + std::to_string(mId);
    }
    //-------------------------------------------------------
    bool PostEffect::CreateCompositor(Ogre::Material* material, Ogre::CompositorChain* chain)
    {
        mCompositor = Ogre::CompositorManager::getSingleton().create("Compositor/" + GetUniquePostfix(),
            Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
        Ogre::CompositionTechnique* technique = mCompositor->createTechnique();

        {
            Ogre::CompositionTechnique::TextureDefinition* textureBg = technique->createTextureDefinition(mSceneRtName);
            textureBg->name = mSceneRtName;
            textureBg->scope = Ogre::CompositionTechnique::TS_GLOBAL;
            textureBg->width = mRenderWindow->getWidth(); //same as render window
            textureBg->height = mRenderWindow->getHeight(); //same as render window
            textureBg->formatList.push_back(Ogre::PixelFormat::PF_R8G8B8A8);
        }

        {
            Ogre::CompositionTargetPass* target = technique->createTargetPass();
            target->setInputMode(Ogre::CompositionTargetPass::IM_PREVIOUS);
            target->setOutputName(mSceneRtName);
        }
        {
            Ogre::CompositionTargetPass* target = technique->getOutputTargetPass();
            target->setInputMode(Ogre::CompositionTargetPass::IM_NONE);
            Ogre::CompositionPass* pass = target->createPass();

            //pass->setInput(0, mSceneRtName);
            Ogre::Pass* matPass = material->getBestTechnique()->getPass(0);
            auto it = matPass->getTextureUnitStateIterator();
            for (size_t idx = 0; true == it.hasMoreElements(); ++idx)
            {
                auto texUnitState = it.getNext();
                if (texUnitState->getTextureName() == mSceneRtName)
                {
                    pass->setInput(idx, mSceneRtName);
                    break;
                }
            }

            pass->setMaterialName(GetEffectMaterialName());
        }

        bool success = false;
        mCompositorInstance = chain->addCompositor(mCompositor);
        if (nullptr != mCompositorInstance)
        {
            mCompositorInstance->addListener(this);
            success = true;
        }
        return success;
    }
    //-------------------------------------------------------
    void PostEffect::Prepare(const Ogre::RenderWindow* window, Ogre::CompositorChain* chain)
    {
        mRenderWindow = window;

        //Setup parameters dictionary
        CreateParametersDictionary();

        mSceneRtName = "Texture/RT/" + GetUniquePostfix();

        //Create material for the specific effect if it was not created before (by another instance)
        Ogre::Material* effectMaterial = Ogre::MaterialManager::getSingleton().getByName(GetEffectMaterialName()).get();
        if (nullptr == effectMaterial)
        {
            effectMaterial = CreateEffectMaterialPrototype(mSceneRtName);
            assert(nullptr != effectMaterial);
        }
        //Create compositor using the created material and add it to the end of the chain
        if (false == CreateCompositor(effectMaterial, chain))
        {
            throw std::runtime_error("PostEffect[InitializeCompositor]: compositor is not supported");
        }
    }

} //namespace OgreEffect