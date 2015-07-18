/**
* @file PostEffectComplex.cpp
*
* Copyright (c) 2015 by Gruzdev Alexey
*
* Code covered by the MIT License
* The authors make no representations about the suitability of this software
* for any purpose. It is provided "as is" without express or implied warranty.
*/

#include "PostEffectComplex.h"

#include <OgreRoot.h>
#include <OgreSceneManager.h>
#include <OgreCamera.h>
#include <OgreSceneNode.h>
#include <OgreRenderTexture.h>
#include <OgreRenderTarget.h>
#include <OgreTextureManager.h>
#include <OgreHardwarePixelBuffer.h>
#include <OgreMaterial.h>
#include <OgreTechnique.h>
#include <OgrePass.h>
#include <OgreHighLevelGpuProgram.h>
#include <OgreHighLevelGpuProgramManager.h>
#include <OgreRenderWindow.h>
#include <OgreViewport.h>


namespace
{
    static const char Shader_GL_Blend_V[] = ""
        "#version 120                                                              \n"
        "                                                                          \n"
        "                                                                          \n"
        "void main()                                                               \n"
        "{                                                                         \n"
        "    gl_TexCoord[0] = gl_MultiTexCoord0;                                   \n"
        "    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;               \n"
        "}                                                                         \n"
        "";

    static const char Shader_GL_Blend_F[] = ""
        "#version 120                                                              \n"
        "                                                                          \n"
        "uniform sampler2D texture;                                                \n"
        "uniform sampler2D scene;                                                  \n"
        "                                                                          \n"
        "void main()                                                               \n"
        "{                                                                         \n"
        "    vec4 effect = texture2D(texture, gl_TexCoord[0].st);                  \n"
        "    vec4 image  = texture2D(scene, gl_TexCoord[0].st);                    \n"
        "    gl_FragColor = vec4(mix(image.rgb, effect.rgb, effect.a), 1.0);       \n"
        "}                                                                         \n"
        "";
}

namespace OgreEffect
{

    Ogre::RenderTarget* PostEffectComplex::CreateRenderTarget(const Ogre::String & name, Ogre::Camera* camera, size_t width, size_t height)
    {
        Ogre::TexturePtr rtTexture = Ogre::TextureManager::getSingleton().createManual(name,
            Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
            Ogre::TEX_TYPE_2D,
            width, height, 0,
            Ogre::PF_R8G8B8A8,
            Ogre::TU_RENDERTARGET);


        Ogre::RenderTarget* renderTarget = rtTexture->getBuffer()->getRenderTarget();
        renderTarget->addViewport(camera);
        renderTarget->setAutoUpdated(true);

        Ogre::Viewport* vp = renderTarget->getViewport(0);
        vp->setClearEveryFrame(true);
        vp->setOverlaysEnabled(false);
        vp->setBackgroundColour(Ogre::ColourValue::Black);

        return renderTarget;
    }
    //-------------------------------------------------------

    PostEffectComplex::PostEffectComplex(const Ogre::String & name, size_t id) :
        PostEffect(name, id)
    {
        
    }
    //-------------------------------------------------------
    PostEffectComplex::~PostEffectComplex()
    {
        if (nullptr != mRootNode)
        {
            mRootNode->removeAndDestroyAllChildren();
        }
        if (nullptr != mSceneManager && nullptr != mCamera)
        {
            if (nullptr != mCamera)
            {
                mSceneManager->destroyCamera(mCamera);
            }
            if (nullptr != mRenderTarget)
            {
                Ogre::TextureManager::getSingleton().remove(mRtName);
            }
            Ogre::Root::getSingleton().destroySceneManager(mSceneManager);
        }

    }
    //-------------------------------------------------------
    void PostEffectComplex::DoPrepare()
    {
        mSceneManager = Ogre::Root::getSingleton().createSceneManager(Ogre::ST_GENERIC, "SceneManager/" + GetUniquePostfix());
        if (nullptr == mSceneManager)
        {
            OGRE_EXCEPT(Ogre::Exception::ERR_INTERNAL_ERROR, "Failed to create a scene manager", "PostEffectComplex[DoPrepare]");
        }

        mCamera = mSceneManager->createCamera("Camera/Complex" + GetUniquePostfix());
        if (nullptr == mCamera)
        {
            OGRE_EXCEPT(Ogre::Exception::ERR_INTERNAL_ERROR, "Failed to create a camera", "PostEffectComplex[DoPrepare]");
        }
        //Some default settings which can be changed in the effect
        mCamera->setAspectRatio(static_cast<Ogre::Real>(mRenderWindow->getWidth()) / mRenderWindow->getHeight());
        mCamera->setPosition(Ogre::Vector3(0, 0, 300));
        mCamera->lookAt(Ogre::Vector3(0, 0, 0));
        mCamera->setNearClipDistance(5);

        mRootNode = mSceneManager->getRootSceneNode();

        //Prepare render target
        assert(nullptr != mRenderWindow);
        mRtName = "Texture/RT/Complex" + GetUniquePostfix();
        mRenderTarget = CreateRenderTarget(mRtName, mCamera, mRenderWindow->getWidth(), mRenderWindow->getHeight());
        if (nullptr == mRenderTarget)
        {
            OGRE_EXCEPT(Ogre::Exception::ERR_INTERNAL_ERROR, "Failed to create RT", "PostEffectComplex[DoPrepare]");
        }
        mRenderTarget->addListener(this);

        mViewport = mRenderTarget->getViewport(0);
        mViewport->setBackgroundColour(Ogre::ColourValue(0.0f, 0.0f, 0.0f, 0.0f));

        //Setup geometry
        DoSetupScene();
    }
    //-------------------------------------------------------
    PostEffect::MaterialsVector PostEffectComplex::CreateEffectMaterialPrototypes()
    {
        //Prepare material
        Ogre::MaterialPtr material = Ogre::MaterialManager::getSingleton().create(
            "Material/Complex" + GetUniquePostfix() + "/Blend", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

        {
            Ogre::Technique* techniqueGL = material->getTechnique(0);
            Ogre::Pass* pass = techniqueGL->getPass(0);

            {
                auto vprogram = Ogre::HighLevelGpuProgramManager::getSingleton().createProgram("Shader/GL/V/ComplexEffectBlend/" + GetUniquePostfix(),
                    Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, "glsl", Ogre::GPT_VERTEX_PROGRAM);
                vprogram->setSource(Shader_GL_Blend_V);

                pass->setVertexProgram(vprogram->getName());
            }

            {
                auto fprogram = Ogre::HighLevelGpuProgramManager::getSingleton().createProgram("Shader/GL/F/ComplexEffectBlend/" + GetUniquePostfix(),
                    Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, "glsl", Ogre::GPT_FRAGMENT_PROGRAM);
                fprogram->setSource(Shader_GL_Blend_F);

                auto unit0 = pass->createTextureUnitState(mRtName);
                unit0->setTextureAddressingMode(Ogre::TextureUnitState::TAM_CLAMP);
                unit0->setTextureFiltering(Ogre::TFO_NONE);

                auto unit1 = pass->createTextureUnitState(TEXTURE_MARKER_SCENE);
                unit1->setTextureAddressingMode(Ogre::TextureUnitState::TAM_CLAMP);
                unit1->setTextureFiltering(Ogre::TFO_NONE);

                pass->setFragmentProgram(fprogram->getName());

                auto fparams = pass->getFragmentProgramParameters();
                fparams->setNamedConstant("texture", 0);
                fparams->setNamedConstant("scene", 1);
            }
        }
        material->load();
        return{ material.get() };

    }
    //-------------------------------------------------------
    void PostEffectComplex::DoInit(Ogre::MaterialPtr & material)
    {
        (void)material;
    }
    //-------------------------------------------------------
    void PostEffectComplex::DoUpdate(Ogre::MaterialPtr & material, Ogre::Real time)
    {
        (void)material;
        DoUpdateScene(time);
    }

}