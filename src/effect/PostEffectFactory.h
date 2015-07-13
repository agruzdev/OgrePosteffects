
#ifndef _POSTEFFECT_FACTORY_H_
#define _POSTEFFECT_FACTORY_H_

#include <assert.h>

#include "PostEffect.h"

namespace OgreEffect
{

    class PostEffectFactory
    {
    private:
        PostEffectFactory(const PostEffectFactory&) = delete;
        PostEffectFactory(const PostEffectFactory&&) = delete;
        PostEffectFactory& operator=(const PostEffectFactory&) = delete;
        PostEffectFactory& operator=(const PostEffectFactory&&) = delete;
        //-------------------------------------------------------

    protected:
        const Ogre::String mName;
        //-------------------------------------------------------

    public:
        /**
         *	@param name Name of creating effects
         */
        PostEffectFactory(const Ogre::String& name);

        virtual ~PostEffectFactory() = 0;
        /**
         *	Get effect unique name
         */
        virtual const Ogre::String & GetEffectName() const
        {
            return mName;
        }
        /**
         *	Create effect instance
         */
        virtual PostEffect* Create() const = 0;
        /**
         *	Destroy effect instance
         */
        virtual void Destroy(PostEffect* effect) const = 0;
    };


    template <class EffectType>
    class DefaultPostEffectFactory : public PostEffectFactory
    {
        mutable size_t mIdCounter = 0;
    public:
        DefaultPostEffectFactory(const Ogre::String & name) :
            PostEffectFactory(name)
        {
            assert(false == name.empty());
        }
        virtual PostEffect* Create() const override
        {
            return new EffectType(mName, mIdCounter++);
        }
        virtual void Destroy(PostEffect* effect) const override
        {
            delete static_cast<EffectType*>(effect);
        }
    };

}//namespace OgreEffect

#endif