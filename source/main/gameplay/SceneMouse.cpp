/*
	This source file is part of Rigs of Rods
	Copyright 2005-2012 Pierre-Michel Ricordel
	Copyright 2007-2012 Thomas Fischer
	Copyright 2013-2014 Petr Ohlidal

	For more information, see http://www.rigsofrods.com/

	Rigs of Rods is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License version 3, as
	published by the Free Software Foundation.

	Rigs of Rods is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Rigs of Rods. If not, see <http://www.gnu.org/licenses/>.
*/

/** 
	@file   SceneMouse.h
	@author Thomas Fischer <thomas@thomasfischer.biz>
	@date   11th of March 2011
*/

#include "SceneMouse.h"

#include "Beam.h"
#include "BeamFactory.h"
#include "CameraManager.h"
#include "Application.h"
#include "GUIManager.h"

#ifdef USE_MYGUI
# include <MyGUI.h>
#endif //USE_MYGUI
#include <OgreSceneManager.h>
#include <OgreMaterialManager.h>
#include <OgreResourceGroupManager.h>
#include <OgreTechnique.h>
#include <OgreManualObject.h>

using namespace Ogre;
using namespace RoR;

SceneMouse::SceneMouse()
{
	mouseGrabForce = 30000.0f;
	grab_truck     = NULL;
	
	// load 3d line for mouse picking
	pickLine =  gEnv->sceneManager->createManualObject("PickLineObject");
	pickLineNode = gEnv->sceneManager->getRootSceneNode()->createChildSceneNode("PickLineNode");

	MaterialPtr pickLineMaterial = MaterialManager::getSingleton().create("PickLineMaterial",ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
	pickLineMaterial->setReceiveShadows(false);
	pickLineMaterial->getTechnique(0)->setLightingEnabled(true);
	pickLineMaterial->getTechnique(0)->getPass(0)->setDiffuse(0,0,1,0);
	pickLineMaterial->getTechnique(0)->getPass(0)->setAmbient(0,0,1);
	pickLineMaterial->getTechnique(0)->getPass(0)->setSelfIllumination(0,0,1);

	pickLine->begin("PickLineMaterial", RenderOperation::OT_LINE_LIST);
	pickLine->position(0, 0, 0);
	pickLine->position(0, 0, 0);
	pickLine->end();
	pickLineNode->attachObject(pickLine);
	pickLineNode->setVisible(false);

	// init variables for mouse picking
	releaseMousePick();
}

SceneMouse::~SceneMouse()
{
	
	if (pickLineNode != nullptr)
	{
		gEnv->sceneManager->getRootSceneNode()->removeAndDestroyChild("PickLineNode");
		pickLineNode = nullptr;
	}
	

	if (pickLine != nullptr)
	{
		gEnv->sceneManager->destroyManualObject("PickLineObject");
		pickLine = nullptr;
	}
}

void SceneMouse::releaseMousePick()
{
	if (Application::GetGuiManager()->GetPauseMenuVisible()) return; //Stop everything when pause menu is visible

	// hide mouse line
	if (pickLineNode)
		pickLineNode->setVisible(false);

	// remove forces
	if (grab_truck)
		grab_truck->mouseMove(minnode, Vector3::ZERO, 0);

	// reset the variables
	minnode        = -1;
	grab_truck     = 0;
	mindist        = 99999;
	mouseGrabState = 0;
	lastgrabpos    = Vector3::ZERO;
	lastMouseX     = 0;
	lastMouseY     = 0;

	mouseGrabState = 0;
}

bool SceneMouse::mouseMoved(const OIS::MouseEvent& _arg)
{
	const OIS::MouseState ms = _arg.state;

	// check if handled by the camera
	if (!gEnv->cameraManager || gEnv->cameraManager->mouseMoved(_arg))
		return true;

	// experimental mouse hack
	if (ms.buttonDown(OIS::MB_Left) && mouseGrabState == 0)
	{
		lastMouseY = ms.Y.abs;
		lastMouseX = ms.X.abs;
		
		Ray mouseRay = getMouseRay();

		// walk all trucks
		Beam **trucks = BeamFactory::getSingleton().getTrucks();
		int trucksnum = BeamFactory::getSingleton().getTruckCount();
		minnode = -1;
		grab_truck = NULL;
		for (int i = 0; i < trucksnum; i++)
		{
			if (trucks[i] && trucks[i]->state <= DESACTIVATED)
			{
				// check if our ray intersects with the bounding box of the truck
				std::pair<bool, Real> pair = mouseRay.intersects(trucks[i]->boundingBox);
				if (!pair.first) continue;

				for (int j = 0; j < trucks[i]->free_node; j++)
				{
					if (trucks[i]->node_mouse_grab_disabled[j]) continue;

					// check if our ray intersects with the node
					std::pair<bool, Real> pair = mouseRay.intersects(Sphere(trucks[i]->nodes[j].smoothpos, 0.1f));
					if (pair.first)
					{
						// we hit it, check if its the nearest node
						if (pair.second < mindist)
						{
							mindist    = pair.second;
							minnode    = j;
							grab_truck = trucks[i];
							break;
						}
					}
				}
			}
			
			if (grab_truck) break;
		}

		// check if we hit a node
		if (grab_truck && minnode != -1)
		{
			mouseGrabState = 1;
			pickLineNode->setVisible(true);

			for (std::vector <hook_t>::iterator it = grab_truck->hooks.begin(); it!=grab_truck->hooks.end(); it++)
			{
				if (it->hookNode->id == minnode)
				{
					grab_truck->hookToggle(it->group, MOUSE_HOOK_TOGGLE, minnode);
				}
			}
		}
	} else if (ms.buttonDown(OIS::MB_Left) && mouseGrabState == 1)
	{
		// force applying and so forth happens in update()
		lastMouseY = ms.Y.abs;
		lastMouseX = ms.X.abs;
		// not fixed
		return false;

	} else if (!ms.buttonDown(OIS::MB_Left) && mouseGrabState == 1)
	{
		releaseMousePick();
		// not fixed
		return false;
	}

	return false;
}

void SceneMouse::update(float dt)
{
	if (Application::GetGuiManager()->GetPauseMenuVisible()) return; //Stop everything when pause menu is visible

	if (mouseGrabState == 1 && grab_truck)
	{
		// get values
		Ray mouseRay = getMouseRay();
		lastgrabpos = mouseRay.getPoint(mindist);

		// update visual line
		pickLine->beginUpdate(0);
		pickLine->position(grab_truck->nodes[minnode].AbsPosition);
		pickLine->position(lastgrabpos);
		pickLine->end();

		// add forces
		grab_truck->mouseMove(minnode, lastgrabpos, mouseGrabForce);
	}
}

bool SceneMouse::mousePressed(const OIS::MouseEvent& _arg, OIS::MouseButtonID _id)
{
	if (Application::GetGuiManager()->GetPauseMenuVisible()) return true; //Stop everything when pause menu is visible

	const OIS::MouseState ms = _arg.state;

	if (ms.buttonDown(OIS::MB_Middle))
	{
		if (gEnv->cameraManager && gEnv->cameraManager->getCurrentBehavior() == CameraManager::CAMERA_BEHAVIOR_VEHICLE)
		{
			Beam *truck = BeamFactory::getSingleton().getCurrentTruck();

			if (truck)
			{
				lastMouseY = ms.Y.abs;
				lastMouseX = ms.X.abs;
				Ray mouseRay = getMouseRay();

				Real nearest_camera_distance = std::numeric_limits<float>::max();
				Real nearest_ray_distance = std::numeric_limits<float>::max();
				int nearest_node_index = -1;

				for (int i = 0; i < truck->free_node; i++)
				{
					std::pair<bool, Real> pair = mouseRay.intersects(Sphere(truck->nodes[i].smoothpos, 0.25f));
					if (pair.first)
					{
						Real ray_distance = mouseRay.getDirection().crossProduct(truck->nodes[i].smoothpos - mouseRay.getOrigin()).length();
						if (ray_distance < nearest_ray_distance || (ray_distance == nearest_ray_distance && pair.second < nearest_camera_distance))
						{
							nearest_camera_distance = pair.second;
							nearest_ray_distance    = ray_distance;
							nearest_node_index      = i;
						}
					}
				}
				truck->m_custom_camera_node = nearest_node_index;
			}
		}
	}

	if (gEnv->cameraManager)
	{
		gEnv->cameraManager->mousePressed(_arg, _id);
	}

	return true;
}

bool SceneMouse::mouseReleased(const OIS::MouseEvent& _arg, OIS::MouseButtonID _id)
{
	if (Application::GetGuiManager()->GetPauseMenuVisible()) return true; //Stop everything when pause menu is visible

	if (mouseGrabState == 1)
	{
		releaseMousePick();
	}

	if (gEnv->cameraManager)
	{
		gEnv->cameraManager->mouseReleased(_arg, _id);
	}

	return true;
}

bool SceneMouse::keyPressed(const OIS::KeyEvent& _arg)
{
	return false;
}

bool SceneMouse::keyReleased(const OIS::KeyEvent& _arg)
{
	return false;
}

Ray SceneMouse::getMouseRay()
{
	Viewport *vp = gEnv->mainCamera->getViewport();

	return gEnv->mainCamera->getCameraToViewportRay((float)lastMouseX / (float)vp->getActualWidth(), (float)lastMouseY / (float)vp->getActualHeight());
}
