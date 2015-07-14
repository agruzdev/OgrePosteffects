/**
* @file PostEffectFading.cpp
*
* Copyright (c) 2015 by Gruzdev Alexey
*
* Code covered by the MIT License
* The authors make no representations about the suitability of this software
* for any purpose. It is provided "as is" without express or implied warranty.
*/

#include <cmath>

#include "PostEffectFactory.h"
#include "PostEffectManager.h"

#include <OgreMaterial.h>
#include <OgreMaterialManager.h>
#include <OgreHighLevelGpuProgram.h>
#include <OgreHighLevelGpuProgramManager.h>
#include <OgreTechnique.h>
#include <OgrePass.h>

namespace OgreEffect
{

    namespace
    {
        static const char Shader_GL_Fading_V[] = ""
            "#version 120                                                              \n"
            "                                                                          \n"
            "                                                                          \n"
            "void main()                                                               \n"
            "{                                                                         \n"
            "    gl_TexCoord[0] = gl_MultiTexCoord0;                                   \n"
            "    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;               \n"
            "}                                                                         \n"
            "";

        static const char Shader_GL_Fading_F[] = ""
            "#version 120                                                              \n"
            "                                                                          \n"
            "uniform sampler2D texture;                                                \n"
            "uniform vec4 fadecolor;                                                   \n"
            "                                                                          \n"
            "void main()                                                               \n"
            "{                                                                         \n"
            "    vec3 texCol  = texture2D(texture, gl_TexCoord[0].st).rgb;             \n"
            "    gl_FragColor = vec4(mix(texCol, fadecolor.rgb, fadecolor.a), 1.0);    \n"
            "}                                                                         \n"
            "";
    }


    class PostEffectFading : public PostEffect
    {
        class CmdColourParameter : public Ogre::ParamCommand
        {
        public:
            Ogre::String doGet(const void* target) const
            {
                return Ogre::StringConverter::toString(static_cast<const PostEffectFading*>(target)->mColor);
            }
            void doSet(void* target, const Ogre::String& val)
            {
                static_cast<PostEffectFading*>(target)->mColor = Ogre::StringConverter::parseColourValue(val);
            }
        };
        //-------------------------------------------------------
        static CmdColourParameter msColorParameter;
        //-------------------------------------------------------

        Ogre::ColourValue mColor;

    public:
        PostEffectFading(const Ogre::String& name, size_t id) :
            PostEffect(name, id)
        {

        }
        virtual ~PostEffectFading()
        {

        }

        virtual Ogre::String GetEffectMaterialName() const override
        {
            return "Material/PostEffect/" + GetUniquePostfix();
        }

        virtual Ogre::Material* CreateEffectMaterialPrototype(const Ogre::String & sceneTextureName) override
        {
            Ogre::MaterialPtr material = Ogre::MaterialManager::getSingleton().create(
                GetEffectMaterialName(), Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

            {
                Ogre::Technique* techniqueGL = material->getTechnique(0);
                Ogre::Pass* pass = techniqueGL->getPass(0);

                {
                    auto vprogram = Ogre::HighLevelGpuProgramManager::getSingleton().createProgram("Shader/GL/V/" + GetUniquePostfix(),
                        Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, "glsl", Ogre::GPT_VERTEX_PROGRAM);
                    vprogram->setSource(Shader_GL_Fading_V);
                    pass->setVertexProgram(vprogram->getName());
                }

                {
                    auto fprogram = Ogre::HighLevelGpuProgramManager::getSingleton().createProgram("Shader/GL/F/" + GetUniquePostfix(),
                        Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, "glsl", Ogre::GPT_FRAGMENT_PROGRAM);
                    fprogram->setSource(Shader_GL_Fading_F);
                    pass->setFragmentProgram(fprogram->getName());

                    auto unit0 = pass->createTextureUnitState(sceneTextureName);
                    unit0->setTextureAddressingMode(Ogre::TextureUnitState::TAM_CLAMP);
                    unit0->setTextureFiltering(Ogre::TFO_NONE);

                    auto fparams = pass->getFragmentProgramParameters();
                    fparams->setNamedConstant("texture", 0);
                    fparams->setNamedConstant("fadecolor", Ogre::Vector4(0.0, 0.0, 1.0, 0.5));
                }
            }
            material->load();
            return material.get();
        }

        virtual void DoCreateParametersDictionary(Ogre::ParamDictionary* dictionary) override
        {
            assert(nullptr != dictionary);
            dictionary->addParameter(
                Ogre::ParameterDef("color", "Color of the fading effect", Ogre::PT_COLOURVALUE),
                &msColorParameter);
        }

        virtual void DoInit(Ogre::MaterialPtr & material)
        {
            (void)material;
        }

        virtual void DoUpdate(Ogre::MaterialPtr & material, Ogre::Real time)
        {
            (void)material;
            (void)time;

            Ogre::Real alpha = static_cast<Ogre::Real>(0.1 + std::fabs(std::sin(time)) * 0.5);

            auto fparams = material->getBestTechnique()->getPass(0)->getFragmentProgramParameters();
            fparams->setNamedConstant("fadecolor", Ogre::Vector4(mColor[0], mColor[1], mColor[2], alpha));
        }
    };

    PostEffectFading::CmdColourParameter PostEffectFading::msColorParameter;

    IMPLEMENT_REGISTRATION_FUNCTION(EffectFading)
    {
        Ogre::SharedPtr<PostEffectFactory> factory(new DefaultPostEffectFactory<PostEffectFading>(PostEffectManager::PE_FADING));
        manager->RegisterPostEffectFactory(factory);
    }

}//namespace OgreEffect