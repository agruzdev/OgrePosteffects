/**
 * @file PostEffect.h
 *
 * Copyright (c) 2015 by Gruzdev Alexey
 *
 * Code covered by the MIT License
 * The authors make no representations about the suitability of this software
 * for any purpose. It is provided "as is" without express or implied warranty.
 */


#ifndef _POSTEFFECT_H_
#define _POSTEFFECT_H_

#include <OgreRoot.h>
#include <OgrePrerequisites.h>
#include <OgreStringInterface.h>
#include <OgreResource.h>
#include <OgreCompositor.h>
#include <OgreSharedPtr.h>
#include <OgreCompositorInstance.h>
#include <OgreMaterial.h>
#include <OgreCompositionTechnique.h>

#if (OGRE_VERSION_MAJOR < 1) || (OGRE_VERSION_MAJOR == 1 && OGRE_VERSION_MINOR < 9)
#error Only Ogre version 1.9.0 or higher is supported
#endif

#if OGRE_VERSION_PATCH < 1
    //In OGRE SDK 1.9.0 is used name HashMap
    #define OGRE_HashMap HashMap
#endif

namespace Ogre
{
    class RenderWindow;
    class Timer;
}

namespace OgreEffect
{

    class PostEffect : public Ogre::StringInterface, public Ogre::CompositorInstance::Listener
    {
        struct ManualOutputInfo
        {
            Ogre::String material; ///< material name 
            Ogre::String texture; ///< texture definition name
            Ogre::String marker; ///< temporal name of a texture marker
        };
    protected:
        using MaterialsVector = Ogre::vector<Ogre::Material*>::type;
        //-------------------------------------------------------

        static const Ogre::String DICTIONARY_NAME; ///< name of the parameters dictionary for this class

        static const Ogre::String TEXTURE_MARKER_SCENE; ///< name marker for attaching RT with rendered scene 
        static const Ogre::String TEXTURE_MARKER_PREVIOUS; ///< name marker for attaching RT from the previous pass
        //-------------------------------------------------------

    private:
        //Store materials of the effect shared between all instances
        //Will be initialized by the first effect instance
        //ToDo: maybe using the static field is not the best idea. Consider other solutions
        static OGRE_HashMap<Ogre::String, MaterialsVector> msMaterialPrototypesMap;
        //-------------------------------------------------------
        
        //Small empty texture to use its name in texture unit states
        static void CreateDummyTexture(const Ogre::String & name);
        //Free resources
        static void DestroyDummyTexture(const Ogre::String & name);

        /**
         * Create dummy textures with names of markers in order to
         * the Material Manager will generate 'failed to find texture' errors
         */
        static void CreateMarkerDummies();
        //-------------------------------------------------------

        bool mInited = false;
        Ogre::Real mStartTime = -1;

        //Ogre::CompositorPtr mCompositor;
        Ogre::CompositorInstance* mCompositorInstance;
        Ogre::CompositionTechnique* mCompositionTechnique;

        //name of the render target where the scene will be rendered before applying post effects
        Ogre::String mSceneRtName;

        size_t mTexturesCounter = 0;
        //save materials and texture definitions created for them with the help of CreateTextureDefinition
        Ogre::vector<ManualOutputInfo>::type mManualTextures;

        //-------------------------------------------------------
        //Get current global time 
        Ogre::Real GetTimeInSeconds() const;

        //Basic initialization; then calls DoCreateParametersDictionary()
        void CreateParametersDictionary();


        /**
        * Create and setup post effect compositor; Add to the end of the chain
        * @return true if the created compositor has any supported technique
        */
        void SetupCompositionTechnique(const MaterialsVector & materials, Ogre::CompositorChain* chain);

        /**
         *	Create a simple texture definition for the compositor technique 
         */
        Ogre::CompositionTechnique::TextureDefinition* CreateTextureDefinition(Ogre::String name, size_t width, size_t height, Ogre::PixelFormat format);

        PostEffect(const PostEffect&) = delete;
        PostEffect(const PostEffect&&) = delete;
        PostEffect& operator=(const PostEffect&) = delete;
        PostEffect& operator=(const PostEffect&&) = delete;
        //-------------------------------------------------------

    protected:
        const Ogre::String mTypeName; ///< Unique name of the post effect type
        const size_t mId; ///< Unique number of the post effect instance
        Ogre::String mName; ///< Unique name of the instance

        const Ogre::RenderWindow* mRenderWindow;

        Ogre::Timer* mTimer = Ogre::Root::getSingleton().getTimer();

        //Helper method to generate unique names
        Ogre::String GetUniquePostfix() const;

        /**
         * Create a new local texture for passing output from the specified material
         * The name of the created texture should be used in another material explicitly 
         * (Using market PREVIOUS is not allowed)
         *
         * @return name of the new texture
         */
        Ogre::String CreateOutputTexture(Ogre::String materialName, size_t width, size_t height, Ogre::PixelFormat format = Ogre::PF_R8G8B8A8);

        //-------------------------------------------------------

        //Methods for implementing in the derived classes

        /**
         * Create a material which will be used in the post effect compositor pass
         * This method will be called only one time to create the material prototype during
         * the first PostEffect instance initialization. The compositor instances
         * of the all PostEffect instances will be using copies of this material as
         * it is implemented in the OGRE compositor
         *
         * Use the special name marker TEXTURE_MARKER_SCENE to attach compositor local RT with
         * rendered scene to material's texture unit state. Can be used in any material
         *
         * Use the name marker TEXTURE_MARKER_PREVIOUS to attach output of the previous material
         * to the current material's texture unit state
         *
         * To create a nonlinear structure of processing or use low resolutions passes create 
         * manual output textures with the help of CreateOutputTexture()
         *
         * @return A vector of materials which will be transformed to compositor passes in the same order
         * 
         */
        virtual MaterialsVector CreateEffectMaterialPrototypes() = 0;

        //Setup dictionary values depending on the specific PostEffect implementation
        virtual void DoCreateParametersDictionary(Ogre::ParamDictionary* dictionary) {}
        /*
         * Effects initialization
         */
        virtual void DoInit(Ogre::MaterialPtr & material) {}
        //Updating the effect's material parameters on every frame
        virtual void DoUpdate(Ogre::MaterialPtr & material, Ogre::Real time) {}
        //-------------------------------------------------------

        //Initialize stuff before creating material 
        //Will be called one time per instance
        virtual void DoPrepare() {}

    public:
        /**
         *	Create post effect instance
         * @param name Effect's type name
         * @param id unique id of this instance
         */
        PostEffect(const Ogre::String & name, size_t id);
        /**
         *	Destroy effect instance
         */
        virtual ~PostEffect();

        /**
         *	Get material used for post effect full screen plane rendering in the compositor pass
         */
        //virtual Ogre::String GetEffectMaterialName() const = 0;

        /**
         * Init effect's compositor
         * Will be called by the post effects manager
         */
        void Prepare(const Ogre::RenderWindow* window, Ogre::CompositorChain* chain);

        /**
         * Init post effect's parameters; The method will be called delayed on the first update
         * Call it manually if you want to init the effect at the specific time
         * The material and compositor will be created when the Init() is called
         */
        void Init(Ogre::MaterialPtr & material)
        {
            if (false == mInited)
            {
                //init the effect
                DoInit(material);
                mInited = true;
            }
            else
            {
                //throw std::logic_error("PostEffect[Init]: second initialization attempt");
                OGRE_EXCEPT(Ogre::Exception::ERR_INVALID_STATE, "Second initialization attempt", "PostEffect[Init]");
            }
        }

        /**
         * Update the effects parameters
         * The first call will cause calling Init() method
         * @param material pointer of the local copy of the effect material (unique for the every PostEffect instance)
         * @param time time in seconds spent after the effect begin
         */
        void Update(Ogre::MaterialPtr & material, Ogre::Real time)
        {
            if (false == mInited)
            {
                Init(material);
            }
            DoUpdate(material, time);
        }
        /**
         * Enables/disables the post effect
         */
        void SetEnabled(bool enabled)
        {
            if (nullptr == mCompositorInstance)
            {
                //throw std::runtime_error("PostEffect[SetEnabled]: the effect was not initialized");
                OGRE_EXCEPT(Ogre::Exception::ERR_INVALID_STATE, "The effect was not initialized", "PostEffect[SetEnabled]");
            }
            mCompositorInstance->setEnabled(enabled);
        }

        /**
         *	Get effect type
         *  It is used as unique identificator for effects factory and manager
         */
        const Ogre::String & GetTypeName() const
        {
            return mTypeName;
        }

        const Ogre::String & GetName() const
        {
            return mName;
        }

        /**
         * Is called on the every frame before rendering compositor pass
         * Update effect here
         */
        virtual void notifyMaterialRender(Ogre::uint32 pass_id, Ogre::MaterialPtr & mat) override;

    };

} //namespace OgreEffect
#endif
