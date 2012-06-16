/*
 * VendorMenuComponent.cpp
 *
 *  Created on: 5/27/2012
 *      Author: kyle
 */

#include "server/zone/objects/creature/CreatureObject.h"
#include "VendorMenuComponent.h"
#include "server/zone/objects/scene/components/ObjectMenuComponent.h"
#include "server/zone/objects/scene/components/DataObjectComponentReference.h"
#include "server/zone/objects/tangible/components/vendor/VendorDataComponent.h"
#include "server/zone/packets/object/ObjectMenuResponse.h"
#include "server/zone/packets/chat/ChatSystemMessage.h"
#include "server/zone/objects/tangible/components/vendor/VendorDataComponent.h"
#include "server/zone/managers/vendor/VendorManager.h"

void VendorMenuComponent::fillObjectMenuResponse(SceneObject* sceneObject,
		ObjectMenuResponse* menuResponse, CreatureObject* player) {

	return;

	if(!sceneObject->isVendor())
		return;

	if(sceneObject->isASubChildOf(player)) {
		menuResponse->addRadialMenuItem(14, 3, "@ui:destroy");
		return;
	}

	DataObjectComponentReference* data = sceneObject->getDataObjectComponent();
	if(data == NULL || data->get() == NULL || !data->get()->isVendorData()) {
		return;
	}

	VendorDataComponent* vendorData = cast<VendorDataComponent*>(data->get());
	if(vendorData == NULL) {
		return;
	}

	if(vendorData->getOwnerId() != player->getObjectID())
		return;

	menuResponse->addRadialMenuItem(240, 3, "@player_structure:vendor_control");

	if (!vendorData->isInitialized()) {

		menuResponse->addRadialMenuItemToRadialID(240, 242, 3, "@player_structure:vendor_init");

		menuResponse->addRadialMenuItem(10, 3, "@ui_radial:item_pickup");

		menuResponse->addRadialMenuItem(51, 1, "@ui_radial:item_rotate"); //Rotate
		menuResponse->addRadialMenuItemToRadialID(51, 52, 3, "@ui_radial:item_rotate_left"); //Rotate Left
		menuResponse->addRadialMenuItemToRadialID(51, 53, 3, "@ui_radial:item_rotate_right"); //Rotate Right

	} else {

		menuResponse->addRadialMenuItemToRadialID(240, 241, 3, "@player_structure:vendor_status");
		menuResponse->addRadialMenuItemToRadialID(240, 246, 3, "@player_structure:change_name");

		if (vendorData->isVendorSearchEnabled())
			menuResponse->addRadialMenuItemToRadialID(240, 243, 3, "@player_structure:disable_vendor_search");
		else
			menuResponse->addRadialMenuItemToRadialID(240, 243, 3, "@player_structure:enable_vendor_search");

		if (!vendorData->isRegistered())
			menuResponse->addRadialMenuItemToRadialID(240, 244, 3, "@player_structure:register_vendor");
		else
			menuResponse->addRadialMenuItemToRadialID(240, 244, 3, "@player_structure:unregister_vendor");

	}

	menuResponse->addRadialMenuItemToRadialID(240, 245, 3, "@player_structure:remove_vendor");
}

int VendorMenuComponent::handleObjectMenuSelect(SceneObject* sceneObject,
		CreatureObject* player, byte selectedID) {

	return 0;

	if (!sceneObject->isVendor())
		return 0;

	DataObjectComponentReference* data = sceneObject->getDataObjectComponent();
	if(data == NULL || data->get() == NULL || !data->get()->isVendorData()) {
		return 0;
	}

	VendorDataComponent* vendorData = cast<VendorDataComponent*>(data->get());
	if(vendorData == NULL) {
		return 0;
	}

	if(vendorData->getOwnerId() != player->getObjectID()) {
		return TangibleObjectMenuComponent::handleObjectMenuSelect(sceneObject, player, selectedID);
	}

	ManagedReference<TangibleObject*> vendor = cast<TangibleObject*>(sceneObject);
	if(vendor == NULL)
		return 0;

	switch (selectedID) {
	case 241: {
		VendorManager::instance()->handleDisplayStatus(player, vendor);
		return 0;
	}

	case 242: {
		if (player->getRootParent().get() != vendor->getRootParent().get()) {
			player->sendSystemMessage("@player_structure:vendor_not_in_same_building");
			return 0;
		}

		if (vendorData->isInitialized()) {
			player->sendSystemMessage("@player_structure:vendor_already_initialized");
			return 0;
		}

		player->sendSystemMessage("@player_structure:vendor_initialized");
		vendorData->setInitialized(true);
		vendorData->runVendorUpdate();
		return 0;
	}

	case 243: {
		if (vendorData->isVendorSearchEnabled()) {
			vendorData->setVendorSearchEnabled(false);
			player->sendSystemMessage("@player_structure:vendor_search_disabled");
		} else {
			vendorData->setVendorSearchEnabled(true);
			player->sendSystemMessage("@player_structure:vendor_search_enabled");
		}

		return 0;
	}

	case 244: {
		if (!vendorData->isRegistered())
			VendorManager::instance()->sendRegisterVendorTo(player, vendor);
		else
			VendorManager::instance()->handleUnregisterVendor(player, vendor);
		return 0;
	}

	case 245: {
		VendorManager::instance()->promptDestroyVendor(player, vendor);
		return 0;
	}

	case 246: {
		VendorManager::instance()->promptRenameVendorTo(player, vendor);
		return 0;
	}

	default:
		return 0;
	};
}