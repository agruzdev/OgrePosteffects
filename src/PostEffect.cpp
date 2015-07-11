
#include <string>

#include "PostEffect.h"

#include <OgreCompositorManager.h>
#include <OgreRenderWindow.h>
#include <OgreCompositionTechnique.h>
#include <OgreCompositionTargetPass.h>
#include <OgreCompositionPass.h>
#include <OgreCompositorChain.h>

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
PostEffect::PostEffect(const Ogre::String & name, size_t id):
    mName(name), mId(id), mInited(false)
{
    CreateParametersDictionary();
}
//-------------------------------------------------------
PostEffect::~PostEffect()
{

}
//-------------------------------------------------------
Ogre::String PostEffect::GetUniquePostfix() const
{
    return "PostEffect/" + mName + " / " + std::to_string(mId);
}
//-------------------------------------------------------
bool PostEffect::CreateCompositor(const Ogre::RenderWindow* window, Ogre::CompositorChain* chain)
{
    mCompositor = Ogre::CompositorManager::getSingleton().create("Compositor/" + GetUniquePostfix(),
        Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
    Ogre::CompositionTechnique* technique = mCompositor->createTechnique();

    const Ogre::String rtName = "Texture/RT/" + GetUniquePostfix();
    {
        Ogre::CompositionTechnique::TextureDefinition* textureBg = technique->createTextureDefinition(rtName);
        //textureBg->scope = Ogre::CompositionTechnique::TS_GLOBAL;
        textureBg->width = window->getWidth(); //same as render window
        textureBg->height = window->getHeight(); //same as render window
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