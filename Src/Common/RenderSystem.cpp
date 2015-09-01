#include "stdafx.h"
#include "RenderSystem.h"
#include "../Common/LogSystem.h"
#include "MagicListener.h"
#include "PointCloud.h"
#include "Mesh.h"

namespace MagicCore
{
	RenderSystem* RenderSystem::mpRenderSystem = NULL;

	RenderSystem::RenderSystem(void) : 
        mpRoot(NULL), 
        mpMainCamera(NULL), 
        mpRenderWindow(NULL), 
        mpSceneManager(NULL)
    {
    }

    RenderSystem* RenderSystem::Get()
    {
        if (mpRenderSystem == NULL)
        {
            mpRenderSystem = new RenderSystem;
        }
        return mpRenderSystem;
    }

    void RenderSystem::Init()
    {
        InfoLog << "RenderSystem init...." << std::endl;
        mpRoot = new Ogre::Root();
        bool hasConfig = false;
        if (mpRoot->restoreConfig())
        {
            hasConfig = true;
        }
        else if (mpRoot->showConfigDialog())
        {
            hasConfig = true;
        }

        if (hasConfig)
        {
            // initialise system according to user options.
            mpRenderWindow = mpRoot->initialise(true, "Magic3D");
            // Create the scene manager
            mpSceneManager = mpRoot->createSceneManager(Ogre::ST_GENERIC, "MainSceneManager");
            // Create and initialise the camera
            mpMainCamera = mpSceneManager->createCamera("MainCamera");
            SetupCameraDefaultParameter();
            // Create a viewport covering whole window
            Ogre::Viewport* viewPort = mpRenderWindow->addViewport(mpMainCamera);
            viewPort->setBackgroundColour(Ogre::ColourValue(0.8627450980392157, 0.8627450980392157, 0.8627450980392157));
            // Update the camera aspect ratio to that of the viewport
            mpMainCamera->setAspectRatio(Ogre::Real(viewPort->getActualWidth()) / Ogre::Real(viewPort->getActualHeight()));

            mpRoot->addFrameListener(MagicListener::Get());
            Ogre::WindowEventUtilities::addWindowEventListener(mpRenderWindow, MagicListener::Get());

            //get supported syntax information
            const Ogre::GpuProgramManager::SyntaxCodes &syntaxCodes = Ogre::GpuProgramManager::getSingleton().getSupportedSyntax();
            for (Ogre::GpuProgramManager::SyntaxCodes::const_iterator iter = syntaxCodes.begin();iter != syntaxCodes.end();++iter)
            {
                InfoLog << "supported syntax : " << (*iter) << std::endl;
            }
        }
    }

    void RenderSystem::SetupCameraDefaultParameter()
    {
        if (mpMainCamera != NULL)
        {
            mpMainCamera->setProjectionType(Ogre::PT_PERSPECTIVE);
            mpMainCamera->setPosition(0, 0, 4);
            mpMainCamera->lookAt(0, 0, 0);
            mpMainCamera->setNearClipDistance(0.05);
            mpMainCamera->setFarClipDistance(0);
            mpMainCamera->setAspectRatio((Ogre::Real)mpRenderWindow->getWidth() / (Ogre::Real)mpRenderWindow->getHeight());
        }
    }

    void RenderSystem::Update()
    {
        mpRoot->renderOneFrame();
    }

    Ogre::RenderWindow* RenderSystem::GetRenderWindow()
    {
        return mpRenderWindow;
    }

    Ogre::SceneManager* RenderSystem::GetSceneManager()
    {
        return mpSceneManager;
    }

    int RenderSystem::GetRenderWindowWidth()
    {
        if (mpRenderWindow != NULL)
        {
            return mpRenderWindow->getWidth();
        }
        else
        {
            return 0;
        }
    }
    
    int RenderSystem::GetRenderWindowHeight()
    {
        if (mpRenderWindow != NULL)
        {
            return mpRenderWindow->getHeight();
        }
        else
        {
            return 0;
        }
    }

    Ogre::Camera* RenderSystem::GetMainCamera()
    {
        return mpMainCamera;
    }

    void RenderSystem::RenderPointCloud(std::string pointCloudName, std::string materialName, const GPP::PointCloud* pointCloud)
    {
        if (mpSceneManager == NULL)
        {
            InfoLog << "Error: RenderSystem::mpSceneMagager is NULL when RenderPoingCloud" << std::endl;
            return;
        }
        Ogre::ManualObject* manualObj = NULL;
        if (mpSceneManager->hasManualObject(pointCloudName))
        {
            manualObj = mpSceneManager->getManualObject(pointCloudName);
            manualObj->clear();
        }
        else
        {
            manualObj = mpSceneManager->createManualObject(pointCloudName);
            if (mpSceneManager->hasSceneNode("ModelNode"))
            {
                mpSceneManager->getSceneNode("ModelNode")->attachObject(manualObj);
            }
            else
            {
                mpSceneManager->getRootSceneNode()->createChildSceneNode("ModelNode")->attachObject(manualObj);
            }
        }
        if (pointCloud->HasNormal())
        {
            int pointCount = pointCloud->GetPointCount();
            manualObj->begin(materialName, Ogre::RenderOperation::OT_POINT_LIST);
            for (int pid = 0; pid < pointCount; pid++)
            {
                GPP::Vector3 coord = pointCloud->GetPointCoord(pid);
                GPP::Vector3 normal = pointCloud->GetPointNormal(pid);
                GPP::Vector3 color = pointCloud->GetPointColor(pid);
                manualObj->position(coord[0], coord[1], coord[2]);
                manualObj->normal(normal[0], normal[1], normal[2]);
                manualObj->colour(color[0], color[1], color[2]);
            }
            manualObj->end();
        }
        else
        {
            int pointCount = pointCloud->GetPointCount();
            manualObj->begin(materialName, Ogre::RenderOperation::OT_POINT_LIST);
            for (int pid = 0; pid < pointCount; pid++)
            {
                GPP::Vector3 coord = pointCloud->GetPointCoord(pid);
                GPP::Vector3 color = pointCloud->GetPointColor(pid);
                manualObj->position(coord[0], coord[1], coord[2]);
                manualObj->colour(color[0], color[1], color[2]);
            }
            manualObj->end();
        }
    }

    void RenderSystem::RenderMesh(std::string meshName, std::string materialName, const GPP::TriMesh* mesh)
    {
        Ogre::ManualObject* manualObj = NULL;
        if (mpSceneManager->hasManualObject(meshName))
        {
            manualObj = mpSceneManager->getManualObject(meshName);
            manualObj->clear();
        }
        else
        {
            manualObj = mpSceneManager->createManualObject(meshName);
            if (mpSceneManager->hasSceneNode("ModelNode"))
            {
                mpSceneManager->getSceneNode("ModelNode")->attachObject(manualObj);
            }
            else
            {
                mpSceneManager->getRootSceneNode()->createChildSceneNode("ModelNode")->attachObject(manualObj);
            }
        }
        manualObj->begin(materialName, Ogre::RenderOperation::OT_TRIANGLE_LIST);
        int vertexCount = mesh->GetVertexCount();
        for (int vid = 0; vid < vertexCount; vid++)
        {
            GPP::Vector3 coord = mesh->GetVertexCoord(vid);
            GPP::Vector3 normal = mesh->GetVertexNormal(vid);
            GPP::Vector3 color = mesh->GetVertexColor(vid);
            manualObj->position(coord[0], coord[1], coord[2]);
            manualObj->normal(normal[0], normal[1], normal[2]);
            manualObj->colour(color[0], color[1], color[2]);
        }
        int triangleCount = mesh->GetTriangleCount();
        for (int fid = 0; fid < triangleCount; fid++)
        {
            int vertexIds[3];
            mesh->GetTriangleVertexIds(fid, vertexIds);
            manualObj->triangle(vertexIds[0], vertexIds[1], vertexIds[2]);
        }
        manualObj->end();
    }

    void RenderSystem::HideRenderingObject(std::string objName)
    {
        if (mpSceneManager != NULL)
        {
            if (mpSceneManager->hasManualObject(objName))
            {
                mpSceneManager->destroyManualObject(objName);
            }
        }
    }

    RenderSystem::~RenderSystem(void)
    {
    }
}