/**
* @file PostEffectRain.cpp
*
* Copyright (c) 2015 by Gruzdev Alexey
*
* Code covered by the MIT License
* The authors make no representations about the suitability of this software
* for any purpose. It is provided "as is" without express or implied warranty.
*/

#include <utility>

#include "PostEffectComplex.h"

#include "PostEffectFactory.h"
#include "PostEffectManager.h"

#include <OgreEntity.h>
#include <OgreSceneNode.h>
#include <OgreCamera.h>
#include <OgreTextureManager.h>

#include <OgreMaterial.h>
#include <OgreTechnique.h>
#include <OgrePass.h>
#include <OgreHighLevelGpuProgram.h>
#include <OgreHighLevelGpuProgramManager.h>

#include <OgreParticleSystem.h>
#include <OgreParticleSystemManager.h>
#include <OgreParticleEmitter.h>
#include <OgreParticleEmitterFactory.h>

#include <OgreBoxEmitter.h>
#include <OgreBoxEmitterFactory.h>
#include <OgreParticleAffector.h>
#include <OgreParticleSystemRenderer.h>

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
        "uniform sampler2D dropTexture;                                            \n"
        "                                                                          \n"
        "void main()                                                               \n"
        "{                                                                         \n"
        "    float alpha = texture2D(dropTexture, gl_TexCoord[0].st).a;            \n"
        "    gl_FragColor = vec4(0.8, 0.8, 0.85, alpha * 0.9);                     \n"
        "}                                                                         \n"
        "";
}

namespace OgreEffect
{



    class PostEffectRain : public PostEffectComplex
    {
    public:
        PostEffectRain(const Ogre::String & name, size_t id) :
            PostEffectComplex(name, id)
        {

        }

        ~PostEffectRain()
        {

        }

        Ogre::MaterialPtr CreateRainDropMaterial(const Ogre::String & name)
        {
            Ogre::TexturePtr rainDropTexture = Ogre::TextureManager::getSingleton().getByName("Texture/EffectRain/RainDrop");
            if (nullptr == rainDropTexture.get())
            {
                //load image
                Ogre::Image rainDropImage;
                rainDropImage.load("drop.tga", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
                rainDropTexture = Ogre::TextureManager::getSingleton().loadImage("Texture/EffectRain/RainDrop",
                    Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, rainDropImage, 
                    Ogre::TextureType::TEX_TYPE_2D, 4, 1.0f, true, Ogre::PF_A8);
            }

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

                    auto unit0 = pass->createTextureUnitState(rainDropTexture->getName());
                    unit0->setTextureAddressingMode(Ogre::TextureUnitState::TAM_CLAMP);
                    unit0->setTextureFiltering(Ogre::TFO_TRILINEAR);

                    auto fparams = pass->getFragmentProgramParameters();
                    fparams->setNamedConstant("dropTexture", 0);
                }
            }
            return material;
        }

        virtual void DoSetupScene() override
        {
            Ogre::MaterialPtr transMat = CreateRainDropMaterial("Material/RainDrop/" + GetUniquePostfix());

            mSceneManager->setAmbientLight(Ogre::ColourValue(0.5, 0.5, 0.5));

            Ogre::Light* l = mSceneManager->createLight("Light/Main/" + GetUniquePostfix());
            l->setPosition(20, 80, 50);


            
            

            auto particlesSystem = mSceneManager->createParticleSystem(100000, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
            particlesSystem->setMaterialName(transMat->getName());
            particlesSystem->setDefaultDimensions(3, 20);
            particlesSystem->setCullIndividually(true);

            auto renderer = particlesSystem->getRenderer();
            renderer->setParameter("billboard_type", "oriented_self");
            renderer->setParameter("rotation_type", "vertex");
            
            for (auto depthAndRate : std::vector<std::pair<float, float> >
                { { 200.0f, 5.0f }, { 100.0f, 120.0f }, { -100.0f, 150.0f }, { -400.0f, 200.0f }, { -800.0f, 250.0f }, { -1000.0f, 300.0f } })
            {
                Ogre::Plane plane = Ogre::Plane(Ogre::Vector3(0.0f, 0.0f, 1.0f), Ogre::Vector3(0.0f, 0.0f, depthAndRate.first));
                Ogre::vector<Ogre::Vector4>::type intersection;
                mCamera->forwardIntersect(plane, &intersection);
                size_t width = static_cast<size_t>(2 * std::ceil(intersection[0].x));
                size_t height = static_cast<size_t>(2 * std::ceil(intersection[0].y));

                auto emitter = particlesSystem->addEmitter("Box");
                emitter->setDirection(Ogre::Vector3::NEGATIVE_UNIT_Y);
                emitter->setEmissionRate(depthAndRate.second);
                emitter->setTimeToLive(15);
                emitter->setEnabled(true);
                emitter->setPosition(Ogre::Vector3(0.0f, height * 1.4f, depthAndRate.first));
                emitter->setParameter("width", Ogre::StringConverter::toString(width * 1.5f));
                emitter->setParameter("height", "1");
                emitter->setParameter("depth", "1");
                emitter->setMinParticleVelocity(200.0f);
                emitter->setMaxParticleVelocity(300.0f);
            }

            auto affector = particlesSystem->addAffector("LinearForce");
            affector->setParameter("force_vector", "0 -1 0");
            affector->setParameter("force_application", "add");

            auto node = mSceneManager->getRootSceneNode()->createChildSceneNode();
            node->attachObject(particlesSystem);

            Ogre::Quaternion rotation;
            rotation.FromAngleAxis(Ogre::Radian(Ogre::Degree(10.0f)), Ogre::Vector3::UNIT_Z);
            node->setOrientation(rotation);
        }

        void DoUpdateScene(Ogre::Real time) override
        {
        }

    };

    IMPLEMENT_REGISTRATION_FUNCTION(EffectRain)
    {
        Ogre::SharedPtr<PostEffectFactory> factory(new DefaultPostEffectFactory<PostEffectRain>(PostEffectManager::PE_RAIN));
        manager->RegisterPostEffectFactory(factory);
    }

}