/**
* @file PostEffectBlackWhite.h
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
    static const char Shader_GL_BlWh_V[] = ""
        "#version 120                                                              \n"
        "                                                                          \n"
        "                                                                          \n"
        "void main()                                                               \n"
        "{                                                                         \n"
        "    gl_TexCoord[0] = gl_MultiTexCoord0;                                   \n"
        "    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;               \n"
        "}                                                                         \n"
        "";

    static const char Shader_GL_BlWh_F[] = ""
        "#version 120                                                              \n"
        "                                                                          \n"
        "uniform sampler2D texture;                                                \n"
        "                                                                          \n"
        "void main()                                                               \n"
        "{                                                                         \n"
        "    float grey = dot(texture2D(texture, gl_TexCoord[0].st).rgb, vec3(0.299, 0.587, 0.114)); \n"
        "    gl_FragColor = vec4(grey, grey, grey, 1.0);                           \n"
        "}                                                                         \n"
        "";
}

namespace OgreEffect
{

    class PostEffectBlackWhite : public PostEffect
    {
    public:
        PostEffectBlackWhite(const Ogre::String& name, size_t id) :
            PostEffect(name, id)
        {

        }
        virtual ~PostEffectBlackWhite()
        {

        }

        virtual MaterialsVector CreateEffectMaterialPrototypes() override
        {
            Ogre::MaterialPtr material = Ogre::MaterialManager::getSingleton().create(
                "Material/BlackWhite" + GetUniquePostfix(), Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

            {
                Ogre::Technique* techniqueGL = material->getTechnique(0);
                Ogre::Pass* pass = techniqueGL->getPass(0);

                {
                    auto vprogram = Ogre::HighLevelGpuProgramManager::getSingleton().createProgram("Shader/GL/V/" + GetUniquePostfix(),
                        Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, "glsl", Ogre::GPT_VERTEX_PROGRAM);
                    vprogram->setSource(Shader_GL_BlWh_V);

                    pass->setVertexProgram(vprogram->getName());
                }

                {
                    auto fprogram = Ogre::HighLevelGpuProgramManager::getSingleton().createProgram("Shader/GL/F/" + GetUniquePostfix(),
                        Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, "glsl", Ogre::GPT_FRAGMENT_PROGRAM);
                    fprogram->setSource(Shader_GL_BlWh_F);

                    auto unit0 = pass->createTextureUnitState(TEXTURE_MARKER_SCENE);
                    unit0->setTextureAddressingMode(Ogre::TextureUnitState::TAM_CLAMP);
                    unit0->setTextureFiltering(Ogre::TFO_NONE);

                    pass->setFragmentProgram(fprogram->getName());
                }
            }
            material->load();
            return { material.get() };
        }
    };

    IMPLEMENT_REGISTRATION_FUNCTION(EffectBlackWhite)
    {
        Ogre::SharedPtr<PostEffectFactory> factory(new DefaultPostEffectFactory<PostEffectBlackWhite>(PostEffectManager::PE_BLACKWHITE));
        manager->RegisterPostEffectFactory(factory);
    }

}//OgreEffect