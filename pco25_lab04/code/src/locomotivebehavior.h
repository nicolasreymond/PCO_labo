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
class LocomotiveBehavior : public Launchable {
public:
    /*!
     * \brief locomotiveBehavior Constructeur de la classe
     * @param loco Référence vers la locomotive (non propriétaire).
     * @param sharedSection Section partagée.
     * @param p_arriveeD1,p_arriveeD2,p_releaseD1,p_releaseD2,p_lastD1,p_lastD2 Points de contact liés à la section
     * critique en fonction de la direction.
     * @param p_aiguillageEntree,p_aiguillageSortie {id,dir} aiguillages et sens associé.
     * @param p_changementDeSens Contact auquel la locomotive change de sens.
     */
    LocomotiveBehavior(Locomotive &loco, std::shared_ptr<SharedSectionInterface> sharedSection,
                       int p_arriveeD1, int p_arriveeD2, int p_releaseD1, int p_releaseD2, int p_lastD1, int p_lastD2,
                       std::array<int, 2> p_aiguillageEntree, std::array<int, 2> p_aiguillageSortie,
                       int p_changementDeSens) : loco(loco),
                                                 sharedSection(sharedSection),
                                                 d1Points{p_arriveeD1, p_lastD1, p_arriveeD2, p_releaseD1},
                                                 d2Points{p_arriveeD2, p_lastD2, p_arriveeD1, p_releaseD2},
                                                 aiguillageEntree(p_aiguillageEntree),
                                                 aiguillageSortie(p_aiguillageSortie),
                                                 changementDeSens(p_changementDeSens) {
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

    /**
     * @brief Exécute la séquence des 4 contacts pour traverser la section partagée.
     * @param points Tableau de 4 contacts (en fonction de la direction).
     * @param in Aiguillage d'entrée {id,dir}.
     * @param out Aiguillage de sortie {id,dir}.
     * @param direction Direction de traversée.
    */
    void contactSuccession(const std::array<int, 4> &points, const std::array<int, 2> &in,
                           const std::array<int, 2> &out, SharedSectionInterface::Direction direction) const;

    /**
     * @brief Loco La locomotive dont on représente le comportement
     */
    Locomotive &loco;

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
