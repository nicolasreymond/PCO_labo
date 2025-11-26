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

    // Comportement principal, alterner les directions tant que la vitesse n'est pas nulle (urgence)
    while(loco.vitesse() != 0) {
        if (nbDirectionChange % 2 == 0) {
            contactSuccession(d1Points, aiguillageEntree, aiguillageSortie, SharedSectionInterface::Direction::D1);
        } else {
            contactSuccession(d2Points, aiguillageSortie, aiguillageEntree, SharedSectionInterface::Direction::D2);
        }
        attendre_contact(changementDeSens);
        nbDirectionChange++;
        loco.inverserSens();
    }

    // Si la vitesse est nulle, c'est qu'il y a eu arrêt d'urgence
    sharedSection->stopAll();
}

void LocomotiveBehavior::contactSuccession(const std::array<int, 4> &points, const std::array<int, 2> &in, const std::array<int, 2> &out, const SharedSectionInterface::Direction direction) const {
    // Premier point : demander l'accès et changer l'aiguillage d'entrée
    attendre_contact(points.at(0));
    sharedSection->access(loco, direction);
    diriger_aiguillage(in.at(0), in.at(1), 0);

    // Deuxième point : changer l'aiguillage de sortie
    attendre_contact(points.at(1));
    diriger_aiguillage(out.at(0), out.at(1), 0);

    // Troisième point : quitter la section partagée
    attendre_contact(points.at(2));
    sharedSection->leave(loco, direction);

    // Quatrième point : libérer la section partagée
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
