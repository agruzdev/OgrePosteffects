/**
* @file PostEffectManager.h
*
* Copyright (c) 2015 by Gruzdev Alexey
*
* Code covered by the MIT License
* The authors make no representations about the suitability of this software
* for any purpose. It is provided "as is" without express or implied warranty.
*/

#ifndef _POSTEFFECT_MANAGER_H_
#define _POSTEFFECT_MANAGER_H_

#include <unordered_map>
#include <memory>

#include <loki/Singleton.h>

#include <OgreSharedPtr.h>

#define DECLARE_REGISTRATION_FUNCTION(EffectName) void GlobalRegisterPostEffect_##EffectName(PostEffectManager* manager);
#define IMPLEMENT_REGISTRATION_FUNCTION(EffectName) void GlobalRegisterPostEffect_##EffectName(PostEffectManager* manager)
#define INVOKE_REGISTRATION_FUNCTION(EffectName) GlobalRegisterPostEffect_##EffectName(this);

namespace Ogre
{
    class RenderWindow;
    class Viewport;
}

namespace OgreEffect
{

    class PostEffect;
    class PostEffectFactory;

    class PostEffectManager
    {
    public:
        //Default post effects
        static const Ogre::String PE_NULL;
        static const Ogre::String PE_FADING;
        static const Ogre::String PE_BLUR;
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

        /**
         *	Syntax sugar to fit OGRE style
         */
        static PostEffectManager& getSingleton();
        /**
        *	Syntax sugar to fit OGRE style
        */
        static PostEffectManager* getSingletonPtr();

        //-------------------------------------------------------

        friend struct Loki::CreateUsingNew<PostEffectManager>;
    };

    using sPostEffectsManager = Loki::SingletonHolder<PostEffectManager, Loki::CreateUsingNew>;

}//namespace OgreEffect

#endif

