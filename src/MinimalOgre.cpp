/*
-----------------------------------------------------------------------------
Filename:    MinimalOgre.cpp
-----------------------------------------------------------------------------
 
This source file is part of the
   ___                 __    __ _ _    _ 
  /___\__ _ _ __ ___  / / /\ \ (_) | _(_)
 //  // _` | '__/ _ \ \ \/  \/ / | |/ / |
/ \_// (_| | | |  __/  \  /\  /| |   <| |
\___/ \__, |_|  \___|   \/  \/ |_|_|\_\_|
      |___/                              
      Tutorial Framework
      http://www.ogre3d.org/tikiwiki/
-----------------------------------------------------------------------------
*/
#include "MinimalOgre.h"

#include <OgreMeshManager.h>
#include <OgreHardwarePixelBuffer.h>
#include <OgreRenderTexture.h>
#include <OgreCompositorManager.h> 
#include <OgreCompositionTargetPass.h>
#include <OgreCompositionPass.h>
#include <OgreCompositor.h>
#include <OgreMaterialManager.h>
#include <OgreMaterial.h>
#include <OgreTechnique.h>
#include <OgrePass.h>
#include <OgreGpuProgramManager.h>
#include <OgreGpuProgram.h>
#include <OgreHighLevelGpuProgramManager.h>
#include <OgreHighLevelGpuProgram.h>

#include <OgreStringInterface.h>

#include <boost/algorithm/clamp.hpp>

#include "Shaders.h"

#include "effect/PostEffectManager.h"
#include "effect/PostEffect.h"

const Ogre::Real MinimalOgre::ROTATION_VELOCITY = static_cast<Ogre::Real>(100.0);
const Ogre::Real MinimalOgre::ZOOM_VELOCITY = static_cast<Ogre::Real>(1000.0);
const Ogre::Real MinimalOgre::HEAD_SCALE_MIN = static_cast<Ogre::Real>(0.1);
const Ogre::Real MinimalOgre::HEAD_SCALE_MAX = static_cast<Ogre::Real>(2.0);

//-------------------------------------------------------------------------------------
MinimalOgre::MinimalOgre(void)
    : mRoot(0),
    mCamera(0),
    mSceneMgr(0),
    mWindow(0),
    mResourcesCfg(""),
    mPluginsCfg(""),
    mTrayMgr(0),
    //mCameraMan(0),
    //mDetailsPanel(0),
    //mCursorWasVisible(false),
    mShutDown(false),
    mInputManager(0),
    mMouse(0),
    mKeyboard(0),
	mOverlaySystem(0)
{
}
//-------------------------------------------------------------------------------------
MinimalOgre::~MinimalOgre(void)
{
    if (mTrayMgr) delete mTrayMgr;
    //if (mCameraMan) delete mCameraMan;
	if (mOverlaySystem) delete mOverlaySystem;
 
    //Remove ourself as a Window listener
    Ogre::WindowEventUtilities::removeWindowEventListener(mWindow, this);
    windowClosed(mWindow);
    delete mRoot;
}
 
bool MinimalOgre::go(void)
{
#ifdef NDEBUG
	mResourcesCfg = DATA_DIR"/resources.cfg";
	mPluginsCfg = DATA_DIR"/plugins.cfg";
#else
	mResourcesCfg = DATA_DIR"/resources.cfg";
	mPluginsCfg = DATA_DIR"/plugins_d.cfg";
#endif
    // construct Ogre::Root
    mRoot = new Ogre::Root(mPluginsCfg);
 
//-------------------------------------------------------------------------------------
    // setup resources
    // Load resource paths from config file
    Ogre::ConfigFile cf;
    cf.load(mResourcesCfg);
 
    // Go through all sections & settings in the file
    Ogre::ConfigFile::SectionIterator seci = cf.getSectionIterator();
 
    Ogre::String secName, typeName, archName;
    while (seci.hasMoreElements())
    {
        secName = seci.peekNextKey();
        Ogre::ConfigFile::SettingsMultiMap *settings = seci.getNext();
        Ogre::ConfigFile::SettingsMultiMap::iterator i;
        for (i = settings->begin(); i != settings->end(); ++i)
        {
            typeName = i->first;
            archName = i->second;
            Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
                archName, typeName, secName);
        }
    }
//-------------------------------------------------------------------------------------
    // configure
    // Show the configuration dialog and initialise the system
    // You can skip this and use root.restoreConfig() to load configuration
    // settings if you were sure there are valid ones saved in ogre.cfg
    if(mRoot->restoreConfig() || mRoot->showConfigDialog())
    {
        // If returned true, user clicked OK so initialise
        // Here we choose to let the system create a default rendering window by passing 'true'
        mWindow = mRoot->initialise(true, "MinimalOgre Render Window");
    }
    else
    {
        return false;
    }
//-------------------------------------------------------------------------------------
    // choose scenemanager
    // Get the SceneManager, in this case a generic one
    mSceneMgr = mRoot->createSceneManager(Ogre::ST_GENERIC);
//-------------------------------------------------------------------------------------
	// initialize the OverlaySystem (changed for 1.9)
	mOverlaySystem = new Ogre::OverlaySystem();
    mSceneMgr->addRenderQueueListener(mOverlaySystem);
//-------------------------------------------------------------------------------------
    // create camera
    // Create the camera
    mCamera = mSceneMgr->createCamera("PlayerCam");
 
    // Position it at 500 in Z direction
    mCamera->setPosition(Ogre::Vector3(0,0,80));
    // Look back along -Z
    mCamera->lookAt(Ogre::Vector3(0,0,-300));
    mCamera->setNearClipDistance(5);
 
    //mCameraMan = new OgreBites::SdkCameraMan(mCamera);   // create a default camera controller
//-------------------------------------------------------------------------------------
    // create viewports
    // Create one viewport, entire window
    Ogre::Viewport* vp = mWindow->addViewport(mCamera);
    vp->setBackgroundColour(Ogre::ColourValue(0,0,0));
 
    // Alter the camera aspect ratio to match the viewport
    mCamera->setAspectRatio(
        Ogre::Real(vp->getActualWidth()) / Ogre::Real(vp->getActualHeight()));
//-------------------------------------------------------------------------------------
    // Set default mipmap level (NB some APIs ignore this)
    Ogre::TextureManager::getSingleton().setDefaultNumMipmaps(5);
//-------------------------------------------------------------------------------------
    // Create any resource listeners (for loading screens)
    //createResourceListener();
//-------------------------------------------------------------------------------------
    // load resources
    Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
//-------------------------------------------------------------------------------------
    // Create the scene
    CreateMaterials();
	SetupScene();
    SetupPostEffects();

//-------------------------------------------------------------------------------------
    //create FrameListener
    Ogre::LogManager::getSingletonPtr()->logMessage("*** Initializing OIS ***");
    OIS::ParamList pl;
    size_t windowHnd = 0;
    std::ostringstream windowHndStr;
 
    mWindow->getCustomAttribute("WINDOW", &windowHnd);
    windowHndStr << windowHnd;
    pl.insert(std::make_pair(std::string("WINDOW"), windowHndStr.str()));
 
    mInputManager = OIS::InputManager::createInputSystem( pl );
 
    mKeyboard = static_cast<OIS::Keyboard*>(mInputManager->createInputObject( OIS::OISKeyboard, true ));
    mMouse = static_cast<OIS::Mouse*>(mInputManager->createInputObject( OIS::OISMouse, true ));
 
    mMouse->setEventCallback(this);
    mKeyboard->setEventCallback(this);
 
    //Set initial mouse clipping size
    windowResized(mWindow);
 
    //Register as a Window listener
    Ogre::WindowEventUtilities::addWindowEventListener(mWindow, this);
 
    mInputContext.mKeyboard = mKeyboard;
    mInputContext.mMouse = mMouse;
    

    SetupEffectsGui();

    mRoot->addFrameListener(this);
//-------------------------------------------------------------------------------------
    mRoot->startRendering();
 
    return true;
}
 
void MinimalOgre::SetupEffectsGui()
{
    mTrayMgr = new OgreBites::SdkTrayManager("InterfaceName", mWindow, mInputContext, this);
    mTrayMgr->showFrameStats(OgreBites::TL_BOTTOMLEFT);
    mTrayMgr->showLogo(OgreBites::TL_BOTTOMRIGHT);
    mTrayMgr->showCursor();
    /*
    // create a params panel for displaying sample details
    Ogre::StringVector items;
    items.push_back("cam.pX");
    items.push_back("cam.pY");
    items.push_back("cam.pZ");
    items.push_back("");
    items.push_back("cam.oW");
    items.push_back("cam.oX");
    items.push_back("cam.oY");
    items.push_back("cam.oZ");
    items.push_back("");
    items.push_back("Filtering");
    items.push_back("Poly Mode");

    mDetailsPanel = mTrayMgr->createParamsPanel(OgreBites::TL_NONE, "DetailsPanel", 200, items);
    mDetailsPanel->setParamValue(9, "Bilinear");
    mDetailsPanel->setParamValue(10, "Solid");
    mDetailsPanel->hide();
    */

    //Effects menu
    static const size_t GUI_WIDTH = 175;

    mTrayMgr->createLabel(OgreBites::TL_TOPLEFT, "PageButton", "PostEffects", GUI_WIDTH);
    for (auto effectEntry : mPostEffects)
    {
        Ogre::String label = effectEntry.second->GetTypeName();
        size_t lastSeparatorIdx = label.find_last_of('/');
        if (std::string::npos != lastSeparatorIdx)
        {
            label = label.substr(lastSeparatorIdx + 1);
        }
        auto chBox = mTrayMgr->createCheckBox(OgreBites::TL_TOPLEFT, effectEntry.first, label, GUI_WIDTH);
        chBox->setChecked(false);
    }
}

void MinimalOgre::checkBoxToggled(OgreBites::CheckBox* box)
{
    Ogre::String chBoxName = box->getName();
    auto entryIt = mPostEffects.find(chBoxName);
    if (entryIt != mPostEffects.end())
    {
        assert(nullptr != entryIt->second);
        entryIt->second->SetEnabled(box->isChecked());
    }
}

bool MinimalOgre::frameRenderingQueued(const Ogre::FrameEvent& evt)
{
    if(mWindow->isClosed())
        return false;
 
    if(mShutDown)
        return false;
 
    //Need to capture/update each device
    mKeyboard->capture();
    mMouse->capture();
 
    mTrayMgr->frameRenderingQueued(evt);
    /*
    if (!mTrayMgr->isDialogVisible())
    {
        //mCameraMan->frameRenderingQueued(evt);   // if dialog isn't up, then update the camera
        if (mDetailsPanel->isVisible())   // if details panel is visible, then update its contents
        {
            mDetailsPanel->setParamValue(0, Ogre::StringConverter::toString(mCamera->getDerivedPosition().x));
            mDetailsPanel->setParamValue(1, Ogre::StringConverter::toString(mCamera->getDerivedPosition().y));
            mDetailsPanel->setParamValue(2, Ogre::StringConverter::toString(mCamera->getDerivedPosition().z));
            mDetailsPanel->setParamValue(4, Ogre::StringConverter::toString(mCamera->getDerivedOrientation().w));
            mDetailsPanel->setParamValue(5, Ogre::StringConverter::toString(mCamera->getDerivedOrientation().x));
            mDetailsPanel->setParamValue(6, Ogre::StringConverter::toString(mCamera->getDerivedOrientation().y));
            mDetailsPanel->setParamValue(7, Ogre::StringConverter::toString(mCamera->getDerivedOrientation().z));
        }
    }
    */
 
    return true;
}
//-------------------------------------------------------------------------------------
bool MinimalOgre::keyPressed( const OIS::KeyEvent &arg )
{
    if (mTrayMgr->isDialogVisible()) return true;   // don't process any more keys if dialog is up
 
    if (arg.key == OIS::KC_F)   // toggle visibility of advanced frame stats
    {
        mTrayMgr->toggleAdvancedFrameStats();
    }
    else if (arg.key == OIS::KC_G)   // toggle visibility of even rarer debugging details
    {
        /*
        if (mDetailsPanel->getTrayLocation() == OgreBites::TL_NONE)
        {
            mTrayMgr->moveWidgetToTray(mDetailsPanel, OgreBites::TL_TOPRIGHT, 0);
            mDetailsPanel->show();
        }
        else
        {
            mTrayMgr->removeWidgetFromTray(mDetailsPanel);
            mDetailsPanel->hide();
        }*/
    }
    else if (arg.key == OIS::KC_T)   // cycle texture filtering mode
    {
        /*
        Ogre::String newVal;
        Ogre::TextureFilterOptions tfo;
        unsigned int aniso;
        switch (mDetailsPanel->getParamValue(9).asUTF8()[0])
        {
        case 'B':
            newVal = "Trilinear";
            tfo = Ogre::TFO_TRILINEAR;
            aniso = 1;
            break;
        case 'T':
            newVal = "Anisotropic";
            tfo = Ogre::TFO_ANISOTROPIC;
            aniso = 8;
            break;
        case 'A':
            newVal = "None";
            tfo = Ogre::TFO_NONE;
            aniso = 1;
            break;
        default:
            newVal = "Bilinear";
            tfo = Ogre::TFO_BILINEAR;
            aniso = 1;
        }
        Ogre::MaterialManager::getSingleton().setDefaultTextureFiltering(tfo);
        Ogre::MaterialManager::getSingleton().setDefaultAnisotropy(aniso);
        mDetailsPanel->setParamValue(9, newVal);
        */
    }
    else if (arg.key == OIS::KC_R)   // cycle polygon rendering mode
    {
        /*
        Ogre::String newVal;
        Ogre::PolygonMode pm;
 
        switch (mCamera->getPolygonMode())
        {
        case Ogre::PM_SOLID:
            newVal = "Wireframe";
            pm = Ogre::PM_WIREFRAME;
            break;
        case Ogre::PM_WIREFRAME:
            newVal = "Points";
            pm = Ogre::PM_POINTS;
            break;
        default:
            newVal = "Solid";
            pm = Ogre::PM_SOLID;
        }
 
        mCamera->setPolygonMode(pm);
        //mDetailsPanel->setParamValue(10, newVal);
        */
    }
    else if(arg.key == OIS::KC_F5)   // refresh all textures
    {
        Ogre::TextureManager::getSingleton().reloadAll();
    }
    else if (arg.key == OIS::KC_SYSRQ)   // take a screenshot
    {
        mWindow->writeContentsToTimestampedFile("screenshot", ".jpg");
    }
    else if (arg.key == OIS::KC_ESCAPE)
    {
        mShutDown = true;
    }
 
    //mCameraMan->injectKeyDown(arg);
    return true;
}
 
bool MinimalOgre::keyReleased( const OIS::KeyEvent &arg )
{
    //mCameraMan->injectKeyUp(arg);
    return true;
}
 
bool MinimalOgre::mouseMoved( const OIS::MouseEvent &evt )
{
#if (OGRE_VERSION_MAJOR == 1) && (OGRE_VERSION_MINOR == 9) && (OGRE_VERSION_PATCH == 0)
    if (mTrayMgr->injectMouseMove(evt)) return true;
#else
    if (mTrayMgr->injectPointerMove(evt)) return true;
#endif
    Ogre::SceneNode* headNode = mOgreHead->getParentSceneNode();
    if (nullptr != headNode)
    {
        if (true == evt.state.buttonDown(OIS::MouseButtonID::MB_Left))
        {
            Ogre::Quaternion rotOY, rotOX;
            rotOY.FromAngleAxis(Ogre::Radian(evt.state.X.rel / ROTATION_VELOCITY), Ogre::Vector3::UNIT_Y);
            rotOX.FromAngleAxis(Ogre::Radian(evt.state.Y.rel / ROTATION_VELOCITY), Ogre::Vector3::UNIT_X);
            Ogre::Quaternion oldOrientation = headNode->getOrientation();
            headNode->setOrientation(rotOY * rotOX * oldOrientation);
        }
        //The scroll wheel is actually considered mouse motion, it's the z component of the motion.
        //http://www.ogre3d.org/forums/viewtopic.php?t=36681
        if (0 != evt.state.Z.rel)
        {
            Ogre::Real scaleFactor = static_cast<Ogre::Real>(1) + evt.state.Z.rel / ZOOM_VELOCITY;
            Ogre::Real oldScale = headNode->getScale().length();
            scaleFactor = boost::algorithm::clamp(scaleFactor, HEAD_SCALE_MIN / oldScale, HEAD_SCALE_MAX / oldScale);
            headNode->scale(scaleFactor * Ogre::Vector3::UNIT_SCALE);
        }
    }

    return true;
}
 
bool MinimalOgre::mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
#if (OGRE_VERSION_MAJOR == 1) && (OGRE_VERSION_MINOR == 9) && (OGRE_VERSION_PATCH == 0)
    if (mTrayMgr->injectMouseDown(arg, id)) return true;
#else
	if (mTrayMgr->injectPointerDown(arg, id)) return true;
#endif
    return true;
}
 
bool MinimalOgre::mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
#if (OGRE_VERSION_MAJOR == 1) && (OGRE_VERSION_MINOR == 9) && (OGRE_VERSION_PATCH == 0)
    if (mTrayMgr->injectMouseUp(arg, id)) return true;
#else
	if (mTrayMgr->injectPointerUp(arg, id)) return true;
#endif
    return true;
}
 
//Adjust mouse clipping area
void MinimalOgre::windowResized(Ogre::RenderWindow* rw)
{
    unsigned int width, height, depth;
    int left, top;
    rw->getMetrics(width, height, depth, left, top);
 
    const OIS::MouseState &ms = mMouse->getMouseState();
    ms.width = width;
    ms.height = height;
}
 
//Unattach OIS before window shutdown (very important under Linux)
void MinimalOgre::windowClosed(Ogre::RenderWindow* rw)
{
    //Only close for window that created OIS (the main window in these demos)
    if( rw == mWindow )
    {
        if( mInputManager )
        {
            mInputManager->destroyInputObject( mMouse );
            mInputManager->destroyInputObject( mKeyboard );
 
            OIS::InputManager::destroyInputSystem(mInputManager);
            mInputManager = 0;
        }
    }
}

Ogre::RenderTarget* MinimalOgre::CreateRenderTarget(const Ogre::String & name, Ogre::Camera* camera, size_t width, size_t height)
{
	Ogre::TexturePtr rtTexture = Ogre::TextureManager::getSingleton().createManual(name,
		Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
		Ogre::TEX_TYPE_2D,
		width, height, 0, 
		Ogre::PF_BYTE_RGBA,
		Ogre::TU_RENDERTARGET);
	

	Ogre::RenderTarget* renderTarget = rtTexture->getBuffer()->getRenderTarget();
	renderTarget->addViewport(camera);
	renderTarget->setAutoUpdated(true);

	Ogre::Viewport* vp = renderTarget->getViewport(0);
	vp->setClearEveryFrame(true);
	vp->setOverlaysEnabled(true);
	vp->setBackgroundColour(Ogre::ColourValue::Black);

	return renderTarget;
}

void MinimalOgre::preRenderTargetUpdate(const Ogre::RenderTargetEvent& evt)
{
	//mOgreHead->setVisible(false);
	//mBgTexturePlane->setVisible(true);
}

void MinimalOgre::postRenderTargetUpdate(const Ogre::RenderTargetEvent& evt)
{
	//mOgreHead->setVisible(true);
	//mBgTexturePlane->setVisible(false);
}


void MinimalOgre::CreateMaterials()
{
	//Copy
    {
        Ogre::MaterialPtr material = Ogre::MaterialManager::getSingleton().create(
        "Material/Copy", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

        {
            Ogre::Technique* techniqueGL = material->getTechnique(0);
            Ogre::Pass* pass = techniqueGL->getPass(0);

            {
                auto vprogram = Ogre::HighLevelGpuProgramManager::getSingleton().createProgram("Shader/GL/Copy/V",
                    Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, "glsl", Ogre::GPT_VERTEX_PROGRAM);
                vprogram->setSource(Shader_GL_Copy_V);
                //auto vparams = vprogram->createParameters();
                //vparams->setNamedAutoConstant("modelviewproj", Ogre::GpuProgramParameters::ACT_WORLDVIEWPROJ_MATRIX);
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
        {
            Ogre::Technique* techniqueDX = material->createTechnique();
            Ogre::Pass* pass = techniqueDX->createPass();

            {
                auto vprogram = Ogre::HighLevelGpuProgramManager::getSingleton().createProgram("Shader/DX/Copy/V",
                    Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, "hlsl", Ogre::GPT_VERTEX_PROGRAM);
                vprogram->setSource(Shader_DX_Copy_V);
                vprogram->setParameter("target", "vs_3_0");
                vprogram->setParameter("entry_point", "VS");
                vprogram->load();
                if (true == vprogram->isSupported())
                {
                    auto vparams = vprogram->createParameters();
                    vparams->setNamedAutoConstant("ModelViewProj", Ogre::GpuProgramParameters::ACT_WORLDVIEWPROJ_MATRIX);
                }
            }

            {
                auto fprogram = Ogre::HighLevelGpuProgramManager::getSingleton().createProgram("Shader/DX/Copy/F",
                    Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, "hlsl", Ogre::GPT_FRAGMENT_PROGRAM);
                fprogram->setSource(Shader_DX_Copy_F);
                fprogram->setParameter("target", "ps_3_0");
                fprogram->setParameter("entry_point", "PS");
                fprogram->load();

                pass->createTextureUnitState();
            }

            pass->setVertexProgram("Shader/DX/Copy/V");
            pass->setFragmentProgram("Shader/DX/Copy/F");
        }
        material->load();
    }
    //Background material
    {
        //load image
        Ogre::Image bgImage;
        //bgImage.load("background.jpg", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
        //http://www.wallpaperup.com/176525/bridge_river_trees_landscape.html
        bgImage.load("bridge.jpg", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
        Ogre::TexturePtr bgTexture = Ogre::TextureManager::getSingleton().loadImage("Texture/BG",
            Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, bgImage);

        Ogre::MaterialPtr bgMaterial = Ogre::MaterialManager::getSingleton().getByName("Material/Copy")->clone("Material/BG");
        auto bgTexUnitState = bgMaterial->getBestTechnique()->getPass(0)->getTextureUnitState(0);
        bgTexUnitState->setTexture(bgTexture);
        bgTexUnitState->setTextureFiltering(Ogre::TFO_TRILINEAR);

        bgMaterial->load();
	}


}

void MinimalOgre::SetupScene()
{
	mOgreHead = mSceneMgr->createEntity("Head", "ogrehead.mesh");

	Ogre::SceneNode* headNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	headNode->attachObject(mOgreHead);
    headNode->setScale(0.27f * Ogre::Vector3::UNIT_SCALE);

	// Set ambient light
	mSceneMgr->setAmbientLight(Ogre::ColourValue(0.5, 0.5, 0.5));

	// Create a light
	Ogre::Light* l = mSceneMgr->createLight("MainLight");
	l->setPosition(20, 80, 50);

	Ogre::Plane bgPlane = Ogre::Plane(Ogre::Vector3(0.0f, 0.0f, 1.0f), Ogre::Vector3(0.0f, 0.0f, -100.0f));
	Ogre::vector<Ogre::Vector4>::type intersection;
	mCamera->forwardIntersect(bgPlane, &intersection);
    size_t width  = 2 * std::ceil(intersection[0].x);
	size_t height = 2 * std::ceil(intersection[0].y);

	Ogre::MeshPtr bgPlaneMesh = Ogre::MeshManager::getSingleton().createPlane("Mesh/BgPlane",
		Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, bgPlane, width, height);

	mBgTexturePlane = mSceneMgr->createEntity(bgPlaneMesh);
    mBgTexturePlane->setMaterialName("Material/BG");

	mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(mBgTexturePlane);
}

void MinimalOgre::SetupPostEffects()
{
    //OgreEffect::PostEffect* postEffect = OgreEffect::PostEffectManager::getSingleton().CreatePostEffect(OgreEffect::PostEffectManager::PE_NULL, mWindow, mCamera->getViewport());
    //OgreEffect::PostEffect* postEffect = OgreEffect::PostEffectManager::getSingleton().CreatePostEffect(OgreEffect::PostEffectManager::PE_FADING, mWindow, mCamera->getViewport());
    //OgreEffect::PostEffect* postEffect = OgreEffect::PostEffectManager::getSingleton().CreatePostEffect(OgreEffect::PostEffectManager::PE_BLUR, mWindow, mCamera->getViewport());
    //OgreEffect::PostEffect* postEffect = OgreEffect::PostEffectManager::getSingleton().CreatePostEffect("ComplexTest", mWindow, mCamera->getViewport());
    //if(nullptr != postEffect)
    //{
    //    postEffect->setParameter("color", "1 0 0");
    //    postEffect->SetEnabled(true);
    //}


    auto postEffects = OgreEffect::PostEffectManager::getSingleton().CreatePostEffectsChain({
#ifdef TEST_EFFECTS
        "DownsampleTest",
#endif
        OgreEffect::PostEffectManager::PE_BLACKWHITE,
        OgreEffect::PostEffectManager::PE_BLUR,
#ifdef TEST_EFFECTS
        "ComplexTest",
#endif
        OgreEffect::PostEffectManager::PE_BLOOM,
        OgreEffect::PostEffectManager::PE_GODRAYS,
        OgreEffect::PostEffectManager::PE_FADING,
        OgreEffect::PostEffectManager::PE_RAIN,
    }, mWindow, mCamera->getViewport(), true);

    for (OgreEffect::PostEffect* effect : postEffects)
    {
        mPostEffects["CheckBox/" + effect->GetName()] = effect;
    }

}

 
