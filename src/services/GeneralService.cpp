/**
 * Author: Dominik Egger
 * Copyright: Distributed Organisms B.V. (DoBots)
 * Date: Oct 22, 2014
 * License: LGPLv3+, Apache License, or MIT, your choice
 */

#include <services/GeneralService.h>

using namespace BLEpp;

GeneralService::GeneralService(Nrf51822BluetoothStack &stack) : _temperatureCharacteristic(NULL), _stack(&stack){

	setUUID(UUID(GENERAL_UUID));
	setName("General Service");

	log(INFO, "Create general service");
	characStatus.push_back( { TEMPERATURE_UUID, true });
	characStatus.push_back( { CHANGE_NAME_UUID, true });
}

/**
 * Seperate function that actually adds the characteristics. This allows to introduce dependencies between construction
 * of the different services and the characteristics on those services.
 */
void GeneralService::addSpecificCharacteristics() {
	for ( CharacteristicStatusT &status : characStatus) {
		switch(status.UUID) {
		case TEMPERATURE_UUID: 
			if (status.enabled) {
				log(DEBUG, "Create characteristic %i to read temperature", TEMPERATURE_UUID);
				addTemperatureCharacteristic();
			} else {
				log(INFO, "Disabled temperature characteristic");
			}
		break;
		case CHANGE_NAME_UUID:
			if (status.enabled) {
				log(DEBUG, "Create characteristic %i to change BLE name", CHANGE_NAME_UUID);
				addChangeNameCharacteristic();
			} else {
				log(INFO, "Disabled change name characteristic");
			}
		break;
		}
	}
}

void GeneralService::addTemperatureCharacteristic() {
	_temperatureCharacteristic = new CharacteristicT<int32_t>();
	_temperatureCharacteristic->setUUID(UUID(getUUID(), TEMPERATURE_UUID));
	_temperatureCharacteristic->setName("Temperature");
	_temperatureCharacteristic->setDefaultValue(0);
	_temperatureCharacteristic->setNotifies(true);

	addCharacteristic(_temperatureCharacteristic);
}

void GeneralService::addChangeNameCharacteristic() {
	_changeNameCharacteristic = createCharacteristicRef<std::string>();
	(*_changeNameCharacteristic)
		.setUUID(UUID(getUUID(), CHANGE_NAME_UUID))
		.setName("Change Name")
		.setDefaultValue(getBLEName())
		.setWritable(true)
		.onWrite([&](const std::string& value) -> void {
			std::string name(value);
			log(INFO, "Set bluetooth name to: %s", name.c_str());
			setBLEName(name);
		})
		;
}

std::string & GeneralService::getBLEName() {
	_name = "not set";
	if (_stack) {
		_name = _stack->getDeviceName();
	}
	return _name;
}

void GeneralService::setBLEName(std::string &name) {
	if (name.length() > 31) {
		log(ERROR, "Name is too long");
		return;
	} 
	if (_stack) {
		_stack->updateDeviceName(name);
	}
}

GeneralService& GeneralService::createService(Nrf51822BluetoothStack& stack) {
	GeneralService* svc = new GeneralService(stack);
	stack.addService(svc);
	svc->addSpecificCharacteristics();
	return *svc;
}

void GeneralService::setTemperature(int32_t temperature) {
	*_temperatureCharacteristic = temperature;
}

void GeneralService::loop() {
	int32_t temp;
	temp = getTemperature();
	setTemperature(temp);
}
