/**
* @file PostEffectComplex.h
*
* Copyright (c) 2015 by Gruzdev Alexey
*
* Code covered by the MIT License
* The authors make no representations about the suitability of this software
* for any purpose. It is provided "as is" without express or implied warranty.
*/


#ifndef _POSTEFFECT_COMPLEX_H_
#define _POSTEFFECT_COMPLEX_H_

#include "PostEffect.h"

#include <OgreRenderTargetListener.h>

namespace Ogre
{
    class SceneManager;
    class Camera;
    class SceneNode;
    class RenderTarget;
    class Viewport;
}

namespace OgreEffect
{

    class PostEffectComplex : public PostEffect, public Ogre::RenderTargetListener
    {
    private:
        static Ogre::RenderTarget* CreateRenderTarget(const Ogre::String & name, Ogre::Camera* camera, size_t width, size_t height);
        //-------------------------------------------------------

        Ogre::String mRtName;
        Ogre::RenderTarget* mRenderTarget;

        //The material will be created automatically
        //Derived classes should set up a scene in DoSetupScene()
        MaterialsVector CreateEffectMaterialPrototypes() override;

        //Override PostEffect methods
        void DoInit(Ogre::MaterialPtr & material) override;
        void DoUpdate(Ogre::MaterialPtr & material, Ogre::Real time) override;

        void DoPrepare() override;

    protected:
        Ogre::SceneManager* mSceneManager;
        Ogre::Camera* mCamera;
        Ogre::Viewport* mViewport;
        Ogre::SceneNode* mRootNode;
        //-------------------------------------------------------

        /**
         *	Set up a scene for the post effect
         *  Use local SceneManager, Camera, Viewport and SceneNode instead of calling Ogre Root
         */
        virtual void DoSetupScene() = 0;

        /**
         *	Update the effect's scene
         */
        virtual void DoUpdateScene(Ogre::Real time) {};

    public:
        PostEffectComplex(const Ogre::String & name, size_t id);

        virtual ~PostEffectComplex();
    };


}


#endif //_POSTEFFECT_COMPLEX_H_
