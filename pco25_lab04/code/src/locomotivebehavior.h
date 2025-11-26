//  /$$$$$$$   /$$$$$$   /$$$$$$         /$$$$$$   /$$$$$$   /$$$$$$  /$$$$$$$ 
// | $$__  $$ /$$__  $$ /$$__  $$       /$$__  $$ /$$$_  $$ /$$__  $$| $$____/ 
// | $$  \ $$| $$  \__/| $$  \ $$      |__/  \ $$| $$$$\ $$|__/  \ $$| $$      
// | $$$$$$$/| $$      | $$  | $$        /$$$$$$/| $$ $$ $$  /$$$$$$/| $$$$$$$ 
// | $$____/ | $$      | $$  | $$       /$$____/ | $$\ $$$$ /$$____/ |_____  $$
// | $$      | $$    $$| $$  | $$      | $$      | $$ \ $$$| $$       /$$  \ $$
// | $$      |  $$$$$$/|  $$$$$$/      | $$$$$$$$|  $$$$$$/| $$$$$$$$|  $$$$$$/
// |__/       \______/  \______/       |________/ \______/ |________/ \______/ 

#ifndef LOCOMOTIVEBEHAVIOR_H
#define LOCOMOTIVEBEHAVIOR_H

#include "general.h"
#include "locomotive.h"
#include "launchable.h"
#include "sharedsectioninterface.h"

/**
 * @brief La classe LocomotiveBehavior représente le comportement d'une locomotive
 */
class LocomotiveBehavior : public Launchable
{
public:
    /*!
     * \brief locomotiveBehavior Constructeur de la classe
     * \param loco la locomotive dont on représente le comportement
     */
    LocomotiveBehavior(Locomotive& loco, std::shared_ptr<SharedSectionInterface> sharedSection,
        int arriveeD1, int arriveeD2, int releaseD1, int releaseD2, int lastD1, int lastD2,
        std::array<int, 2> aiguillageEntree, std::array<int, 2> aiguillageSortie, int changementDeSens):
        loco(loco),
        sharedSection(sharedSection)
    {
        // Eventuel code supplémentaire du constructeur
        d1Points = {arriveeD1, lastD1, arriveeD2, releaseD1};
        d2Points = {arriveeD2, lastD2, arriveeD1, releaseD2};
        this->aiguillageEntree = aiguillageEntree;
        this->aiguillageSortie= aiguillageSortie;
        this->changementDeSens = changementDeSens;
    }


protected:
    /*!
     * \brief run Fonction lancée par le thread, représente le comportement de la locomotive
     */
    void run() override;

    /*!
     * \brief printStartMessage Message affiché lors du démarrage du thread
     */
    void printStartMessage() override;

    /*!
     * \brief printCompletionMessage Message affiché lorsque le thread a terminé
     */
    void printCompletionMessage() override;

    void contactSuccession(std::array<int, 4> points, SharedSectionInterface::Direction direction);

    /**
     * @brief loco La locomotive dont on représente le comportement
     */
    Locomotive& loco;

    /**
     * @brief sharedSection Pointeur sur la section partagée
     */
    std::shared_ptr<SharedSectionInterface> sharedSection;

    /*
     * Vous êtes libres d'ajouter des méthodes ou attributs
     *
     * Par exemple la priorité ou le parcours
     */
    int nbDirectionChange{0};
    std::array<int, 4> d1Points{};
    std::array<int, 4> d2Points{};
    std::array<int, 2> aiguillageEntree{};
    std::array<int, 2> aiguillageSortie{};
    int changementDeSens{0};
};

#endif // LOCOMOTIVEBEHAVIOR_H
