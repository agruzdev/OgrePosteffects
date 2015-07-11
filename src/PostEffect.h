
#include <OgreStringInterface.h>
#include <OgreResource.h>
#include <OgreCompositor.h>
#include <OgreSharedPtr.h>
#include <OgreCompositorInstance.h>

#ifndef _POSTEFFECT_H_
#define _POSTEFFECT_H_

namespace Ogre
{
    class RenderWindow;
}

class PostEffect : public Ogre::StringInterface, public Ogre::CompositorInstance::Listener
{
    static const Ogre::String DICTIONARY_NAME;
    static const Ogre::String COMPOSITOR_NAME_PREFIX;
    //-------------------------------------------------------
    
    const Ogre::String mName; ///< Unique name of the post effect type
    const size_t mId; ///< Unique number of the post effect instance

    bool mInited;

    Ogre::CompositorPtr mCompositor;
    Ogre::CompositorInstance* mCompositorInstance;
    //-------------------------------------------------------

    //Basic initialization; then calls DoCreateParametersDictionary()
    void CreateParametersDictionary();
    //Helper method to generate unique names
    Ogre::String GetUniquePostfix() const;

    PostEffect(const PostEffect&) = delete;
    PostEffect(const PostEffect&&) = delete;
    PostEffect& operator=(const PostEffect&) = delete;
    PostEffect& operator=(const PostEffect&&) = delete;
    //-------------------------------------------------------

protected:
    /**
     * Create and setup post effect compositor
     * @return true if the created compositor has any supported technique
     */	
    bool CreateCompositor(const Ogre::RenderWindow* window, Ogre::CompositorChain* chain);

    //-------------------------------------------------------

    //Methods for implementing in the derived classes
    //
    //Setup dictionary values depending on the specific PostEffect implementation
    virtual void DoCreateParametersDictionary(Ogre::ParamDictionary* dictionary) {}
    //Effect specific implementations
    virtual bool DoInit(const Ogre::RenderWindow* window) { return true; }
    virtual void DoUpdate(Ogre::Real time) {}
    //-------------------------------------------------------

public:
    PostEffect(const Ogre::String & name, size_t id);
    ~PostEffect();

    bool Init(const Ogre::RenderWindow* window, Ogre::CompositorChain* chain)
    {
        mInited = false;
        if (true == DoInit(window))
        {
            if (true == CreateCompositor(window, chain))
            {
                mInited = true;
            }
        }
        return mInited;
    }

    void Update(Ogre::Real time)
    {
        if (true == mInited)
        {
            DoUpdate(time);
        }
    }

    void SetEnabled(bool enabled)
    {
        if (true == mInited && nullptr != mCompositorInstance)
        {
            mCompositorInstance->setEnabled(enabled);
        }
    }

    /**
     *	Get effects name
     *  It is used as unique identificator for effects factory and manager
     */
    const Ogre::String & GetName() const
    {
        return mName;
    }

    /**
     *	Get material used for post effect full screen plane
     *  It is supposed to get rendered scene image in the texture unit 0 and output processed image
     */
    virtual Ogre::String GetEffectMaterialName() const = 0;
};

#endif
