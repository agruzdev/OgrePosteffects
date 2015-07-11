
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


class PostEffectNull : public PostEffect
{
    static const Ogre::String EFFECT_MATERIAL_NAME;

public:
    PostEffectNull(const Ogre::String& name, size_t id):
        PostEffect(name, id)
    {

    }
    virtual ~PostEffectNull()
    {

    }

    virtual Ogre::String GetEffectMaterialName() const override
    {
        return EFFECT_MATERIAL_NAME;
    }

    virtual bool DoInit(const Ogre::RenderWindow* window) override
    {
        (void)window;

        Ogre::MaterialPtr material = Ogre::MaterialManager::getSingleton().create(
            EFFECT_MATERIAL_NAME, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

        {
            Ogre::Technique* techniqueGL = material->getTechnique(0);
            Ogre::Pass* pass = techniqueGL->getPass(0);

            {
                auto vprogram = Ogre::HighLevelGpuProgramManager::getSingleton().createProgram("Shader/GL/Copy/V",
                    Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, "glsl", Ogre::GPT_VERTEX_PROGRAM);
                vprogram->setSource(Shader_GL_Copy_V);
            }

            {
                auto fprogram = Ogre::HighLevelGpuProgramManager::getSingleton().createProgram("Shader/GL/Copy/F",
                    Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, "glsl", Ogre::GPT_FRAGMENT_PROGRAM);
                fprogram->setSource(Shader_GL_Copy_F);

                auto unit0 = pass->createTextureUnitState();
                unit0->setTextureAddressingMode(Ogre::TextureUnitState::TAM_CLAMP);
                unit0->setTextureFiltering(Ogre::TFO_NONE);
            }

            pass->setVertexProgram("Shader/GL/Copy/V");
            pass->setFragmentProgram("Shader/GL/Copy/F");
        }
        material->load();
        return true;
    }
};

const Ogre::String PostEffectNull::EFFECT_MATERIAL_NAME = "Material/PostEffectNull";

IMPLEMENT_REGISTRATION_FUNCTION(EffectNull)
{
    Ogre::SharedPtr<PostEffectFactory> factory(new DefaultPostEffectFactory<PostEffectNull>(PostEffectManager::PE_NULL));
    manager->RegisterPostEffectFactory(factory);
}