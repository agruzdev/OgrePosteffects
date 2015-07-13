
#include <string>

#include "PostEffect.h"

#include <OgreCompositorManager.h>
#include <OgreRenderWindow.h>
#include <OgreCompositionTechnique.h>
#include <OgreCompositionTargetPass.h>
#include <OgreCompositionPass.h>
#include <OgreCompositorChain.h>
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
        return "PostEffect/" + mName + " / " + std::to_string(mId);
    }
    //-------------------------------------------------------
    bool PostEffect::CreateCompositor(Ogre::CompositorChain* chain)
    {
        mCompositor = Ogre::CompositorManager::getSingleton().create("Compositor/" + GetUniquePostfix(),
            Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
        Ogre::CompositionTechnique* technique = mCompositor->createTechnique();

        const Ogre::String rtName = "Texture/RT/" + GetUniquePostfix();
        {
            Ogre::CompositionTechnique::TextureDefinition* textureBg = technique->createTextureDefinition(rtName);
            //textureBg->scope = Ogre::CompositionTechnique::TS_GLOBAL;
            textureBg->width = mRenderWindow->getWidth(); //same as render window
            textureBg->height = mRenderWindow->getHeight(); //same as render window
            textureBg->formatList.push_back(Ogre::PixelFormat::PF_R8G8B8A8);
        }

        {
            Ogre::CompositionTargetPass* target = technique->createTargetPass();
            target->setInputMode(Ogre::CompositionTargetPass::IM_PREVIOUS);
            target->setOutputName(rtName);
        }
        {
            Ogre::CompositionTargetPass* target = technique->getOutputTargetPass();
            target->setInputMode(Ogre::CompositionTargetPass::IM_NONE);
            Ogre::CompositionPass* pass = target->createPass();
            pass->setInput(0, rtName);
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

        //Create material for the specific effect if it was not created before (by another instance)
        if (nullptr == Ogre::MaterialManager::getSingleton().getByName(GetEffectMaterialName()).get())
        {
            CreateEffectMaterialPrototype();
        }
        //Create compositor using the created material and add it to the end of the chain
        if (false == CreateCompositor(chain))
        {
            throw std::runtime_error("PostEffect[InitializeCompositor]: compositor is not supported");
        }
    }

} //namespace OgreEffect