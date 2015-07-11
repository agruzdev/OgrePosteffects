
#ifndef _POSTEFFECT_MANAGER_H_
#define _POSTEFFECT_MANAGER_H_

#include <unordered_map>
#include <memory>

#include <OgreSingleton.h>
#include <OgreSharedPtr.h>

#define DECLARE_REGISTRATION_FUNCTION(EffectName) void GlobalRegisterPostEffect_##EffectName();
#define IMPLEMENT_REGISTRATION_FUNCTION(EffectName) void GlobalRegisterPostEffect_##EffectName()
#define INVOKE_REGISTRATION_FUNCTION(EffectName) GlobalRegisterPostEffect_##EffectName();

namespace Ogre
{
    class RenderWindow;
    class Viewport;
}

class PostEffect;
class PostEffectFactory;

class PostEffectManager : public Ogre::Singleton<PostEffectManager>
{
public:
    //Default post effects
    static const Ogre::String PE_NULL;
    //-------------------------------------------------------

private:
    using FactoriesMap = std::unordered_map<Ogre::String, Ogre::SharedPtr<PostEffectFactory> >;
    //-------------------------------------------------------

    void RegisterDefaultFactories();
    //-------------------------------------------------------

    FactoriesMap mFactories;
    //-------------------------------------------------------

    PostEffectManager(const PostEffectManager&) = delete;
    PostEffectManager(const PostEffectManager&&) = delete;
    PostEffectManager& operator=(const PostEffectManager&) = delete;
    PostEffectManager& operator=(const PostEffectManager&&) = delete;
    //-------------------------------------------------------

public:  

    PostEffectManager();
    ~PostEffectManager();

    /**
     *	Create single post effect
     *  It will be the only effect in the OGRE compositors chain
     */
    PostEffect* CreatePostEffect(const Ogre::String & effectType, Ogre::RenderWindow* window, Ogre::Viewport* viewport);

    

    /**
     * Register post effects factory to use the post effect in future
     * Use it to register manually created post effect
     */
    void RegisterPostEffectFactory(Ogre::SharedPtr<PostEffectFactory> factory);
    /**
     * Unregister the effect factory 
     */
    void UnregisterPostEffectFactory(Ogre::SharedPtr<PostEffectFactory> factory);

    /** Override standard Singleton retrieval.
    @remarks
    Why do we do this? Well, it's because the Singleton
    implementation is in a .h file, which means it gets compiled
    into anybody who includes it. This is needed for the
    Singleton template to work, but we actually only want it
    compiled into the implementation of the class based on the
    Singleton, not all of them. If we don't change this, we get
    link errors when trying to use the Singleton-based class from
    an outside dll.
    @par
    This method just delegates to the template version anyway,
    but the implementation stays in this single compilation unit,
    preventing link errors.
    */
    static PostEffectManager& getSingleton(void);
    /** Override standard Singleton retrieval.
    @remarks
    Why do we do this? Well, it's because the Singleton
    implementation is in a .h file, which means it gets compiled
    into anybody who includes it. This is needed for the
    Singleton template to work, but we actually only want it
    compiled into the implementation of the class based on the
    Singleton, not all of them. If we don't change this, we get
    link errors when trying to use the Singleton-based class from
    an outside dll.
    @par
    This method just delegates to the template version anyway,
    but the implementation stays in this single compilation unit,
    preventing link errors.
    */
    static PostEffectManager* getSingletonPtr(void);

};



#endif

