
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

    virtual void CreateEffectMaterialPrototype() override
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

                auto unit0 = pass->createTextureUnitState();
                unit0->setTextureAddressingMode(Ogre::TextureUnitState::TAM_CLAMP);
                unit0->setTextureFiltering(Ogre::TFO_NONE);

                auto fparams = pass->getFragmentProgramParameters();
                fparams->setNamedConstant("texture", 0);
                fparams->setNamedConstant("fadecolor", Ogre::Vector4(0.0, 0.0, 1.0, 0.5));
            }
        }
        material->load();
    }

    virtual void DoInit(Ogre::MaterialPtr & material)
    {
        (void)material;
    }
};

IMPLEMENT_REGISTRATION_FUNCTION(EffectFading)
{
    Ogre::SharedPtr<PostEffectFactory> factory(new DefaultPostEffectFactory<PostEffectFading>(PostEffectManager::PE_FADING));
    manager->RegisterPostEffectFactory(factory);
}