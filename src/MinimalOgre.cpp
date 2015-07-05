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

#include "Shaders.h"

//-------------------------------------------------------------------------------------
MinimalOgre::MinimalOgre(void)
    : mRoot(0),
    mCamera(0),
    mSceneMgr(0),
    mWindow(0),
    mResourcesCfg(""),
    mPluginsCfg(""),
    mTrayMgr(0),
    mCameraMan(0),
    mDetailsPanel(0),
    mCursorWasVisible(false),
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
    if (mCameraMan) delete mCameraMan;
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
 
    mCameraMan = new OgreBites::SdkCameraMan(mCamera);   // create a default camera controller
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
    mTrayMgr = new OgreBites::SdkTrayManager("InterfaceName", mWindow, mInputContext, this);
    mTrayMgr->showFrameStats(OgreBites::TL_BOTTOMLEFT);
    mTrayMgr->showLogo(OgreBites::TL_BOTTOMRIGHT);
    mTrayMgr->hideCursor();
 
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
 
    mRoot->addFrameListener(this);
//-------------------------------------------------------------------------------------
    mRoot->startRendering();
 
    return true;
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
 
    if (!mTrayMgr->isDialogVisible())
    {
        mCameraMan->frameRenderingQueued(evt);   // if dialog isn't up, then update the camera
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
        if (mDetailsPanel->getTrayLocation() == OgreBites::TL_NONE)
        {
            mTrayMgr->moveWidgetToTray(mDetailsPanel, OgreBites::TL_TOPRIGHT, 0);
            mDetailsPanel->show();
        }
        else
        {
            mTrayMgr->removeWidgetFromTray(mDetailsPanel);
            mDetailsPanel->hide();
        }
    }
    else if (arg.key == OIS::KC_T)   // cycle texture filtering mode
    {
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
    }
    else if (arg.key == OIS::KC_R)   // cycle polygon rendering mode
    {
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
        mDetailsPanel->setParamValue(10, newVal);
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
 
    mCameraMan->injectKeyDown(arg);
    return true;
}
 
bool MinimalOgre::keyReleased( const OIS::KeyEvent &arg )
{
    mCameraMan->injectKeyUp(arg);
    return true;
}
 
bool MinimalOgre::mouseMoved( const OIS::MouseEvent &arg )
{
    if (mTrayMgr->injectPointerMove(arg)) return true;
    mCameraMan->injectPointerMove(arg);
    return true;
}
 
bool MinimalOgre::mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
	if (mTrayMgr->injectPointerDown(arg, id)) return true;
	mCameraMan->injectPointerDown(arg, id);
    return true;
}
 
bool MinimalOgre::mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
	if (mTrayMgr->injectPointerUp(arg, id)) return true;
	mCameraMan->injectPointerUp(arg, id);
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

        Ogre::Technique* technique = material->getTechnique(0);
        Ogre::Pass* pass = technique->getPass(0);

        {
            auto vprogram = Ogre::HighLevelGpuProgramManager::getSingleton().createProgram("Shader/Copy/V",
                Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, "glsl", Ogre::GPT_VERTEX_PROGRAM);
            vprogram->setSource(Shader_Copy_V);
            //auto vparams = vprogram->createParameters();
            //vparams->setNamedAutoConstant("modelview_matrix", Ogre::GpuProgramParameters::ACT_WORLDVIEW_MATRIX);
            //vparams->setNamedAutoConstant("projection_matrix", Ogre::GpuProgramParameters::ACT_PROJECTION_MATRIX);
        }

        {
            auto fprogram = Ogre::HighLevelGpuProgramManager::getSingleton().createProgram("Shader/Copy/F",
                Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, "glsl", Ogre::GPT_FRAGMENT_PROGRAM);
            fprogram->setSource(Shader_Copy_F);

            Ogre::Image image;
            image.load("spheremap.png", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
            auto texture = Ogre::TextureManager::getSingleton().loadImage("Tex", 
                Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, image);

            auto unit0 = pass->createTextureUnitState("Tex");
            unit0->setTextureAddressingMode(Ogre::TextureUnitState::TAM_CLAMP);
            unit0->setTextureFiltering(Ogre::TFO_NONE);
        }

        pass->setVertexProgram("Shader/Copy/V");
        pass->setFragmentProgram("Shader/Copy/F");

        //material->setDepthCheckEnabled(false);
        //material->setSceneBlending(Ogre::SBT_TRANSPARENT_ALPHA);
        //material->setCullingMode(Ogre::CULL_NONE);
        //material->setColourWriteEnabled(true);
	}


}

void MinimalOgre::SetupScene()
{
	mOgreHead = mSceneMgr->createEntity("Head", "ogrehead.mesh");

	Ogre::SceneNode* headNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	headNode->attachObject(mOgreHead);

	// Set ambient light
	mSceneMgr->setAmbientLight(Ogre::ColourValue(0.5, 0.5, 0.5));

	// Create a light
	Ogre::Light* l = mSceneMgr->createLight("MainLight");
	l->setPosition(20, 80, 50);


	//Create compositor
	Ogre::CompositorPtr compositor = Ogre::CompositorManager::getSingleton().create("compositor", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
	Ogre::CompositionTechnique* technique = compositor->createTechnique();
	
	{
		Ogre::CompositionTechnique::TextureDefinition* textureBg = technique->createTextureDefinition("RT");
		textureBg->scope = Ogre::CompositionTechnique::TS_GLOBAL;
		textureBg->width  = mWindow->getWidth(); //same as render window
		textureBg->height = mWindow->getHeight(); //same as render window
		textureBg->formatList.push_back(Ogre::PixelFormat::PF_R8G8B8A8);
	}

	{
		Ogre::CompositionTargetPass* target = technique->createTargetPass();
		//Ogre::CompositionPass* pass = target->createPass();
		target->setInputMode(Ogre::CompositionTargetPass::IM_PREVIOUS);
		target->setOutputName("RT");
		//pass->setIdentifier(1);
	}
	{
		Ogre::CompositionTargetPass* target = technique->getOutputTargetPass();
		target->setInputMode(Ogre::CompositionTargetPass::IM_NONE);
		Ogre::CompositionPass* pass = target->createPass();
		pass->setInput(0, "RT");
		pass->setMaterialName("Material/Copy");
        //pass->setMaterialName("BaseWhite");
		//pass->setInput(1);
	}

	Ogre::CompositorInstance* compInstance = Ogre::CompositorManager::getSingleton().addCompositor(mCamera->getViewport(), "compositor");
	if (nullptr != compInstance)
	{
		compInstance->addListener(this);
		compInstance->setEnabled(true);
	}

}



 
