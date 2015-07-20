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
#include <OgreTextureManager.h>

namespace OgreEffect
{

    const Ogre::String PostEffect::DICTIONARY_NAME = "PostEffect";

    const Ogre::String PostEffect::TEXTURE_MARKER_PREVIOUS = "TM_Previous";
    const Ogre::String PostEffect::TEXTURE_MARKER_SCENE = "TM_Scene";

    OGRE_HashMap<Ogre::String, PostEffect::MaterialsVector> PostEffect::msMaterialPrototypesMap = {};

    //-------------------------------------------------------
    void PostEffect::CreateDummyTexture(const Ogre::String & name)
    {
        Ogre::TextureManager::getSingleton().createManual(name,
            Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
            Ogre::TEX_TYPE_2D, 1, 1, 0, Ogre::PF_L8);
    }
    //-------------------------------------------------------
    void PostEffect::DestroyDummyTexture(const Ogre::String & name)
    {
        Ogre::TextureManager::getSingleton().remove(name);
    }
    //-------------------------------------------------------
    void PostEffect::CreateMarkerDummies()
    {
        for (const Ogre::String & name: { TEXTURE_MARKER_PREVIOUS, TEXTURE_MARKER_SCENE })
        {
            if (true == Ogre::TextureManager::getSingleton().getByName(name).isNull())
            {
                CreateDummyTexture(name);
            }
        }
    }
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
        mTypeName(name), mId(id)
    {
        mName = GetUniquePostfix();
        mTimer = Ogre::Root::getSingleton().getTimer();

        //create dummy textures if they were not created
        CreateMarkerDummies();
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
        return "PostEffect/" + mTypeName + "/" + std::to_string(mId);
    }
    //-------------------------------------------------------
    Ogre::CompositionTechnique::TextureDefinition* PostEffect::CreateTextureDefinition(Ogre::String name, size_t width, size_t height, Ogre::PixelFormat format)
    {
        Ogre::CompositionTechnique::TextureDefinition* texture = mCompositionTechnique->createTextureDefinition(name);
        if (nullptr == texture)
        {
            OGRE_EXCEPT(Ogre::Exception::ERR_INTERNAL_ERROR, "Failed to create texture definition", "PostEffect[CreateTextureDefinition]");
        }
        texture->width = width; //same as render window
        texture->height = height; //same as render window
        texture->formatList.push_back(format);
        return texture;
    }
    //-------------------------------------------------------
    Ogre::String PostEffect::CreateOutputTexture(Ogre::String materialName, size_t width, size_t height, Ogre::PixelFormat format /* = Ogre::PF_R8G8B8A8 */)
    {
        ManualOutputInfo info;
        info.material = materialName;
        info.texture = "TD/" + GetUniquePostfix() + "/" + Ogre::StringConverter::toString(mTexturesCounter);
        info.marker = "Texture/Dummy/" + GetUniquePostfix() + "/" + Ogre::StringConverter::toString(mTexturesCounter);
        ++mTexturesCounter;

        //Create dummy texture to use as marker
        CreateDummyTexture(info.marker);

        //Create new texture definition for the compositor
        CreateTextureDefinition(info.texture, width, height, format);

        //save the info 
        mManualTextures.push_back(info);

        return info.marker;
    }
    //-------------------------------------------------------
    void PostEffect::SetupCompositionTechnique(const MaterialsVector & materials, Ogre::CompositorChain* chain)
    {
        //Create a RT to render the scene into
        {
            Ogre::CompositionTechnique::TextureDefinition* textureBg = mCompositionTechnique->createTextureDefinition(mSceneRtName);
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
            CreateTextureDefinition(rtPingPong[0], mRenderWindow->getWidth(), mRenderWindow->getHeight(), Ogre::PF_R8G8B8A8);
            //will not be used in the case of 2 materials
            if (materials.size() > 2)
            {
                CreateTextureDefinition(rtPingPong[1], mRenderWindow->getWidth(), mRenderWindow->getHeight(), Ogre::PF_R8G8B8A8);
            }
        }

        {
            Ogre::CompositionTargetPass* target = mCompositionTechnique->createTargetPass();
            target->setInputMode(Ogre::CompositionTargetPass::IM_PREVIOUS);
            target->setOutputName(mSceneRtName);
        }

        //create composition target passes for the each material
        //all found markers substitute by created texture definitions
        bool previousOutputWasManual = false;
        size_t materialsNumber = materials.size();
        for (size_t matIdx = 0; matIdx < materialsNumber; ++matIdx)
        {
            Ogre::Material* material = materials[matIdx];
            if (false == material->isLoaded())
            {
                material->load();
            }

            bool isOutputPass = (matIdx == materialsNumber - 1);
            //Create target passes for all materials except the last one; it will be used in the output pass
            Ogre::CompositionTargetPass* target = (false == isOutputPass) ?
                mCompositionTechnique->createTargetPass() : mCompositionTechnique->getOutputTargetPass();

            target->setInputMode(Ogre::CompositionTargetPass::IM_NONE);
            Ogre::CompositionPass* pass = target->createPass();

            //Iterate all texture unit states to replace texture markers with the local compositor RTs
            assert(nullptr != material->getBestTechnique());
            Ogre::Pass* matPass = material->getBestTechnique()->getPass(0);
            auto it = matPass->getTextureUnitStateIterator();
            for (size_t texIdx = 0; true == it.hasMoreElements(); ++texIdx)
            {
                //change texture markers to render target names
                auto textureName = it.getNext()->getTextureName();
                if (TEXTURE_MARKER_SCENE == textureName)
                {
                    pass->setInput(texIdx, mSceneRtName);
                }
                else if (TEXTURE_MARKER_PREVIOUS == textureName)
                {
                    if (previousOutputWasManual)
                    {
                        OGRE_EXCEPT(Ogre::Exception::ERR_INTERNAL_ERROR, "Wrong texture marker. \
                            The PREVIOUS texture cant be used after manual output texture. Use the explicit texture name instead", "PostEffect[CreateCompositor]");
                    }
                    if (0 == matIdx)
                    {
                        OGRE_EXCEPT(Ogre::Exception::ERR_INTERNAL_ERROR, "Wrong texture marker. The PREVIOUS texture cant be used in the first material", "PostEffect[CreateCompositor]");
                    }
                    //attach one of render targets and change the index to the other one
                    pass->setInput(texIdx, rtPingPong[pingPongIdx]);
                    pingPongIdx = 1 - pingPongIdx;
                }
                else //check manual textures 
                {
                    auto entryIt = std::find_if(mManualTextures.cbegin(), mManualTextures.cend(), [&textureName](const ManualOutputInfo& info) { return info.marker == textureName; });
                    if (entryIt != mManualTextures.cend())
                    {
                        pass->setInput(texIdx, entryIt->texture);
                    }
                }
            }
            if (false == isOutputPass)
            {
                //check manual output textures
                //auto entryIt = mManualTextures.find(material->getName());
                Ogre::String materialName = material->getName();
                auto entryIt = std::find_if(mManualTextures.cbegin(), mManualTextures.cend(), [&materialName](const ManualOutputInfo & info){return info.material == materialName;});
                if (entryIt != mManualTextures.cend())
                {
                    //output to the manually created texture
                    target->setOutputName(entryIt->texture);
                    previousOutputWasManual = true;
                }
                else
                {
                    //pingPongIdx is already changed and point another texture
                    target->setOutputName(rtPingPong[pingPongIdx]);
                    previousOutputWasManual = false;
                }
            }

            //pass->setMaterialName(GetEffectMaterialName());
            pass->setMaterialName(material->getName());
        }
        //Free texture dummies for the manual textures
        for (const auto & entry : mManualTextures)
        {
            DestroyDummyTexture(entry.marker);
        }
    }
    //-------------------------------------------------------
    void PostEffect::Prepare(const Ogre::RenderWindow* window, Ogre::CompositorChain* chain)
    {
        mRenderWindow = window;
        mSceneRtName = "Texture/RT/" + GetUniquePostfix();

        //Setup parameters dictionary
        CreateParametersDictionary();

        //Prepare a derived class
        DoPrepare();

        //Create compositor and technique
        //The technique can be used during setting up materials to create additional output textures
        Ogre::CompositorPtr compositor = Ogre::CompositorManager::getSingleton().create("Compositor/" + GetUniquePostfix(),
            Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
        mCompositionTechnique = compositor->createTechnique();

        //Check if materials for this effect type have been created already
        auto & prototypes = msMaterialPrototypesMap[mTypeName]; //get or create
        if (true == prototypes.empty())
        {
            prototypes = CreateEffectMaterialPrototypes();
            assert(false == prototypes.empty());
        }

        //Setup composition technique using the created material and add it to the end of the chain
        SetupCompositionTechnique(prototypes, chain);
        mCompositorInstance = chain->addCompositor(compositor);
        if (nullptr == mCompositorInstance)
        { 
            OGRE_EXCEPT(Ogre::Exception::ERR_INTERNAL_ERROR, "Compositor is not supported", "PostEffect[InitializeCompositor]");
        }
        mCompositorInstance->addListener(this);
    }
    //-------------------------------------------------------
    void PostEffect::notifyMaterialRender(Ogre::uint32 pass_id, Ogre::MaterialPtr & mat)
    {
        (void)pass_id;

        //compute time from the beginning of the effect
        Ogre::Real time = 0;
        if (mStartTime < static_cast<Ogre::Real>(0))
        {
            mStartTime = GetTimeInSeconds();
        }
        else
        {
            time = GetTimeInSeconds() - mStartTime;
        }

        //update the effect
        Update(mat, time);
    }

} //namespace OgreEffect