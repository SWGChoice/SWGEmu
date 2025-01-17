/*
 * SurveyMissionObjectiveImplementation.cpp
 *
 *  Created on: 22/06/2010
 *      Author: victor
 */

#include "server/zone/objects/mission/SurveyMissionObjective.h"
#include "server/zone/objects/mission/MissionObserver.h"
#include "server/zone/objects/mission/MissionObject.h"
#include "server/zone/objects/scene/ObserverEventType.h"
#include "server/zone/objects/creature/CreatureObject.h"
#include "server/zone/objects/resource/ResourceSpawn.h"
#include "server/zone/managers/object/ObjectManager.h"
#include "server/zone/managers/mission/MissionManager.h"
#include "server/zone/ZoneServer.h"

void SurveyMissionObjectiveImplementation::activate() {
	MissionObjectiveImplementation::activate();

	if (hasObservers()) {
		return;
	}

	ManagedReference<CreatureObject*> player = getPlayerOwner();
	if (player != NULL) {
		ManagedReference<MissionObserver*> observer = new MissionObserver(_this.getReferenceUnsafeStaticCast());
		addObserver(observer, true);

		player->registerObserver(ObserverEventType::SURVEY, observer);
	}
}

void SurveyMissionObjectiveImplementation::abort() {
	MissionObjectiveImplementation::abort();

	if (!hasObservers())
		return;

	ManagedReference<MissionObserver*> observer = getObserver(0);

	ManagedReference<CreatureObject*> player = getPlayerOwner();
	if (player != NULL) {
		player->dropObserver(ObserverEventType::SURVEY, observer);

		dropObserver(observer, true);
	}
}

void SurveyMissionObjectiveImplementation::complete() {
	MissionObjectiveImplementation::complete();
}

int SurveyMissionObjectiveImplementation::notifyObserverEvent(MissionObserver* observer, uint32 eventType, Observable* observable, ManagedObject* arg1, int64 arg2) {
	if (eventType == ObserverEventType::SURVEY) {
		ManagedReference<CreatureObject*> player = getPlayerOwner();
		if (player == NULL) {
			return 0;
		}

		ResourceSpawn* sampledSpawn = cast<ResourceSpawn*>( arg1);

		int sampledDensity = (int)arg2;
		if (sampledSpawn->getSurveyMissionSpawnFamilyName() == spawnFamily && (sampledDensity >= efficiency)) {
			Vector3 startPosition;
			startPosition.setX(mission.get()->getStartPositionX());
			startPosition.setY(mission.get()->getStartPositionY());
			float distance = startPosition.distanceTo(player->getWorldPosition());
			if (distance > 500.0f) {
				complete();

				return 1;
			} else {
				StringIdChatParameter stringId("mission/mission_generic", "survey_too_close");
				stringId.setDI(500);
				stringId.setDF(distance);
				player->sendSystemMessage(stringId);

				return 0;
			}
		} else {
			return 0;
		}
	}

	dropObserver(observer, true);

	updateToDatabase();

	return 1;
}
