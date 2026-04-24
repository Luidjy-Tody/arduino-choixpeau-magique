#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Constantes matérielles
const int BTN_OK   = 2;
const int BTN_HAUT = 3;
const int BTN_BAS  = 4;

const int LED_R = 9;
const int LED_G = 10;
const int LED_B = 11;

const int LDR_PIN = A0;

// LCD I2C
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Noms des maisons
const char* MAISONS[4] = {
  "Gryffondor",
  "Serpentard",
  "Serdaigle",
  "Poufsouffle"
};

// Etats de la machine
enum Etat {
  ACCUEIL,
  SELECTION_MODE,
  MANUEL_QUESTION,
  AUTO_CALCUL,
  RESULTAT
};

Etat etatCourant = ACCUEIL;

// Variables globales
int scores[4] = {0, 0, 0, 0};
int questionCourante = 0;
int maisonChoisie = -1;
int choixMenu = 0;
int choixReponse = 0;

unsigned long autoStartTime = 0;
unsigned long dernierAnimTime = 0;
int nbPointsAnim = 1;

// Questions du mode manuel
const int NB_QUESTIONS = 4;

struct Question {
  const char* texte;
  const char* rep1;
  const char* rep2;
  int maisonRep1;
  int maisonRep2;
};

Question questions[NB_QUESTIONS] = {
  {"Qualite pref?", "Courage",  "Sagesse",   0, 2},
  {"Face danger?",  "Foncer",   "Observer",  0, 2},
  {"Tu preferes?",  "Amis",     "Ambition",  3, 1},
  {"Ton style?",    "Loyaute",  "Ruse",      3, 1}
};

// Textes plus courts pour le LCD
const char* QUESTIONS_AFF[NB_QUESTIONS] = {
  "Qualite?",
  "Danger?",
  "Prefere?",
  "Style?"
};

const char* REP1_AFF[NB_QUESTIONS] = {
  "Courage",
  "Foncer",
  "Amis",
  "Loyaute"
};

const char* REP2_AFF[NB_QUESTIONS] = {
  "Sages",
  "Obser.",
  "Ambition",
  "Ruse"
};

// Prototypes
void updateFSM();

bool boutonAppuye(int pin);

void afficherAccueil();
void afficherMenu();
void afficherQuestion(int n);
void afficherAnimationAuto(int nbPoints);
void afficherResultat(int maison);

void eteindreLED();
void couleurModeManuel();
void couleurModeAuto();
void setCouleurMaison(int maison);

void resetScores();
int maisonGagnante();
int determinerMaisonAuto();

void setup() {
  pinMode(BTN_OK, INPUT_PULLUP);
  pinMode(BTN_HAUT, INPUT_PULLUP);
  pinMode(BTN_BAS, INPUT_PULLUP);

  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);

  Serial.begin(9600);

  lcd.init();
  lcd.backlight();

  randomSeed(analogRead(LDR_PIN));

  eteindreLED();
  afficherAccueil();

  Serial.println("Etat initial : ACCUEIL");
}

void loop() {
  updateFSM();
}

// Machine à états
void updateFSM() {
  switch (etatCourant) {

    case ACCUEIL:
      if (boutonAppuye(BTN_OK)) {
        choixMenu = 0;
        etatCourant = SELECTION_MODE;
        afficherMenu();
        couleurModeManuel();
        Serial.println("Passage au menu");
      }
      break;

    case SELECTION_MODE:
      if (boutonAppuye(BTN_HAUT)) {
        choixMenu = 0;
        afficherMenu();
        couleurModeManuel();
        Serial.println("> Manuel");
      }

      if (boutonAppuye(BTN_BAS)) {
        choixMenu = 1;
        afficherMenu();
        couleurModeAuto();
        Serial.println("> Auto");
      }

      if (boutonAppuye(BTN_OK)) {
        if (choixMenu == 0) {
          resetScores();
          questionCourante = 0;
          choixReponse = 0;
          etatCourant = MANUEL_QUESTION;
          afficherQuestion(questionCourante);
          Serial.println("Mode Manuel");
        } else {
          etatCourant = AUTO_CALCUL;
          autoStartTime = millis();
          dernierAnimTime = 0;
          nbPointsAnim = 1;
          afficherAnimationAuto(nbPointsAnim);
          Serial.println("Mode Auto");
        }
      }
      break;

    case MANUEL_QUESTION:
      if (boutonAppuye(BTN_HAUT)) {
        choixReponse = 0;
        afficherQuestion(questionCourante);
      }

      if (boutonAppuye(BTN_BAS)) {
        choixReponse = 1;
        afficherQuestion(questionCourante);
      }

      if (boutonAppuye(BTN_OK)) {
        if (choixReponse == 0) {
          scores[questions[questionCourante].maisonRep1]++;
        } else {
          scores[questions[questionCourante].maisonRep2]++;
        }

        questionCourante++;

        if (questionCourante < NB_QUESTIONS) {
          choixReponse = 0;
          afficherQuestion(questionCourante);
        } else {
          maisonChoisie = maisonGagnante();
          afficherResultat(maisonChoisie);
          setCouleurMaison(maisonChoisie);
          etatCourant = RESULTAT;

          Serial.print("Resultat Manuel -> ");
          Serial.println(MAISONS[maisonChoisie]);
        }
      }
      break;

    case AUTO_CALCUL:
      if (millis() - dernierAnimTime >= 300) {
        dernierAnimTime = millis();
        nbPointsAnim++;
        if (nbPointsAnim > 5) {
          nbPointsAnim = 1;
        }
        afficherAnimationAuto(nbPointsAnim);
      }

      if (millis() - autoStartTime >= 2000) {
        maisonChoisie = determinerMaisonAuto();
        afficherResultat(maisonChoisie);
        setCouleurMaison(maisonChoisie);
        etatCourant = RESULTAT;

        Serial.print("Mode Auto -> ");
        Serial.println(MAISONS[maisonChoisie]);
      }
      break;

    case RESULTAT:
      if (boutonAppuye(BTN_OK)) {
        eteindreLED();
        afficherAccueil();
        etatCourant = ACCUEIL;
        Serial.println("Retour accueil");
      }
      break;
  }
}

// Anti-rebond
bool boutonAppuye(int pin) {
  const unsigned long debounceDelay = 50;

  static int etatStableOK = HIGH;
  static int etatStableHaut = HIGH;
  static int etatStableBas = HIGH;

  static int dernierEtatLuOK = HIGH;
  static int dernierEtatLuHaut = HIGH;
  static int dernierEtatLuBas = HIGH;

  static unsigned long dernierTempsOK = 0;
  static unsigned long dernierTempsHaut = 0;
  static unsigned long dernierTempsBas = 0;

  int* etatStable;
  int* dernierEtatLu;
  unsigned long* dernierTemps;

  if (pin == BTN_OK) {
    etatStable = &etatStableOK;
    dernierEtatLu = &dernierEtatLuOK;
    dernierTemps = &dernierTempsOK;
  } else if (pin == BTN_HAUT) {
    etatStable = &etatStableHaut;
    dernierEtatLu = &dernierEtatLuHaut;
    dernierTemps = &dernierTempsHaut;
  } else {
    etatStable = &etatStableBas;
    dernierEtatLu = &dernierEtatLuBas;
    dernierTemps = &dernierTempsBas;
  }

  int lecture = digitalRead(pin);
  bool appuiDetecte = false;

  if (lecture != *dernierEtatLu) {
    *dernierTemps = millis();
    *dernierEtatLu = lecture;
  }

  if ((millis() - *dernierTemps) > debounceDelay) {
    if (lecture != *etatStable) {
      *etatStable = lecture;

      if (*etatStable == LOW) {
        appuiDetecte = true;
      }
    }
  }

  return appuiDetecte;
}

// Affichage LCD
void afficherAccueil() {
  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print("* Choixpeau *");
  lcd.setCursor(1, 1);
  lcd.print("[OK] Demarrer");
}

void afficherMenu() {
  lcd.clear();

  lcd.setCursor(0, 0);
  if (choixMenu == 0) {
    lcd.print(">MODE: MANUEL");
  } else {
    lcd.print(" MODE: MANUEL");
  }

  lcd.setCursor(0, 1);
  if (choixMenu == 1) {
    lcd.print(">MODE: AUTO");
  } else {
    lcd.print(" MODE: AUTO");
  }
}

void afficherQuestion(int n) {
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Q");
  lcd.print(n + 1);
  lcd.print("/");
  lcd.print(NB_QUESTIONS);
  lcd.print(" ");
  lcd.print(QUESTIONS_AFF[n]);

  lcd.setCursor(0, 1);

  if (choixReponse == 0) {
    lcd.print("<");
    lcd.print(REP1_AFF[n]);
    lcd.print("> ");
    lcd.print(REP2_AFF[n]);
  } else {
    lcd.print(REP1_AFF[n]);
    lcd.print(" <");
    lcd.print(REP2_AFF[n]);
    lcd.print(">");
  }
}

void afficherAnimationAuto(int nbPoints) {
  lcd.clear();
  lcd.setCursor(2, 0);
  lcd.print("Tri en cours");

  lcd.setCursor(5, 1);
  for (int i = 0; i < nbPoints; i++) {
    lcd.print(".");
  }
}

void afficherResultat(int maison) {
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("**");
  lcd.print(MAISONS[maison]);
  lcd.print("**");

  lcd.setCursor(0, 1);
  if (choixMenu == 0) {
    lcd.print("Score: ");
    lcd.print(scores[maison]);
    lcd.print(" pts");
  } else {
    lcd.print("OK pour retour");
  }
}

// LED RGB anode commune
void eteindreLED() {
  analogWrite(LED_R, 255);
  analogWrite(LED_G, 255);
  analogWrite(LED_B, 255);
}

void couleurModeManuel() {
  analogWrite(LED_R, 255);
  analogWrite(LED_G, 255);
  analogWrite(LED_B, 0);
}

void couleurModeAuto() {
  analogWrite(LED_R, 255);
  analogWrite(LED_G, 0);
  analogWrite(LED_B, 255);
}

void setCouleurMaison(int maison) {
  switch (maison) {
    case 0: // Gryffondor
      analogWrite(LED_R, 0);
      analogWrite(LED_G, 255);
      analogWrite(LED_B, 255);
      break;

    case 1: // Serpentard
      analogWrite(LED_R, 255);
      analogWrite(LED_G, 0);
      analogWrite(LED_B, 255);
      break;

    case 2: // Serdaigle
      analogWrite(LED_R, 255);
      analogWrite(LED_G, 255);
      analogWrite(LED_B, 0);
      break;

    case 3: // Poufsouffle
      analogWrite(LED_R, 0);
      analogWrite(LED_G, 0);
      analogWrite(LED_B, 255);
      break;
  }
}

// Fonctions utiles
void resetScores() {
  for (int i = 0; i < 4; i++) {
    scores[i] = 0;
  }
}

int maisonGagnante() {
  int indexMax = 0;

  for (int i = 1; i < 4; i++) {
    if (scores[i] > scores[indexMax]) {
      indexMax = i;
    }
  }

  return indexMax;
}

int determinerMaisonAuto() {
  return random(0, 4);
}