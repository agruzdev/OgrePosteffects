/**
* @file PostEffectBloom.h
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

    static const char Shader_GL_Common_V[] = ""
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

#define THRESHOLD "0.7"
    static const char Shader_GL_Threshold_F[] = ""
        "#version 120                                                                               \n"
        "                                                                                           \n"
        "uniform sampler2D texture;                                                                 \n"
        "                                                                                           \n"
        "void main()                                                                                \n"
        "{                                                                                          \n"
        "    vec3  rgb = texture2D(texture, gl_TexCoord[0].st).rgb;                                 \n"
        "    float lum = dot(rgb, vec3(0.299, 0.587, 0.114));                                       \n"
        "    gl_FragColor = (lum > "THRESHOLD") ? vec4(rgb, lum) : vec4(0.0, 0.0, 0.0, 0.0);        \n"
        "}                                                                                          \n"
        "";


#define KOEF0 "0.4"
#define KOEF1 "0.3"
#define KOEF2 "0.2"

    static const char Shader_GL_Blur_Horz_F[] = ""
        "#version 120                                                                                       \n"
        "                                                                                                   \n"
        "uniform sampler2D texture;                                                                         \n"
        "uniform vec4 offset;                                                                               \n"
        "                                                                                                   \n"
        "void main()                                                                                        \n"
        "{                                                                                                  \n"
        "    vec2 coords  = gl_TexCoord[0].st;                                                              \n"
        "    float step = offset.z * 1.5;                                                                         \n"
        "    gl_FragColor = "KOEF2" * texture2D(texture, coords  - vec2(step + step, 0.0)) +               \n"
        "                   "KOEF1" * texture2D(texture, coords  - vec2(step, 0.0)) +                      \n"
        "                   "KOEF0" * texture2D(texture, coords) +                                         \n"
        "                   "KOEF1" * texture2D(texture, coords  + vec2(step, 0.0)) +                      \n"
        "                   "KOEF2" * texture2D(texture, coords  + vec2(step + step, 0.0));                \n"
        "}                                                                                                  \n"
        "";

    static const char Shader_GL_Blur_Vert_F[] = ""
        "#version 120                                                                                       \n"
        "                                                                                                   \n"
        "uniform sampler2D texture;                                                                         \n"
        "uniform vec4 offset;                                                                               \n"
        "                                                                                                   \n"
        "void main()                                                                                        \n"
        "{                                                                                                  \n"
        "    vec2 coords  = gl_TexCoord[0].st;                                                              \n"
        "    float step = offset.w * 1.5;                                                                         \n"
        "    gl_FragColor = "KOEF2" * texture2D(texture, coords  - vec2(0.0, step + step)) +               \n"
        "                   "KOEF1" * texture2D(texture, coords  - vec2(0.0, step)) +                      \n"
        "                   "KOEF0" * texture2D(texture, coords) +                                         \n"
        "                   "KOEF1" * texture2D(texture, coords  + vec2(0.0, step)) +                      \n"
        "                   "KOEF2" * texture2D(texture, coords  + vec2(0.0, step + step));                \n"
        "}                                                                                                  \n"
        "";

    static const char Shader_GL_Blend_F[] = ""
        "#version 120                                                              \n"
        "                                                                          \n"
        "uniform sampler2D texture;                                                \n"
        "uniform sampler2D texbloom;                                               \n"
        "                                                                          \n"
        "void main()                                                               \n"
        "{                                                                         \n"
        "    vec3 rgb = texture2D(texture, gl_TexCoord[0].st).rgb;                 \n"
        "    vec4 bloom = texture2D(texbloom, gl_TexCoord[0].st).rgba;             \n"
        "    gl_FragColor = vec4(clamp(rgb + bloom.rgb, 0.0, 1.0), 1.0);           \n"
        "}                                                                         \n"
        "";
}

namespace OgreEffect
{

    class PostEffectBloom : public PostEffect
    {
    public:
        PostEffectBloom(const Ogre::String& name, size_t id) :
            PostEffect(name, id)
        {

        }
        virtual ~PostEffectBloom()
        {

        }

        virtual MaterialsVector CreateEffectMaterialPrototypes() override
        {
            Ogre::MaterialPtr materialThreshold = Ogre::MaterialManager::getSingleton().create(
                "Material/PostEffect/" + GetUniquePostfix() + "/Threshold", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
            {
                Ogre::Technique* techniqueGL = materialThreshold->getTechnique(0);
                Ogre::Pass* pass = techniqueGL->getPass(0);
                {
                    auto vprogram = Ogre::HighLevelGpuProgramManager::getSingleton().createProgram("Shader/GL/V/Threshold/" + GetUniquePostfix(),
                        Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, "glsl", Ogre::GPT_VERTEX_PROGRAM);
                    vprogram->setSource(Shader_GL_Common_V);
                    pass->setVertexProgram(vprogram->getName());
                }
                {
                    auto fprogram = Ogre::HighLevelGpuProgramManager::getSingleton().createProgram("Shader/GL/F/Threshold/" + GetUniquePostfix(),
                        Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, "glsl", Ogre::GPT_FRAGMENT_PROGRAM);
                    fprogram->setSource(Shader_GL_Threshold_F);

                    auto unit0 = pass->createTextureUnitState(TEXTURE_MARKER_SCENE);
                    unit0->setTextureAddressingMode(Ogre::TextureUnitState::TAM_CLAMP);
                    unit0->setTextureFiltering(Ogre::TFO_NONE);

                    pass->setFragmentProgram(fprogram->getName());
                }
            }
            auto outputThreshold = CreateOutputTexture(materialThreshold->getName(), mRenderWindow->getWidth() / 2, mRenderWindow->getHeight() / 2);
            //-------------------------------------------------------

            Ogre::MaterialPtr materialDownsample = Ogre::MaterialManager::getSingleton().create(
                "Material/Downsample/" + GetUniquePostfix(), Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

            {
                Ogre::Technique* techniqueGL = materialDownsample->getTechnique(0);
                Ogre::Pass* pass = techniqueGL->getPass(0);
                {
                    auto vprogram = Ogre::HighLevelGpuProgramManager::getSingleton().createProgram("Shader/GL/V/DS/" + GetUniquePostfix(),
                        Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, "glsl", Ogre::GPT_VERTEX_PROGRAM);
                    vprogram->setSource(Shader_GL_Common_V);
                    pass->setVertexProgram(vprogram->getName());
                }
                {
                    auto fprogram = Ogre::HighLevelGpuProgramManager::getSingleton().createProgram("Shader/GL/F/DS/" + GetUniquePostfix(),
                        Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, "glsl", Ogre::GPT_FRAGMENT_PROGRAM);
                    fprogram->setSource(Shader_GL_Copy_F);

                    auto unit0 = pass->createTextureUnitState(outputThreshold);
                    unit0->setTextureAddressingMode(Ogre::TextureUnitState::TAM_CLAMP);
                    unit0->setTextureFiltering(Ogre::TFO_BILINEAR);

                    pass->setFragmentProgram(fprogram->getName());
                }
            }
            auto outputDownsample = CreateOutputTexture(materialDownsample->getName(), mRenderWindow->getWidth() / 4, mRenderWindow->getHeight() / 4);
            //-------------------------------------------------------

            Ogre::MaterialPtr materialDownsample2 = Ogre::MaterialManager::getSingleton().create(
                "Material/Downsample2/" + GetUniquePostfix(), Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

            {
                Ogre::Technique* techniqueGL = materialDownsample2->getTechnique(0);
                Ogre::Pass* pass = techniqueGL->getPass(0);
                {
                    auto vprogram = Ogre::HighLevelGpuProgramManager::getSingleton().createProgram("Shader/GL/V/DS2/" + GetUniquePostfix(),
                        Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, "glsl", Ogre::GPT_VERTEX_PROGRAM);
                    vprogram->setSource(Shader_GL_Common_V);
                    pass->setVertexProgram(vprogram->getName());
                }
                {
                    auto fprogram = Ogre::HighLevelGpuProgramManager::getSingleton().createProgram("Shader/GL/F/DS2/" + GetUniquePostfix(),
                        Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, "glsl", Ogre::GPT_FRAGMENT_PROGRAM);
                    fprogram->setSource(Shader_GL_Copy_F);

                    auto unit0 = pass->createTextureUnitState(outputDownsample);
                    unit0->setTextureAddressingMode(Ogre::TextureUnitState::TAM_CLAMP);
                    unit0->setTextureFiltering(Ogre::TFO_BILINEAR);

                    pass->setFragmentProgram(fprogram->getName());
                }
            }
            auto outputDownsample2 = CreateOutputTexture(materialDownsample2->getName(), mRenderWindow->getWidth() / 8, mRenderWindow->getHeight() / 8);
            //-------------------------------------------------------

            Ogre::MaterialPtr materialHorz = Ogre::MaterialManager::getSingleton().create(
                "Material/PostEffect/" + GetUniquePostfix() + "/Horz", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
            {
                Ogre::Technique* techniqueGL = materialHorz->getTechnique(0);
                Ogre::Pass* pass = techniqueGL->getPass(0);
                {
                    auto vprogram = Ogre::HighLevelGpuProgramManager::getSingleton().createProgram("Shader/GL/V/H/" + GetUniquePostfix(),
                        Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, "glsl", Ogre::GPT_VERTEX_PROGRAM);
                    vprogram->setSource(Shader_GL_Common_V);
                    pass->setVertexProgram(vprogram->getName());
                }
                {
                    auto fprogram = Ogre::HighLevelGpuProgramManager::getSingleton().createProgram("Shader/GL/F/H/" + GetUniquePostfix(),
                        Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, "glsl", Ogre::GPT_FRAGMENT_PROGRAM);
                    fprogram->setSource(Shader_GL_Blur_Horz_F);

                    auto unit0 = pass->createTextureUnitState(outputDownsample2);
                    unit0->setTextureAddressingMode(Ogre::TextureUnitState::TAM_CLAMP);
                    unit0->setTextureFiltering(Ogre::TFO_BILINEAR);

                    pass->setFragmentProgram(fprogram->getName());

                    auto fparams = pass->getFragmentProgramParameters();
                    fparams->setNamedConstant("texture", 0);
                    fparams->setNamedAutoConstant("offset", Ogre::GpuProgramParameters::ACT_VIEWPORT_SIZE);
                }
            }
            auto outputHorz = CreateOutputTexture(materialHorz->getName(), mRenderWindow->getWidth() / 8, mRenderWindow->getHeight() / 8);
            //-------------------------------------------------------
            Ogre::MaterialPtr materialVert = Ogre::MaterialManager::getSingleton().create(
                "Material/PostEffect/" + GetUniquePostfix() + "/Vert", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
            {
                Ogre::Technique* techniqueGL = materialVert->getTechnique(0);
                Ogre::Pass* pass = techniqueGL->getPass(0);
                {
                    auto vprogram = Ogre::HighLevelGpuProgramManager::getSingleton().createProgram("Shader/GL/V/V/" + GetUniquePostfix(),
                        Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, "glsl", Ogre::GPT_VERTEX_PROGRAM);
                    vprogram->setSource(Shader_GL_Common_V);

                    pass->setVertexProgram(vprogram->getName());
                }
                {
                    auto fprogram = Ogre::HighLevelGpuProgramManager::getSingleton().createProgram("Shader/GL/F/V/" + GetUniquePostfix(),
                        Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, "glsl", Ogre::GPT_FRAGMENT_PROGRAM);
                    fprogram->setSource(Shader_GL_Blur_Vert_F);

                    auto unit0 = pass->createTextureUnitState(outputHorz);
                    unit0->setTextureAddressingMode(Ogre::TextureUnitState::TAM_CLAMP);
                    unit0->setTextureFiltering(Ogre::TFO_BILINEAR);

                    pass->setFragmentProgram(fprogram->getName());

                    auto fparams = pass->getFragmentProgramParameters();
                    fparams->setNamedConstant("texture", 0);
                    fparams->setNamedAutoConstant("offset", Ogre::GpuProgramParameters::ACT_VIEWPORT_SIZE);
                }
            }
            auto outputVert = CreateOutputTexture(materialVert->getName(), mRenderWindow->getWidth() / 8, mRenderWindow->getHeight() / 8);
            //-------------------------------------------------------
            Ogre::MaterialPtr materialBlend = Ogre::MaterialManager::getSingleton().create(
                "Material/Blend/" + GetUniquePostfix(), Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
            {
                Ogre::Technique* techniqueGL = materialBlend->getTechnique(0);
                Ogre::Pass* pass = techniqueGL->getPass(0);
                {
                    auto vprogram = Ogre::HighLevelGpuProgramManager::getSingleton().createProgram("Shader/GL/V/Blend/" + GetUniquePostfix(),
                        Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, "glsl", Ogre::GPT_VERTEX_PROGRAM);
                    vprogram->setSource(Shader_GL_Common_V);
                    pass->setVertexProgram(vprogram->getName());
                }
                {
                    auto fprogram = Ogre::HighLevelGpuProgramManager::getSingleton().createProgram("Shader/GL/F/Blend/" + GetUniquePostfix(),
                        Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, "glsl", Ogre::GPT_FRAGMENT_PROGRAM);
                    fprogram->setSource(Shader_GL_Blend_F);

                    auto unit0 = pass->createTextureUnitState(TEXTURE_MARKER_SCENE);
                    unit0->setTextureAddressingMode(Ogre::TextureUnitState::TAM_CLAMP);
                    unit0->setTextureFiltering(Ogre::TFO_NONE);

                    auto unit1 = pass->createTextureUnitState(outputVert);
                    unit1->setTextureAddressingMode(Ogre::TextureUnitState::TAM_CLAMP);
                    unit1->setTextureFiltering(Ogre::TFO_BILINEAR);

                    pass->setFragmentProgram(fprogram->getName());
                    auto fparams = pass->getFragmentProgramParameters();
                    fparams->setNamedConstant("texture", 0);
                    fparams->setNamedConstant("texbloom", 1);
                }
            }

            //-------------------------------------------------------
            return{ materialThreshold.get(), materialDownsample.get(), materialDownsample2.get(), materialHorz.get(), materialVert.get(), materialBlend.get() };
        }
    };

    IMPLEMENT_REGISTRATION_FUNCTION(EffectBloom)
    {
        Ogre::SharedPtr<PostEffectFactory> factory(new DefaultPostEffectFactory<PostEffectBloom>(PostEffectManager::PE_BLOOM));
        manager->RegisterPostEffectFactory(factory);
    }

}//OgreEffect
