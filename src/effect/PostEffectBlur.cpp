/**
* @file PostEffectBlur.h
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

namespace
{
    static const char Shader_GL_Blur_V[] = ""
        "#version 120                                                              \n"
        "                                                                          \n"
        "                                                                          \n"
        "void main()                                                               \n"
        "{                                                                         \n"
        "    gl_TexCoord[0] = gl_MultiTexCoord0;                                   \n"
        "    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;               \n"
        "}                                                                         \n"
        "";

#if 0
    //Gaussian values in points [-2, -1, 0, 1, 2] with sigma = 1.2
#define GAUSS0 "0.343406"
#define GAUSS1 "0.242667"
#define GAUSS2 "0.085629"
#else
    //Gaussian values in points [-2, -1, 0, 1, 2] with sigma = 2.0
#define GAUSS0 "0.251379"
#define GAUSS1 "0.221841"
#define GAUSS2 "0.152469"
#endif

    static const char Shader_GL_Blur_Horz_F[] = ""
        "#version 120                                                                                       \n"
        "                                                                                                   \n"
        "uniform sampler2D texture;                                                                         \n"
        "uniform vec4 offset;                                                                               \n"
        "                                                                                                   \n"
        "void main()                                                                                        \n"
        "{                                                                                                  \n"
        "    vec2 coords  = gl_TexCoord[0].st;                                                              \n"
        "    float step = offset.z;                                                                         \n"
        "    gl_FragColor = "GAUSS2" * texture2D(texture, coords  - vec2(step + step, 0.0)) +               \n"
        "                   "GAUSS1" * texture2D(texture, coords  - vec2(step, 0.0)) +                      \n"
        "                   "GAUSS0" * texture2D(texture, coords) +                                         \n"
        "                   "GAUSS1" * texture2D(texture, coords  + vec2(step, 0.0)) +                      \n"
        "                   "GAUSS2" * texture2D(texture, coords  + vec2(step + step, 0.0));                \n"
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
        "    float step = offset.w;                                                                         \n"
        "    gl_FragColor = "GAUSS2" * texture2D(texture, coords  - vec2(0.0, step + step)) +               \n"
        "                   "GAUSS1" * texture2D(texture, coords  - vec2(0.0, step)) +                      \n"
        "                   "GAUSS0" * texture2D(texture, coords) +                                         \n"
        "                   "GAUSS1" * texture2D(texture, coords  + vec2(0.0, step)) +                      \n"
        "                   "GAUSS2" * texture2D(texture, coords  + vec2(0.0, step + step));                \n"
        "}                                                                                                  \n"
        "";
}

namespace OgreEffect
{

    class PostEffectBlur : public PostEffect
    {
    public:
        PostEffectBlur(const Ogre::String& name, size_t id) :
            PostEffect(name, id)
        {

        }
        virtual ~PostEffectBlur()
        {

        }

        virtual MaterialsVector CreateEffectMaterialPrototypes() override
        {
            Ogre::MaterialPtr material_horz = Ogre::MaterialManager::getSingleton().create(
                "Material/PostEffect/" + GetUniquePostfix() + "/Horz", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

            {
                Ogre::Technique* techniqueGL = material_horz->getTechnique(0);
                Ogre::Pass* pass = techniqueGL->getPass(0);

                {
                    auto vprogram = Ogre::HighLevelGpuProgramManager::getSingleton().createProgram("Shader/GL/V/H/" + GetUniquePostfix(),
                        Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, "glsl", Ogre::GPT_VERTEX_PROGRAM);
                    vprogram->setSource(Shader_GL_Blur_V);

                    pass->setVertexProgram(vprogram->getName());
                }

                {
                    auto fprogram = Ogre::HighLevelGpuProgramManager::getSingleton().createProgram("Shader/GL/F/H/" + GetUniquePostfix(),
                        Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, "glsl", Ogre::GPT_FRAGMENT_PROGRAM);
                    fprogram->setSource(Shader_GL_Blur_Horz_F);

                    auto unit0 = pass->createTextureUnitState(TEXTURE_MARKER_SCENE);
                    unit0->setTextureAddressingMode(Ogre::TextureUnitState::TAM_CLAMP);
                    unit0->setTextureFiltering(Ogre::TFO_NONE);

                    pass->setFragmentProgram(fprogram->getName());

                    auto fparams = pass->getFragmentProgramParameters();
                    fparams->setNamedConstant("texture", 0);
                    fparams->setNamedAutoConstant("offset", Ogre::GpuProgramParameters::ACT_VIEWPORT_SIZE);
                }
            }
            material_horz->load();


            Ogre::MaterialPtr material_vert = Ogre::MaterialManager::getSingleton().create(
                "Material/PostEffect/" + GetUniquePostfix() + "/Vert", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

            {
                Ogre::Technique* techniqueGL = material_vert->getTechnique(0);
                Ogre::Pass* pass = techniqueGL->getPass(0);

                {
                    auto vprogram = Ogre::HighLevelGpuProgramManager::getSingleton().createProgram("Shader/GL/V/V/" + GetUniquePostfix(),
                        Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, "glsl", Ogre::GPT_VERTEX_PROGRAM);
                    vprogram->setSource(Shader_GL_Blur_V);

                    pass->setVertexProgram(vprogram->getName());
                }

                {
                    auto fprogram = Ogre::HighLevelGpuProgramManager::getSingleton().createProgram("Shader/GL/F/V/" + GetUniquePostfix(),
                        Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, "glsl", Ogre::GPT_FRAGMENT_PROGRAM);
                    fprogram->setSource(Shader_GL_Blur_Vert_F);

                    auto unit0 = pass->createTextureUnitState(TEXTURE_MARKER_PREVIOUS);
                    unit0->setTextureAddressingMode(Ogre::TextureUnitState::TAM_CLAMP);
                    unit0->setTextureFiltering(Ogre::TFO_NONE);

                    pass->setFragmentProgram(fprogram->getName());

                    auto fparams = pass->getFragmentProgramParameters();
                    fparams->setNamedConstant("texture", 0);
                    fparams->setNamedAutoConstant("offset", Ogre::GpuProgramParameters::ACT_VIEWPORT_SIZE);
                }
            }
            material_vert->load();

            return{ material_horz.get(), material_vert.get() };
        }
    };

    IMPLEMENT_REGISTRATION_FUNCTION(EffectBlur)
    {
        Ogre::SharedPtr<PostEffectFactory> factory(new DefaultPostEffectFactory<PostEffectBlur>(PostEffectManager::PE_BLUR));
        manager->RegisterPostEffectFactory(factory);
    }

}//OgreEffect
