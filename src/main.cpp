/**************************************************************************************************
Nom du fichier : main.cpp
Auteur : Mathieu Durand
Date de création : 2023/10/16

Description : Fichier principale Arduino
              
Notes : 

Modifications : 

***************************************************************************************************/

// *************************************************************************************************
//  INCLUDES
// *************************************************************************************************	

#include <Arduino.h>
#include <LibRobus.h>
#include "RobusDraw.h"
#include "PencilColor.h"

// *************************************************************************************************
//  CONSTANTES
// *************************************************************************************************
/* VIDE */

// *************************************************************************************************
//  FONCTIONS LOCALES
// *************************************************************************************************
/* VIDE */

// *************************************************************************************************
//  STRUCTURES ET UNIONS
// *************************************************************************************************
/* VIDE */

// *************************************************************************************************
// VARIABLES GLOBALES
// *************************************************************************************************
/* VIDE */

/**
 * @brief Initialisation du programme.
 * @author Mathieu Durand
 */
void setup()
{   
    BoardInit();
    Serial.begin(9600); // 115200 ou 9600
    RobusDraw::initialize(10);
    Serial.println("Successfully initialized SD card!");
    RobusDraw::setPrecision(0.5);
    RobusPosition::setCurveTightness(50);
    RobusPosition::setFollowAngularVelocityScale(3);
    RobusMovement::setPIDAngular(0.5, 0, 0.07, 0);

    bool success = RobusDraw::loadDrawing("fardx.txt");

    if (success) {
        Serial.println("Successfully loaded");
    } else {
        Serial.println("Failed to load");
    }

    RobusDraw::startDrawing();
    
    // Décommenter si le programme a absolument besoin du serial.
    //while(!Serial);
}

/**
 * @brief Boucle du programme.
 * @author Mathieu Durand
*/
void loop()
{
    RobusDraw::update();
}