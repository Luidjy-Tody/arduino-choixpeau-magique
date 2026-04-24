# TP_Choixpeau_Magique_Arduino

Projet Arduino réalisé dans le cadre du TP **Choixpeau Magique Interactif**.

## Description

Ce projet simule un choixpeau magique inspiré de l'univers de Harry Potter.

Le système utilise :

- une **carte Arduino Uno**
- un **écran LCD 16x2 avec module I2C**
- une **LED RGB à anode commune**
- **3 boutons poussoirs**
- un **photoresistor / LDR** sur **A0** pour le bonus

Le programme permet de trier l'utilisateur dans une maison de Poudlard :

- **Gryffondor**
- **Serpentard**
- **Serdaigle**
- **Poufsouffle**

## Fonctionnalités

Le projet contient :

- un **écran d'accueil**
- un **menu de sélection du mode**
- un **mode manuel**
- un **mode auto**
- un **affichage du résultat**
- une **LED RGB** qui change de couleur selon la maison obtenue
- une **gestion des boutons avec anti-rebond logiciel**

## Modes disponibles

### Mode manuel

En mode manuel, l'utilisateur répond à **4 questions**.

Chaque réponse ajoute un point à une maison.

À la fin, le programme compare les scores et affiche la maison gagnante sur le LCD.

### Mode auto

En mode auto, la maison est choisie automatiquement.

Dans la version de base, le choix est fait avec `random()`.

Dans la version bonus, le **photoresistor / LDR** branché sur **A0** peut être utilisé pour influencer ou déterminer le choix selon la lumière ambiante.

## Matériel utilisé

- 1 × Arduino Uno R3
- 1 × LCD 16x2 + module I2C
- 1 × LED RGB anode commune
- 3 × boutons poussoirs
- 3 × résistances pour la LED RGB
- 1 × photoresistor / LDR
- 1 × résistance 10 kΩ pour la LDR
- breadboard
- fils de connexion

## Branchement

### LCD I2C

- VCC → 5V
- GND → GND
- SDA → A4
- SCL → A5

### LED RGB anode commune

- patte commune → 5V
- R → résistance → D9
- G → résistance → D10
- B → résistance → D11

### Boutons

- Bouton **OK** → D2 et GND
- Bouton **HAUT** → D3 et GND
- Bouton **BAS** → D4 et GND

Les boutons sont utilisés avec `INPUT_PULLUP`.

### Photoresistor / LDR

Montage en pont diviseur :

- 5V → LDR → A0
- A0 → résistance 10 kΩ → GND

## Installation

Avant d’ouvrir ou de téléverser le projet, il faut installer **Arduino IDE** sur l’ordinateur.

### Étapes

1. Télécharger **Arduino IDE** depuis le site officiel Arduino
2. Installer le logiciel
3. Ouvrir le fichier du projet dans Arduino IDE
4. Installer la bibliothèque **LiquidCrystal I2C**
5. Sélectionner la bonne carte Arduino
6. Sélectionner le bon port
7. Compiler puis téléverser le code

## Structure logique du programme

Le programme est organisé avec une **machine à états finis (FSM)**.

Les états principaux sont :

- `ACCUEIL`
- `SELECTION_MODE`
- `MANUEL_QUESTION`
- `AUTO_CALCUL`
- `RESULTAT`

La boucle principale `loop()` appelle uniquement :

```cpp
updateFSM();
```

La fonction `updateFSM()` gère les transitions entre les différents états.

## Gestion des boutons

Les boutons utilisent :

- `INPUT_PULLUP`
- une fonction anti-rebond logiciel avec `millis()`

Cela permet d'éviter qu'un seul appui soit détecté plusieurs fois.

## Couleurs des maisons

- **Gryffondor** → rouge
- **Serpentard** → vert
- **Serdaigle** → bleu
- **Poufsouffle** → jaune

## Bibliothèques nécessaires

Pour le LCD I2C, il faut installer la bibliothèque :

- **LiquidCrystal I2C**

Dans Arduino IDE :

1. `Sketch`
2. `Include Library`
3. `Manage Libraries...`
4. Rechercher **LiquidCrystal I2C**
5. Installer la bibliothèque correspondante

## Téléversement

1. Ouvrir le projet dans **Arduino IDE**
2. Vérifier la bibliothèque LCD I2C
3. Sélectionner la bonne carte Arduino
4. Sélectionner le bon port
5. Compiler
6. Téléverser le code

## Remarque sur l'adresse I2C

Selon le module LCD, l'adresse peut être :

- `0x27`
- ou `0x3F`

Si l'écran n'affiche rien, il faut tester l'autre adresse dans cette ligne :

```cpp
LiquidCrystal_I2C lcd(0x27, 16, 2);
```

## Bonus

Le bonus du projet est l'utilisation du **photoresistor / LDR** sur **A0**.

Ce capteur permet de lire la lumière ambiante avec :

```cpp
analogRead(A0);
```

La valeur lue peut ensuite être utilisée pour :

- influencer le mode auto
- initialiser l'aléatoire
- créer une logique de tri basée sur la lumière

## Auteur

Projet réalisé par **Luidjy** dans le cadre du TP d'embarqué / Arduino en **L2 Informatique**.
