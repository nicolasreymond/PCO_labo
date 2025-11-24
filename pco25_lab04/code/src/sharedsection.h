//  /$$$$$$$   /$$$$$$   /$$$$$$         /$$$$$$   /$$$$$$   /$$$$$$  /$$$$$$$
// | $$__  $$ /$$__  $$ /$$__  $$       /$$__  $$ /$$$_  $$ /$$__  $$| $$____/
// | $$  \ $$| $$  \__/| $$  \ $$      |__/  \ $$| $$$$\ $$|__/  \ $$| $$
// | $$$$$$$/| $$      | $$  | $$        /$$$$$$/| $$ $$ $$  /$$$$$$/| $$$$$$$
// | $$____/ | $$      | $$  | $$       /$$____/ | $$\ $$$$ /$$____/ |_____  $$
// | $$      | $$    $$| $$  | $$      | $$      | $$ \ $$$| $$       /$$  \ $$
// | $$      |  $$$$$$/|  $$$$$$/      | $$$$$$$$|  $$$$$$/| $$$$$$$$|  $$$$$$/
// |__/       \______/  \______/       |________/ \______/ |________/ \______/


#ifndef SHAREDSECTION_H
#define SHAREDSECTION_H

#include <QDebug>
#include <unistd.h>

#include <pcosynchro/pcosemaphore.h>

#ifdef USE_FAKE_LOCO
#  include "fake_locomotive.h"
#else
#  include "locomotive.h"
#endif

#ifndef USE_FAKE_LOCO
#include "ctrain_handler.h"
#endif

#include "sharedsectioninterface.h"

/**
 * @brief La classe SharedSection implémente l'interface SharedSectionInterface qui
 * propose les méthodes liées à la section partagée.
 */
class SharedSection final : public SharedSectionInterface {
public:
    /**
     * @brief SharedSection Constructeur de la classe qui représente la section partagée.
     * Initialisez vos éventuels attributs ici, sémaphores etc.
     */
    SharedSection() = default;

    /**
     * @brief Request access to the shared section
     * @param loco Locomotive who asked access
     * @param d Direction of the locomotive
     */
    void access(Locomotive &loco, Direction d) override {
        mutex.acquire();
        auto it = locoState.find(loco.numero());
        if (it == locoState.end()) {
            locoState.emplace(loco.numero(), LocoInfo{d, LocoState::Access});
        } else {
            if (it->second.state == LocoState::Access) {
                errors++;
                mutex.release();
                return;
            }
            it->second.dir = d;
            it->second.state = LocoState::Access;
        }

        sameDirection = d == currentDirection;
        while (inUse) {
            loco.arreter();
            hasLocoWaiting = true;
            mutex.release();
            // Éviter les haut-le-coeur, on veut pas rendre les gens malades !
            sleep(2);
            mutex.acquire();
        }
        loco.demarrer(); // Ne fais rien si on avance déjà
        hasLocoWaiting = false;
        currentDirection = d;
        inUse = true;
        mutex.release();
    }

    /**
     * @brief Notify the shared section that a Locomotive has left (not freed yed).
     * @param loco Locomotive who left
     * @param d Direction of the locomotive
     */
    void leave(Locomotive &loco, Direction d) override {
        mutex.acquire();
        auto it = locoState.find(loco.numero());
        if (it == locoState.end()) {
            // Si la loco n'existe pas encore dans l'état, c'est une erreur (normalement impossible car on appelle leave après access)
            errors++;
            mutex.release();
            return;
        }
        if (it->second.dir != d || it->second.state != LocoState::Access) {
            // Si la loco n'était pas dans la bonne direction ou n'a pas appelé access en dernier, c'est une erreur
            errors++;
            mutex.release();
            return;
        }
        it->second.state = LocoState::Leave;

        if (hasLocoWaiting && sameDirection) {
            mutex.release();
            return;
        }
        inUse = false;
        sameDirection = false;
        mutex.release();
    }

    /**
     * @brief Notify the shared section that it can now be accessed again (freed).
     * @param loco Locomotive who sent the notification
     */
    void release(Locomotive &loco) override {
        mutex.acquire();
        auto it = locoState.find(loco.numero());
        if (it == locoState.end()) {
            // Si la loco n'existe pas encore dans l'état, c'est une erreur (normalement impossible car on appelle release après leave)
            errors++;
            mutex.release();
            return;
        }
        if (it->second.state != LocoState::Leave) {
            // Si la loco n'a pas appelé leave en dernier, c'est une erreur
            errors++;
            mutex.release();
            return;
        }
        it->second.state = LocoState::Release;

        if (hasLocoWaiting && sameDirection) {
            // Libérer la section partagée et réinitialiser le test de direction
            inUse = false;
            sameDirection = false;
        }
        mutex.release();
    }

    /**
     * @brief Stop all locomotives to access this shared section
     */
    void stopAll() override {
        // Useless ?
        /*
         * Comme j'ai rien à mettre ici, voici une petite histoire (vraiment rien de pertinent, à ne lire qu'en cas de temps à perdre) :
         *
         * C'est l'histoire d'un jeune paon. Il vivait dans la forêt avec sa famille. Un jour, il décida de partir à
         * l'aventure pour découvrir le monde. Il a voulu traverser des rails, quand il s'est rendu compte qu'il y avait déjà un autre paon !
         * TODO : À compléter (avant de faire les tests idéalement, question de priorité)
         */
    }

    /**
     * @brief Return nbErrors
     * @return nbErrors
     */
    int nbErrors() override {
        mutex.acquire();
        const int result = errors;
        mutex.release();
        return result;
    }

private:
    /*
     * Vous êtes libres d'ajouter des méthodes ou attributs
     * pour implémenter la section partagée.
     */
    enum class LocoState { Access, Leave, Release };

    struct LocoInfo {
        Direction dir;
        LocoState state;
    };

    bool inUse{false}, sameDirection{false}, hasLocoWaiting{false};
    Direction currentDirection{Direction::D1};

    PcoSemaphore sem{0};
    PcoSemaphore mutex{1};

    std::map<int, LocoInfo> locoState;
    int errors{0};
};


#endif // SHAREDSECTION_H
