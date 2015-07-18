/**
* @file PostEffectComplexTest.cpp
*
* Copyright (c) 2015 by Gruzdev Alexey
*
* Code covered by the MIT License
* The authors make no representations about the suitability of this software
* for any purpose. It is provided "as is" without express or implied warranty.
*/

#include "PostEffectComplex.h"

#include "PostEffectFactory.h"
#include "PostEffectManager.h"

#include <OgreEntity.h>
#include <OgreSceneNode.h>

#include <OgreMaterial.h>
#include <OgreTechnique.h>
#include <OgrePass.h>
#include <OgreHighLevelGpuProgram.h>
#include <OgreHighLevelGpuProgramManager.h>

namespace
{
    static const char Shader_GL_Trans_V[] = ""
        "#version 120                                                              \n"
        "                                                                          \n"
        "                                                                          \n"
        "void main()                                                               \n"
        "{                                                                         \n"
        "    gl_TexCoord[0] = gl_MultiTexCoord0;                                   \n"
        "    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;               \n"
        "}                                                                         \n"
        "";

    static const char Shader_GL_Trans_F[] = ""
        "#version 120                                                              \n"
        "                                                                          \n"
        "void main()                                                               \n"
        "{                                                                         \n"
        "    gl_FragColor = vec4(1.0, 1.0, 1.0, 0.5);                              \n"
        "}                                                                         \n"
        "";
}

namespace OgreEffect
{



    class PostEffectComplexTest : public PostEffectComplex
    {

        Ogre::SceneNode* mSolidObjNode = nullptr;
        Ogre::SceneNode* mTransparentObjNode = nullptr;

    public:
        PostEffectComplexTest(const Ogre::String & name, size_t id) :
            PostEffectComplex(name, id)
        {

        }

        ~PostEffectComplexTest()
        {

        }

        Ogre::MaterialPtr CreateTransparentMaterial(const Ogre::String & name)
        {
            Ogre::MaterialPtr material = Ogre::MaterialManager::getSingleton().create(
                name, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

            {
                Ogre::Technique* techniqueGL = material->getTechnique(0);
                Ogre::Pass* pass = techniqueGL->getPass(0);

                {
                    auto vprogram = Ogre::HighLevelGpuProgramManager::getSingleton().createProgram("Shader/GL/V/Trans/" + GetUniquePostfix(),
                        Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, "glsl", Ogre::GPT_VERTEX_PROGRAM);
                    vprogram->setSource(Shader_GL_Trans_V);

                    pass->setVertexProgram(vprogram->getName());
                }

                {
                    auto fprogram = Ogre::HighLevelGpuProgramManager::getSingleton().createProgram("Shader/GL/F/Trans/" + GetUniquePostfix(),
                        Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, "glsl", Ogre::GPT_FRAGMENT_PROGRAM);
                    fprogram->setSource(Shader_GL_Trans_F);

                    pass->setFragmentProgram(fprogram->getName());
                }
            }
            return material;
        }

        virtual void DoSetupScene() override
        {
            Ogre::MaterialPtr transMat = CreateTransparentMaterial("Material/Transparent/" + GetUniquePostfix());

            mSceneManager->setAmbientLight(Ogre::ColourValue(0.5, 0.5, 0.5));

            Ogre::Light* l = mSceneManager->createLight("Light/Main/" + GetUniquePostfix());
            l->setPosition(20, 80, 50);

            //auto sphere = mSceneManager->createEntity(Ogre::SceneManager::PT_SPHERE);
            //mRootNode->attachObject(sphere);

            auto solidObj = mSceneManager->createEntity(Ogre::SceneManager::PT_SPHERE);
            mSolidObjNode = mRootNode->createChildSceneNode();
            mSolidObjNode->attachObject(solidObj);
            mSolidObjNode->setPosition(-60.0f, 0.0f, 0.0f);

            auto transparentObj = mSceneManager->createEntity(Ogre::SceneManager::PT_SPHERE);
            transparentObj->setMaterial(transMat);
            mTransparentObjNode = mRootNode->createChildSceneNode();
            mTransparentObjNode->attachObject(transparentObj);
            mTransparentObjNode->setPosition(60.0f, 0.0f, 0.0f);

        }

        void DoUpdateScene(Ogre::Real time) override
        {
            Ogre::Real offset = static_cast<Ogre::Real>(std::sin(time) * 10.0);

            mSolidObjNode->setPosition(Ogre::Vector3(-60.0f, offset, 0.0f));
            mTransparentObjNode->setPosition(Ogre::Vector3(60.0f, -offset, 0.0f));
        }

    };

    IMPLEMENT_REGISTRATION_FUNCTION(EffectComplexTest)
    {
        Ogre::SharedPtr<PostEffectFactory> factory(new DefaultPostEffectFactory<PostEffectComplexTest>("ComplexTest"));
        manager->RegisterPostEffectFactory(factory);
    }

}