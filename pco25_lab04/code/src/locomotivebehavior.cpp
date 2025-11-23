//  /$$$$$$$   /$$$$$$   /$$$$$$         /$$$$$$   /$$$$$$   /$$$$$$  /$$$$$$$ 
// | $$__  $$ /$$__  $$ /$$__  $$       /$$__  $$ /$$$_  $$ /$$__  $$| $$____/ 
// | $$  \ $$| $$  \__/| $$  \ $$      |__/  \ $$| $$$$\ $$|__/  \ $$| $$      
// | $$$$$$$/| $$      | $$  | $$        /$$$$$$/| $$ $$ $$  /$$$$$$/| $$$$$$$ 
// | $$____/ | $$      | $$  | $$       /$$____/ | $$\ $$$$ /$$____/ |_____  $$
// | $$      | $$    $$| $$  | $$      | $$      | $$ \ $$$| $$       /$$  \ $$
// | $$      |  $$$$$$/|  $$$$$$/      | $$$$$$$$|  $$$$$$/| $$$$$$$$|  $$$$$$/
// |__/       \______/  \______/       |________/ \______/ |________/ \______/ 


#include "locomotivebehavior.h"
#include "ctrain_handler.h"

void LocomotiveBehavior::run()
{
    //Initialisation de la locomotive
    loco.allumerPhares();
    loco.demarrer();
    loco.afficherMessage("Ready!");

    /* A vous de jouer ! */

    // Vous pouvez appeler les méthodes de la section partagée comme ceci :
    //sharedSection->access(loco, direction);
    //sharedSection->leave(loco, direction);
    //sharedSection->stopAtStation(loco);

    while(loco.vitesse() != 0) {
        // On attend qu'une locomotive arrive sur le contact 1.
        // Pertinent de faire ça dans les deux threads? Pas sûr...
        if (nbDirectionChange % 2 == 0) {
            contactSuccession(d1Points, SharedSectionInterface::Direction::D1);
        } else {
            contactSuccession(d2Points, SharedSectionInterface::Direction::D2);
        }
        attendre_contact(changementDeSens);
        nbDirectionChange++;
        loco.inverserSens();
    }

    // Si la vitesse est nulle, c'est qu'il y a eu arrêt d'urgence
    sharedSection->stopAll();
}

void LocomotiveBehavior::contactSuccession(std::array<int, 4> points, SharedSectionInterface::Direction direction) {
    attendre_contact(points.at(0));
    sharedSection->access(loco, direction);
    if (direction == SharedSectionInterface::Direction::D1) {
        std::string message = "Changement d'aiguillage " + std::to_string(aiguillageEntree.at(0)) +
                      " en direction " + std::to_string(aiguillageEntree.at(1));
        afficher_message_loco(loco.numero(), message.c_str());
        diriger_aiguillage(aiguillageEntree.at(0), aiguillageEntree.at(1), 0);
    } else {
        diriger_aiguillage(aiguillageSortie.at(0), aiguillageSortie.at(1), 0);
    }
    attendre_contact(points.at(1));
    if (direction == SharedSectionInterface::Direction::D1) {
        std::string message = "Changement d'aiguillage " + std::to_string(aiguillageSortie.at(0)) +
                      " en direction " + std::to_string(aiguillageSortie.at(1));
        afficher_message_loco(loco.numero(), message.c_str());
        diriger_aiguillage(aiguillageSortie.at(0), aiguillageSortie.at(1), 0);
    } else {
        diriger_aiguillage(aiguillageEntree.at(0), aiguillageEntree.at(1), 0);
    }
    attendre_contact(points.at(2));
    sharedSection->leave(loco, direction);
    attendre_contact(points.at(3));
    sharedSection->release(loco);
}



void LocomotiveBehavior::printStartMessage()
{
    qDebug() << "[START] Thread de la loco" << loco.numero() << "lancé";
    loco.afficherMessage("Je suis lancée !");
}

void LocomotiveBehavior::printCompletionMessage()
{
    qDebug() << "[STOP] Thread de la loco" << loco.numero() << "a terminé correctement";
    loco.afficherMessage("J'ai terminé");
}
