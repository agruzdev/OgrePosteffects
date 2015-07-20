/**
* @file PostEffectDownsampleTest.h
*
* Copyright (c) 2015 by Gruzdev Alexey
*
* Code covered by the MIT License
* The authors make no representations about the suitability of this software
* for any purpose. It is provided "as is" without express or implied warranty.
*/

#include "PostEffectFactory.h"
#include "PostEffectManager.h"

#include <OgreMaterial.h>
#include <OgreMaterialManager.h>
#include <OgreHighLevelGpuProgram.h>
#include <OgreHighLevelGpuProgramManager.h>
#include <OgreTechnique.h>
#include <OgrePass.h>
#include <OgreRenderWindow.h>

namespace
{
    static const char Shader_GL_Copy_V[] = ""
        "#version 120                                                              \n"
        "                                                                          \n"
        "                                                                          \n"
        "void main()                                                               \n"
        "{                                                                         \n"
        "    gl_TexCoord[0] = gl_MultiTexCoord0;                                   \n"
        "    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;               \n"
        "}                                                                         \n"
        "";

    static const char Shader_GL_Copy_F[] = ""
        "#version 120                                                              \n"
        "                                                                          \n"
        "uniform sampler2D texture;                                                \n"
        "                                                                          \n"
        "void main()                                                               \n"
        "{                                                                         \n"
        "    gl_FragColor = texture2D(texture, gl_TexCoord[0].st);                 \n"
        "}                                                                         \n"
        "";
}

namespace OgreEffect
{

    class PostEffectDownsampleTest : public PostEffect
    {
    public:
        PostEffectDownsampleTest(const Ogre::String& name, size_t id) :
            PostEffect(name, id)
        {

        }
        virtual ~PostEffectDownsampleTest()
        {

        }

        virtual MaterialsVector CreateEffectMaterialPrototypes() override
        {
            Ogre::MaterialPtr material1 = Ogre::MaterialManager::getSingleton().create(
                "Material/Downsample1/" + GetUniquePostfix(), Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

            {
                Ogre::Technique* techniqueGL = material1->getTechnique(0);
                Ogre::Pass* pass = techniqueGL->getPass(0);

                {
                    auto vprogram = Ogre::HighLevelGpuProgramManager::getSingleton().createProgram("Shader/GL/V/" + GetUniquePostfix(),
                        Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, "glsl", Ogre::GPT_VERTEX_PROGRAM);
                    vprogram->setSource(Shader_GL_Copy_V);

                    pass->setVertexProgram(vprogram->getName());
                }

                {
                    auto fprogram = Ogre::HighLevelGpuProgramManager::getSingleton().createProgram("Shader/GL/F/" + GetUniquePostfix(),
                        Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, "glsl", Ogre::GPT_FRAGMENT_PROGRAM);
                    fprogram->setSource(Shader_GL_Copy_F);

                    auto unit0 = pass->createTextureUnitState(TEXTURE_MARKER_SCENE);
                    unit0->setTextureAddressingMode(Ogre::TextureUnitState::TAM_CLAMP);
                    unit0->setTextureFiltering(Ogre::TFO_NONE);

                    pass->setFragmentProgram(fprogram->getName());
                }
            }
            
            auto output1 = CreateOutputTexture(material1->getName(), mRenderWindow->getWidth() / 2, mRenderWindow->getHeight() / 2);

            Ogre::MaterialPtr material2 = Ogre::MaterialManager::getSingleton().create(
                "Material/Downsample2/" + GetUniquePostfix(), Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

            {
                Ogre::Technique* techniqueGL = material2->getTechnique(0);
                Ogre::Pass* pass = techniqueGL->getPass(0);

                {
                    auto vprogram = Ogre::HighLevelGpuProgramManager::getSingleton().createProgram("Shader/GL/V/2/" + GetUniquePostfix(),
                        Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, "glsl", Ogre::GPT_VERTEX_PROGRAM);
                    vprogram->setSource(Shader_GL_Copy_V);

                    pass->setVertexProgram(vprogram->getName());
                }

                {
                    auto fprogram = Ogre::HighLevelGpuProgramManager::getSingleton().createProgram("Shader/GL/F/2/" + GetUniquePostfix(),
                        Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, "glsl", Ogre::GPT_FRAGMENT_PROGRAM);
                    fprogram->setSource(Shader_GL_Copy_F);

                    auto unit0 = pass->createTextureUnitState(output1);
                    unit0->setTextureAddressingMode(Ogre::TextureUnitState::TAM_CLAMP);
                    unit0->setTextureFiltering(Ogre::TFO_NONE);

                    pass->setFragmentProgram(fprogram->getName());
                }
            }

            auto output2 = CreateOutputTexture(material2->getName(), mRenderWindow->getWidth() / 4, mRenderWindow->getHeight() / 4);

            Ogre::MaterialPtr material3 = Ogre::MaterialManager::getSingleton().create(
                "Material/Downsample3/" + GetUniquePostfix(), Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

            {
                Ogre::Technique* techniqueGL = material3->getTechnique(0);
                Ogre::Pass* pass = techniqueGL->getPass(0);

                {
                    auto vprogram = Ogre::HighLevelGpuProgramManager::getSingleton().createProgram("Shader/GL/V/3/" + GetUniquePostfix(),
                        Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, "glsl", Ogre::GPT_VERTEX_PROGRAM);
                    vprogram->setSource(Shader_GL_Copy_V);

                    pass->setVertexProgram(vprogram->getName());
                }

                {
                    auto fprogram = Ogre::HighLevelGpuProgramManager::getSingleton().createProgram("Shader/GL/F/3/" + GetUniquePostfix(),
                        Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, "glsl", Ogre::GPT_FRAGMENT_PROGRAM);
                    fprogram->setSource(Shader_GL_Copy_F);

                    auto unit0 = pass->createTextureUnitState(output2);
                    unit0->setTextureAddressingMode(Ogre::TextureUnitState::TAM_CLAMP);
                    unit0->setTextureFiltering(Ogre::TFO_NONE);

                    pass->setFragmentProgram(fprogram->getName());
                }
            }

            auto output3 = CreateOutputTexture(material3->getName(), mRenderWindow->getWidth() / 8, mRenderWindow->getHeight() / 8);

            Ogre::MaterialPtr material4 = Ogre::MaterialManager::getSingleton().create(
                "Material/Downsample4/" + GetUniquePostfix(), Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

            {
                Ogre::Technique* techniqueGL = material4->getTechnique(0);
                Ogre::Pass* pass = techniqueGL->getPass(0);

                {
                    auto vprogram = Ogre::HighLevelGpuProgramManager::getSingleton().createProgram("Shader/GL/V/4/" + GetUniquePostfix(),
                        Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, "glsl", Ogre::GPT_VERTEX_PROGRAM);
                    vprogram->setSource(Shader_GL_Copy_V);

                    pass->setVertexProgram(vprogram->getName());
                }

                {
                    auto fprogram = Ogre::HighLevelGpuProgramManager::getSingleton().createProgram("Shader/GL/F/4/" + GetUniquePostfix(),
                        Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, "glsl", Ogre::GPT_FRAGMENT_PROGRAM);
                    fprogram->setSource(Shader_GL_Copy_F);

                    auto unit0 = pass->createTextureUnitState(output3);
                    unit0->setTextureAddressingMode(Ogre::TextureUnitState::TAM_CLAMP);
                    unit0->setTextureFiltering(Ogre::TFO_NONE);

                    pass->setFragmentProgram(fprogram->getName());
                }
            }

            return { material1.get(), material2.get(), material3.get(), material4.get() };
        }
    };

    IMPLEMENT_REGISTRATION_FUNCTION(EffectDownsampleTest)
    {
        Ogre::SharedPtr<PostEffectFactory> factory(new DefaultPostEffectFactory<PostEffectDownsampleTest>("DownsampleTest"));
        manager->RegisterPostEffectFactory(factory);
    }

}//OgreEffect