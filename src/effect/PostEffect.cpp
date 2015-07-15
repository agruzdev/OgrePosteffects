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
#include <array>

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

    const Ogre::String PostEffect::TEXTURE_MARKER_PREVIOUS = "TM_Previous";
    const Ogre::String PostEffect::TEXTURE_MARKER_SCENE = "TM_Scene";

    PostEffect::MaterialsVector PostEffect::msMaterialPrototypes = {}; 

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
    bool PostEffect::CreateCompositor(const MaterialsVector & materials, Ogre::CompositorChain* chain)
    {
        mCompositor = Ogre::CompositorManager::getSingleton().create("Compositor/" + GetUniquePostfix(),
            Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
        Ogre::CompositionTechnique* technique = mCompositor->createTechnique();

        //Create a RT to render the scene into
        {
            Ogre::CompositionTechnique::TextureDefinition* textureBg = technique->createTextureDefinition(mSceneRtName);
            textureBg->name = mSceneRtName;
            textureBg->scope = Ogre::CompositionTechnique::TS_GLOBAL;
            textureBg->width = mRenderWindow->getWidth(); //same as render window
            textureBg->height = mRenderWindow->getHeight(); //same as render window
            textureBg->formatList.push_back(Ogre::PixelFormat::PF_R8G8B8A8);
        }

        //if there are more than one material in the post effect, then create additional render targets 
        std::array<Ogre::String, 2> rtPingPong = { "ping", "pong" };
        int pingPongIdx = 0;
        if (materials.size() > 1)
        {
            {
                Ogre::CompositionTechnique::TextureDefinition* texture = technique->createTextureDefinition(rtPingPong[0]);
                texture->width = mRenderWindow->getWidth(); //same as render window
                texture->height = mRenderWindow->getHeight(); //same as render window
                texture->formatList.push_back(Ogre::PixelFormat::PF_R8G8B8A8);
            }
            //will not be used in the case of 2 materials
            if (materials.size() > 2)
            {
                Ogre::CompositionTechnique::TextureDefinition* textureBg = technique->createTextureDefinition(rtPingPong[1]);
                textureBg->width = mRenderWindow->getWidth(); //same as render window
                textureBg->height = mRenderWindow->getHeight(); //same as render window
                textureBg->formatList.push_back(Ogre::PixelFormat::PF_R8G8B8A8);
            }
        }

        {
            Ogre::CompositionTargetPass* target = technique->createTargetPass();
            target->setInputMode(Ogre::CompositionTargetPass::IM_PREVIOUS);
            target->setOutputName(mSceneRtName);
        }

        size_t materialsNumber = materials.size();
        for (size_t matIdx = 0; matIdx < materialsNumber; ++matIdx)
        {
            Ogre::Material* material = materials[matIdx];

            bool isOutputPass = (matIdx == materialsNumber - 1);
            //Create target passes for all materials except the last one; it will be used in the output pass
            Ogre::CompositionTargetPass* target = (false == isOutputPass) ?
                technique->createTargetPass() : technique->getOutputTargetPass();

            target->setInputMode(Ogre::CompositionTargetPass::IM_NONE);
            Ogre::CompositionPass* pass = target->createPass();

            //Iterate all texture unit states to replace texture markers with the local compositor RTs
            Ogre::Pass* matPass = material->getBestTechnique()->getPass(0);
            auto it = matPass->getTextureUnitStateIterator();
            for (size_t texIdx = 0; true == it.hasMoreElements(); ++texIdx)
            {
                auto textureName = it.getNext()->getTextureName();
                if (TEXTURE_MARKER_SCENE == textureName)
                {
                    pass->setInput(texIdx, mSceneRtName);
                }
                else if (TEXTURE_MARKER_PREVIOUS == textureName)
                {
                    //attach one of render targets and change the index to the other one
                    pass->setInput(texIdx, rtPingPong[pingPongIdx]);
                    pingPongIdx = 1 - pingPongIdx;
                }
            }
            if (false == isOutputPass)
            {
                //pingPongIdx is already changed and point another texture
                target->setOutputName(rtPingPong[pingPongIdx]);
            }

            //pass->setMaterialName(GetEffectMaterialName());
            pass->setMaterialName(material->getName());
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
        /*
        Ogre::Material* effectMaterial = Ogre::MaterialManager::getSingleton().getByName(GetEffectMaterialName()).get();
        if (nullptr == effectMaterial)
        {
            effectMaterial = CreateEffectMaterialPrototype(mSceneRtName);
            assert(nullptr != effectMaterial);
        }*/
        if (true == msMaterialPrototypes.empty())
        {
            msMaterialPrototypes = CreateEffectMaterialPrototypes();
            assert(false == msMaterialPrototypes.empty());
        }
        //Create compositor using the created material and add it to the end of the chain
        if (false == CreateCompositor(msMaterialPrototypes, chain))
        {
            throw std::runtime_error("PostEffect[InitializeCompositor]: compositor is not supported");
        }
    }

} //namespace OgreEffect