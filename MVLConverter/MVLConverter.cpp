#define _CRT_SECURE_NO_WARNINGS

#include <string>
#include <iostream>
#include <filesystem>
#include <fstream>

#include "tinystr.h"
#include "tinyxml.h"

namespace fs = std::filesystem;
using namespace std;

int main()
{
    ofstream saveFile;
    saveFile.open("output.txt");
    if (!saveFile.is_open()) {
        cout << "The output.txt file is missing!\n";
        system("pause");
        return 0;
    }

    int CurrentVehicleID;
    cout << "Enter the starting vehicle ID: ";
    cin >> CurrentVehicleID;
    cout << "\n";

    //string folderPath = "C:\\MVLConverter\\Debug\\xml";
    string folderPath = "xml";
    for (const auto& entry : fs::directory_iterator(folderPath)) {
        char path[128];
        strcpy(path, entry.path().string().c_str());
        TiXmlDocument settingsFile(path);
        settingsFile.LoadFile();

        string fileNameString = entry.path().filename().string();
        fileNameString.erase(fileNameString.end() - 4, fileNameString.end());

        TiXmlElement* rootElement = settingsFile.RootElement();
        if (!rootElement) {
            cout << format("ERROR: Root element not found at {0}, skip this file\n\n", entry.path().filename().string());
            continue;
        }

        TiXmlElement* basic = rootElement->FirstChildElement("basic");
        if (basic) {
            const char* type = basic->FirstChildElement("type")->GetText();

            const char* name = basic->FirstChildElement("name")->GetText();
            char gameName[32];
            strcpy(gameName, name);
            for (int i = 0; i < 32; i++) {
                if (gameName[i] == ' ') gameName[i] = '_';
            }

            const char* anims = basic->FirstChildElement("anims")->GetText();
            const char* comprules = basic->FirstChildElement("comprules")->GetText();
            const char* wheelModel = "0";
            const char* wheelScale = "0";

            if (basic->FirstChildElement("wheelmodel"))
                wheelModel = basic->FirstChildElement("wheelmodel")->GetText();
            else if (basic->FirstChildElement("wheelrotangle"))
                wheelModel = basic->FirstChildElement("wheelrotangle")->GetText();

            if (basic->FirstChildElement("wheelscale"))
                wheelScale = basic->FirstChildElement("wheelscale")->GetText();

            TiXmlElement* aidata = rootElement->FirstChildElement("aidata");
            if (aidata) {
                const char* vehClass = aidata->FirstChildElement("class")->GetText();
                const char* frequency = aidata->FirstChildElement("freq")->GetText();
                const char* level = aidata->FirstChildElement("level")->GetText();
                saveFile << format("default, {0}, {1}, {1}, {2}, null, {5}, {3}, {4}, ", CurrentVehicleID, fileNameString, type, anims, vehClass, gameName);
                saveFile << format("{0}, {1}, {2}, {3}, {4}, PoliceRadio\n", frequency, level, comprules, wheelModel, wheelScale);
            } else {
                saveFile << format("default, {0}, {1}, {1}, {2}, null, {4}, {3}, ignore, ", CurrentVehicleID, fileNameString, type, anims, gameName);
                saveFile << format("10, 7, {0}, {1}, {2}, PoliceRadio\n", comprules, wheelModel, wheelScale);
            }
        }

        TiXmlElement* handling = rootElement->FirstChildElement("handling");
        if (handling) {
            const char* mass = handling->FirstChildElement("mass")->GetText();
            const char* percentSubmerged = handling->FirstChildElement("percentsubmerged")->GetText();
            const char* steeringLock = handling->FirstChildElement("steeringlock")->GetText();
            const char* seatOffset = handling->FirstChildElement("seatoffset")->GetText();
            const char* damageMultiplier = handling->FirstChildElement("damagemultiplier")->GetText();
            const char* monetaryValue = handling->FirstChildElement("value")->GetText();

            const char* flags = handling->FirstChildElement("flags")->GetText();
            // Remove zeros at the beginning of the line
            char vehicleFlags[8];
            bool bZeros = flags[0] == '0';
            int pos = 0;
            for (int i = 0; i < 8; i++) {
                vehicleFlags[i] = 0;

                if (!bZeros) {
                    vehicleFlags[pos] = flags[i];
                    pos++;
                    continue;
                }
                
                if (flags[i] == '0' && flags[i + 1] != '0')
                    bZeros = false;
            }
            if (vehicleFlags[0] == 0)
                vehicleFlags[0] = '0';

            TiXmlElement* dimensions = handling->FirstChildElement("dimensions");
            const char* dimX = dimensions->FirstChildElement("x")->GetText();
            const char* dimY = dimensions->FirstChildElement("y")->GetText();
            const char* dimZ = dimensions->FirstChildElement("z")->GetText();

            TiXmlElement* centreofmass = handling->FirstChildElement("centreofmass");
            const char* massX = centreofmass->FirstChildElement("x")->GetText();
            const char* massY = centreofmass->FirstChildElement("y")->GetText();
            const char* massZ = centreofmass->FirstChildElement("z")->GetText();

            const char* tractionMultiplier;
            const char* tractionLoss;
            const char* tractionBias;
            TiXmlElement* traction = handling->FirstChildElement("traction");
            TiXmlElement* boatSteering = handling->FirstChildElement("boatsteering");
            if (traction) {
                tractionMultiplier = traction->FirstChildElement("multiplier")->GetText();
                tractionLoss = traction->FirstChildElement("loss")->GetText();
                tractionBias = traction->FirstChildElement("bias")->GetText();
            } else if (boatSteering) {
                tractionMultiplier = boatSteering->FirstChildElement("bankforcemult")->GetText();
                tractionLoss = boatSteering->FirstChildElement("rudderturnforce")->GetText();
                tractionBias = boatSteering->FirstChildElement("speedsteerfalloff")->GetText();
            }

            TiXmlElement* transmission = handling->FirstChildElement("transmission");
            const char* numOfGears = transmission->FirstChildElement("numofgears")->GetText();
            const char* maxSpeed = transmission->FirstChildElement("maxspeed")->GetText();
            const char* acceleration = transmission->FirstChildElement("acceleration")->GetText();
            const char* driveType = transmission->FirstChildElement("drivetype")->GetText();
            const char* engineType = transmission->FirstChildElement("enginetype")->GetText();

            const char* brakeDeceleration;
            const char* brakeBias;
            const char* abs = "0";
            TiXmlElement* brakes = handling->FirstChildElement("brakes");
            TiXmlElement* boatBrakes = handling->FirstChildElement("boatbrakes");
            if (brakes) {
                brakeDeceleration = brakes->FirstChildElement("deceleration")->GetText();
                brakeBias = brakes->FirstChildElement("bias")->GetText();
                abs = brakes->FirstChildElement("abs")->GetText();
            } else if (boatBrakes) {
                brakeDeceleration = boatBrakes->FirstChildElement("verticalwavehitlimit")->GetText();
                brakeBias = boatBrakes->FirstChildElement("forwardwavehitbrake")->GetText();
            }

            const char* forceLevel;
            const char* dampening;
            const char* upperLimit;
            const char* lowerLimit;
            const char* suspensionBias;
            const char* antidive;
            TiXmlElement* suspension = handling->FirstChildElement("suspension");
            TiXmlElement* boatSuspension = handling->FirstChildElement("boatsuspension");
            if (suspension) {
                forceLevel = suspension->FirstChildElement("forcelevel")->GetText();
                dampening = suspension->FirstChildElement("dampening")->GetText();
                upperLimit = suspension->FirstChildElement("upperlimit")->GetText();
                lowerLimit = suspension->FirstChildElement("lowerlimit")->GetText();
                suspensionBias = suspension->FirstChildElement("bias")->GetText();
                antidive = suspension->FirstChildElement("antidive")->GetText();
            } else if (boatSuspension) {
                forceLevel = boatSuspension->FirstChildElement("waterresvolumemult")->GetText();
                dampening = boatSuspension->FirstChildElement("waterdampingmult")->GetText();
                upperLimit = boatSuspension->FirstChildElement("upperlimit")->GetText();
                lowerLimit = boatSuspension->FirstChildElement("handbrakedragmult")->GetText();
                suspensionBias = boatSuspension->FirstChildElement("sideslipforce")->GetText();
                antidive = boatSuspension->FirstChildElement("antidive")->GetText();
            }

            TiXmlElement* lights = handling->FirstChildElement("lights");
            const char* front = lights->FirstChildElement("front")->GetText();
            const char* rear = lights->FirstChildElement("rear")->GetText();

            saveFile << format("handling {0} {1} {2} {3} {4} {5} {6} {7} ", CurrentVehicleID, mass, dimX, dimY, dimZ, massX, massY, massZ);
            saveFile << format("{0} {1} {2} {3} {4} {5} ", percentSubmerged, tractionMultiplier, tractionLoss, tractionBias, numOfGears, maxSpeed);
            saveFile << format("{0} {1} {2} {3} {4} {5} ", acceleration, driveType, engineType, brakeDeceleration, brakeBias, abs);
            saveFile << format("{0} {1} {2} {3} {4} {5} ", steeringLock, forceLevel, dampening, seatOffset, damageMultiplier, monetaryValue);
            saveFile << format("{0} {1} {2} {3} {4} {5} {6}\n", upperLimit, lowerLimit, suspensionBias, antidive, vehicleFlags, front, rear);
        }

        TiXmlElement* bikehandling = rootElement->FirstChildElement("bikehandling");
        TiXmlElement* boathandling = rootElement->FirstChildElement("boathandling");
        TiXmlElement* aerohandling = rootElement->FirstChildElement("aerohandling");
        if (bikehandling) {
            const char* leanFwdCOM = bikehandling->FirstChildElement("leanfwdcom")->GetText();
            const char* leanFwdForce = bikehandling->FirstChildElement("leanfwdforce")->GetText();
            const char* leanBackCOM = bikehandling->FirstChildElement("leanbackcom")->GetText();
            const char* leanBackForce = bikehandling->FirstChildElement("leanbackforce")->GetText();
            const char* maxLean = bikehandling->FirstChildElement("maxlean")->GetText();
            const char* fullAnimLean = bikehandling->FirstChildElement("fullanimlean")->GetText();
            const char* desLean = bikehandling->FirstChildElement("deslean")->GetText();
            const char* speedSteer = bikehandling->FirstChildElement("speedsteer")->GetText();
            const char* slipSteer = bikehandling->FirstChildElement("slipsteer")->GetText();
            const char* noPlayerCOMZ = bikehandling->FirstChildElement("noplayercomz")->GetText();
            const char* wheelieAng = bikehandling->FirstChildElement("wheelieang")->GetText();
            const char* stoppieAng = bikehandling->FirstChildElement("stoppieang")->GetText();
            const char* wheelieSteer = bikehandling->FirstChildElement("wheeliesteer")->GetText();
            const char* wheelieStabMult = bikehandling->FirstChildElement("wheeliestabmult")->GetText();
            const char* stoppieStabMult = bikehandling->FirstChildElement("stoppiestabmult")->GetText();
            saveFile << format("handling2 {0} {1} {2} {3} {4} {5} ", CurrentVehicleID, leanFwdCOM, leanFwdForce, leanBackCOM, leanBackForce, maxLean);
            saveFile << format("{0} {1} {2} {3} {4} ", fullAnimLean, desLean, speedSteer, slipSteer, noPlayerCOMZ);
            saveFile << format("{0} {1} {2} {3} {4}\n", wheelieAng, stoppieAng, wheelieSteer, wheelieStabMult, stoppieStabMult);
        } else if (boathandling) {
            const char* thrustY = boathandling->FirstChildElement("thrusty")->GetText();
            const char* thrustZ = boathandling->FirstChildElement("thrustz")->GetText();
            const char* thrustAppZ = boathandling->FirstChildElement("thrustappz")->GetText();
            const char* aqPlaneForce = boathandling->FirstChildElement("aqplaneforce")->GetText();
            const char* aqPlaneLimit = boathandling->FirstChildElement("aqplanelimit")->GetText();
            const char* aqPlaneOffset = boathandling->FirstChildElement("aqplaneoffset")->GetText();
            const char* waveAudioMult = boathandling->FirstChildElement("waveaudiomult")->GetText();
            
            TiXmlElement* moveRes = boathandling->FirstChildElement("moveres");
            const char* moveResX = moveRes->FirstChildElement("x")->GetText();
            const char* moveResY = moveRes->FirstChildElement("y")->GetText();
            const char* moveResZ = moveRes->FirstChildElement("z")->GetText();

            TiXmlElement* turnRes = boathandling->FirstChildElement("turnres");
            const char* turnResX = turnRes->FirstChildElement("x")->GetText();
            const char* turnResY = turnRes->FirstChildElement("y")->GetText();
            const char* turnResZ = turnRes->FirstChildElement("z")->GetText();

            const char* lookLRBehindCamHeight = boathandling->FirstChildElement("looklrbcamheight")->GetText();

            saveFile << format("handling2 {0} {1} {2} {3} {4} ", CurrentVehicleID, thrustY, thrustZ, thrustAppZ, aqPlaneForce);
            saveFile << format("{0} {1} {2} {3} {4} ", aqPlaneLimit, aqPlaneOffset, waveAudioMult, moveResX, moveResY);
            saveFile << format("{0} {1} {2} {3} {4}\n", moveResZ, turnResX, turnResY, turnResZ, lookLRBehindCamHeight);
        } else if (aerohandling) {
            const char* thrust = aerohandling->FirstChildElement("thrust")->GetText();
            const char* thrustFallOff = aerohandling->FirstChildElement("thrustfalloff")->GetText();
            const char* yaw = aerohandling->FirstChildElement("yaw")->GetText();
            const char* yawStab = aerohandling->FirstChildElement("yawstab")->GetText();
            const char* sideSlip = aerohandling->FirstChildElement("sideslip")->GetText();
            const char* roll = aerohandling->FirstChildElement("roll")->GetText();
            const char* rollStab = aerohandling->FirstChildElement("rollstab")->GetText();
            const char* pitch = aerohandling->FirstChildElement("pitch")->GetText();
            const char* pitchStab = aerohandling->FirstChildElement("pitchstab")->GetText();
            const char* formLift = aerohandling->FirstChildElement("formlift")->GetText();
            const char* attackLift = aerohandling->FirstChildElement("attacklift")->GetText();
            const char* moveRes = aerohandling->FirstChildElement("moveres")->GetText();

            TiXmlElement* turnRes = aerohandling->FirstChildElement("turnres");
            const char* turnResX = turnRes->FirstChildElement("x")->GetText();
            const char* turnResY = turnRes->FirstChildElement("y")->GetText();
            const char* turnResZ = turnRes->FirstChildElement("z")->GetText();

            TiXmlElement* speedRes = aerohandling->FirstChildElement("speedres");
            const char* speedResX = speedRes->FirstChildElement("x")->GetText();
            const char* speedResY = speedRes->FirstChildElement("y")->GetText();
            const char* speedResZ = speedRes->FirstChildElement("z")->GetText();

            saveFile << format("handling2 {0} {1} {2} {3} {4} ", CurrentVehicleID, thrust, thrustFallOff, yaw, yawStab);
            saveFile << format("{0} {1} {2} {3} {4} {5} {6} ", sideSlip, roll, rollStab, pitch, pitchStab, formLift, attackLift);
            saveFile << format("{0} {1} {2} {3} {4} {5} {6}\n", moveRes, turnResX, turnResY, turnResZ, speedResX, speedResY, speedResZ);
        }

        TiXmlElement* carColors = rootElement->FirstChildElement("colors");
        if (carColors) {
            TiXmlElement* color = carColors->FirstChildElement("carcol");
            if (color) {
                saveFile << format("carcols, {}", CurrentVehicleID);
                for (color; color; color = color->NextSiblingElement("carcol")) {
                    saveFile << format(", {0}", color->GetText());
                }
                saveFile << "\n";
            } else {
                color = carColors->FirstChildElement("carcol4");
                if (color) {
                    saveFile << format("carcols4, {}", CurrentVehicleID);
                    for (color; color; color = color->NextSiblingElement("carcol4")) {
                        saveFile << format(", {0}", color->GetText());
                    }
                    saveFile << "\n";
                }
            }
        }

        TiXmlElement* audio = rootElement->FirstChildElement("audio");
        if (audio) {
            TiXmlElement* boatSound = audio->FirstChildElement("boatengine");
            if (boatSound) {
                const char* engineType = boatSound->FirstChildElement("type")->GetText();
                const char* baseVolume = boatSound->FirstChildElement("basevolume")->GetText();
                const char* baseFrequency = boatSound->FirstChildElement("basefrequency")->GetText();
                const char* volumeIncrease = boatSound->FirstChildElement("volumeincrease")->GetText();
                const char* frequencyIncrease = boatSound->FirstChildElement("frequencyincrease")->GetText();
                saveFile << format("sounds, {0}, {1}, {2}, ", CurrentVehicleID, baseVolume, volumeIncrease);
                saveFile << format("{0}, {1}, {2}\n", baseFrequency, frequencyIncrease, engineType);
            } else {
                const char* accelerationSampleIndex = audio->FirstChildElement("enginefarsample")->GetText();
                const char* bank = audio->FirstChildElement("enginenearsample")->GetText();
                const char* hornSample = audio->FirstChildElement("hornsample")->GetText();
                const char* hornFrequency = audio->FirstChildElement("hornfreq")->GetText();
                const char* sirenOrAlarmSample = audio->FirstChildElement("sirensample")->GetText();
                const char* sirenOrAlarmFrequency = audio->FirstChildElement("sirenfreq")->GetText();
                const char* doorType = audio->FirstChildElement("doorsounds")->GetText();
                saveFile << format("sounds, {0}, {1}, {2}, {3}, ", CurrentVehicleID, accelerationSampleIndex, bank, hornSample);
                saveFile << format("{0}, {1}, {2}, {3}\n", hornFrequency, sirenOrAlarmSample, sirenOrAlarmFrequency, doorType);
            }
        }

        saveFile << "\n";
        CurrentVehicleID++;
    }

    saveFile.close();

    cout << "The vehicle settings have been moved to the output.txt file\n";
    system("pause");
    return 0;
}