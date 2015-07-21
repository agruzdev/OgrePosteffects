/**
* @file PostEffectGodRays.h
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

#define THRESHOLD "0.6"
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


    // http://fabiensanglard.net/lightScattering/
    // https://code.google.com/p/natureal/source/browse/trunk/PGR2project/shaders/godrays/godrays_fs.glsl?r=18 
    static const char Shader_GL_Blur_F[] = ""
        "#version 120                                                                                       \n"
        "                                                                                                   \n"
        "const int samples = 8;                                                                            \n"
        "const float exposure = 0.2 / float(samples);                                                       \n"
        "const float decay = 1.0;                                                                           \n"
        "const float density = 0.7;                                                                         \n"
        "const float weight = 6.0;                                                                          \n"
        "                                                                                                   \n"
        "uniform sampler2D texture;                                                                         \n"
        "uniform vec2 lightPosition;  //from [0,1] in screen space                                          \n"
        "                                                                                                   \n"
        "void main()                                                                                        \n"
        "{                                                                                                  \n"
        "    vec2 coords = gl_TexCoord[0].st;                                                               \n"
        "    vec2 delta = (coords - lightPosition.xy) / float(samples) * density;                           \n"
        "    float illuminationDecay = 1.0;                                                                 \n"
        "    vec3 color = vec3(0.0, 0.0, 0.0);                                                              \n"
        "    for (int i = 0; i < samples; i++)                                                              \n"
        "    {                                                                                              \n"
        "        coords -= delta;                                                                           \n"
        "        vec3 sample = texture2D(texture, coords).rgb;                                              \n"
        "        sample *= illuminationDecay * weight;                                                      \n"
        "        color += sample;                                                                           \n"
        "        illuminationDecay *= decay;                                                                \n"
        "    }                                                                                              \n"
        "    gl_FragColor = vec4(exposure * color, 1.0);                                                    \n"
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

    class PostEffectGodRays : public PostEffect
    {
    public:
        PostEffectGodRays(const Ogre::String& name, size_t id) :
            PostEffect(name, id)
        {

        }
        virtual ~PostEffectGodRays()
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

            Ogre::MaterialPtr materialBlur = Ogre::MaterialManager::getSingleton().create(
                "Material/PostEffect/" + GetUniquePostfix() + "/Blur", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
            {
                Ogre::Technique* techniqueGL = materialBlur->getTechnique(0);
                Ogre::Pass* pass = techniqueGL->getPass(0);
                {
                    auto vprogram = Ogre::HighLevelGpuProgramManager::getSingleton().createProgram("Shader/GL/V/" + GetUniquePostfix(),
                        Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, "glsl", Ogre::GPT_VERTEX_PROGRAM);
                    vprogram->setSource(Shader_GL_Common_V);
                    pass->setVertexProgram(vprogram->getName());
                }
                {
                    auto fprogram = Ogre::HighLevelGpuProgramManager::getSingleton().createProgram("Shader/GL/F/" + GetUniquePostfix(),
                        Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, "glsl", Ogre::GPT_FRAGMENT_PROGRAM);
                    fprogram->setSource(Shader_GL_Blur_F);

                    auto unit0 = pass->createTextureUnitState(outputDownsample2);
                    unit0->setTextureAddressingMode(Ogre::TextureUnitState::TAM_CLAMP);
                    unit0->setTextureFiltering(Ogre::TFO_BILINEAR);

                    pass->setFragmentProgram(fprogram->getName());

                    auto fparams = pass->getFragmentProgramParameters();
                    fparams->setNamedConstant("texture", 0);
                    fparams->setNamedConstant("lightPosition", Ogre::Vector2(0.5f, 0.5f));
                }
            }
            auto outputBlur = CreateOutputTexture(materialBlur->getName(), mRenderWindow->getWidth() / 8, mRenderWindow->getHeight() / 8);
            //-------------------------------------------------------
            Ogre::MaterialPtr materialBlur2 = Ogre::MaterialManager::getSingleton().create(
                "Material/PostEffect/" + GetUniquePostfix() + "/Blur2", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
            {
                Ogre::Technique* techniqueGL = materialBlur2->getTechnique(0);
                Ogre::Pass* pass = techniqueGL->getPass(0);
                {
                    auto vprogram = Ogre::HighLevelGpuProgramManager::getSingleton().createProgram("Shader/GL/V/b2/" + GetUniquePostfix(),
                        Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, "glsl", Ogre::GPT_VERTEX_PROGRAM);
                    vprogram->setSource(Shader_GL_Common_V);
                    pass->setVertexProgram(vprogram->getName());
                }
                {
                    auto fprogram = Ogre::HighLevelGpuProgramManager::getSingleton().createProgram("Shader/GL/F/b2/" + GetUniquePostfix(),
                        Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, "glsl", Ogre::GPT_FRAGMENT_PROGRAM);
                    fprogram->setSource(Shader_GL_Blur_F);

                    auto unit0 = pass->createTextureUnitState(outputBlur);
                    unit0->setTextureAddressingMode(Ogre::TextureUnitState::TAM_CLAMP);
                    unit0->setTextureFiltering(Ogre::TFO_BILINEAR);

                    pass->setFragmentProgram(fprogram->getName());

                    auto fparams = pass->getFragmentProgramParameters();
                    fparams->setNamedConstant("texture", 0);
                    fparams->setNamedConstant("lightPosition", Ogre::Vector2(0.5f, 0.5f));
                }
            }
            auto outputBlur2 = CreateOutputTexture(materialBlur2->getName(), mRenderWindow->getWidth() / 8, mRenderWindow->getHeight() / 8);
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

                    auto unit1 = pass->createTextureUnitState(outputBlur2);
                    unit1->setTextureAddressingMode(Ogre::TextureUnitState::TAM_CLAMP);
                    unit1->setTextureFiltering(Ogre::TFO_BILINEAR);

                    pass->setFragmentProgram(fprogram->getName());
                    auto fparams = pass->getFragmentProgramParameters();
                    fparams->setNamedConstant("texture", 0);
                    fparams->setNamedConstant("texbloom", 1);
                }
            }

            //-------------------------------------------------------
            return{ materialThreshold.get(), materialDownsample.get(), materialDownsample2.get(), materialBlur.get(), materialBlur2.get(), materialBlend.get() };
        }
    };

    IMPLEMENT_REGISTRATION_FUNCTION(EffectGodRays)
    {
        Ogre::SharedPtr<PostEffectFactory> factory(new DefaultPostEffectFactory<PostEffectGodRays>(PostEffectManager::PE_GODRAYS));
        manager->RegisterPostEffectFactory(factory);
    }

}//OgreEffect
