/**
 * @file symbole.cpp
 * @brief Les symboles que le chargeur voit de {{name}}.
 *
 * Ce fichier n'appartient qu'a la bibliotheque partagee : il est la seule
 * surface que dlsym atteint. Un nom de symbole brut est ce qu'il cherche,
 * il ne connait ni les namespaces ni le mangling C++, d'ou le extern "C"
 * et la portee globale.
 *
 * Tout le reste de {{name}} vit dans includes/, compile une seule fois par
 * l'etage objet, et se retrouve aussi bien dans la statique que dans la
 * partagee.
 *
 * @addtogroup {{name}}
 * @{
 */

extern "C" {

    //TODO : ce que le chargeur doit trouver ici

}

/** @} */
