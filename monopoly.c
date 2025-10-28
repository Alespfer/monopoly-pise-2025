#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h> 

//--- Valeurs constantes ---

#define MAX_PLAYERS 6
#define ARGENT_INITIAL 1500
#define ARGENT_DEPART 200
#define MAX_PROPRIETES 28
#define MAX_CARTES 16
#define NB_CASES 40
#define MAX_BEST_SCORE 10
#define NB_CASES_MILIEU 9
#define LARGEUR_CELL 16
#define SIDE_SPACE (((NB_CASES_MILIEU + 2) * (LARGEUR_CELL - 1) + 1) - 2 * LARGEUR_CELL)

//--- Préfixe général et macros ANSI ---
#define ECHAP_CODE "\033"
#define ECHAP_SEQ(seq) "\033[" seq

//--- Attributs de texte ---
#define TEXTE_ATTR(x) ECHAP_SEQ(#x) "m"

//--- Couleurs de l’arrière‑plan ---
#define FOND_NOIR TEXTE_ATTR(40)
#define FOND_ROUGE TEXTE_ATTR(41)
#define FOND_VERT TEXTE_ATTR(42)
#define FOND_JAUNE TEXTE_ATTR(43)
#define FOND_BLUE TEXTE_ATTR(44)
#define FOND_MAGENTA TEXTE_ATTR(45)
#define FOND_CYAN TEXTE_ATTR(46)
#define FOND_BLANC TEXTE_ATTR(47)

//--- Couleur du texte ---
#define TEXTE_NOIR TEXTE_ATTR(30)
#define TEXTE_BLANC TEXTE_ATTR(37)

//--- Remise à zéro des attributs ---
#define EFFACE_ATTR TEXTE_ATTR(0)


//--- STRUCTURES DE DONNÉES ---

struct Carte
{
    char type[20];         // Commuanuté/Chance
    int effet;             // Type d'effet
    int valeur;            // Somme à payer/recevoir
    char description[400]; // Description de l'effet
};

struct joueur
{
    char Nom[50];
    int argent;                     // Argent du joueur
    int position;                   // Position sur le plateau
    int nbProprietes;               // nb total de propriétés
    int proprietes[MAX_PROPRIETES]; // Tableau qui stocke les indices des cases des propriétés
    int enPrison;                   // Entier qui agit comme booléen
    int tourEnPrison;               // Tours passés en prison
    int cartesSortiePrison;         // nb total de cartes (0 - 2)
    int enJeu;                      // Entier booléen qui indique si le joueur reste dans le jeu (pour les éliminés)
    int libereChance;               // Entier booléen
    int libereCommunaute;           // Entier booléen
};


struct CasePlateau
{
    char nom[100];       // Nom des cases
    char nom_abrege[15]; // Nom abrégé
    char couleur;        // Un seul caractère qui représente l'initiale du groupe
    int prix_achat;      // Prix d'achat
    int prix_maison;     // Prix de construction
    int loyer;           // Loyer de base
    int loyer1m;         // Loyer 1 maison
    int loyer2m;
    int loyer3m;
    int loyer4m;
    int loyerh;
    int hypotheque;     // montant de l'hypothèque
    int Type;           // type d'effet
    int proprietaire;   // Indice du joueur propriétaire (player). Si prop libre il est égal à -1
    int estHypothequee; // Entier booléen qui indique si la prop est hypothéquée ou pas
    int nbMaisons;      // nb maisons construites
    int Hotel;          // Entier booléen (1 = présence d'hôtel)
};

struct ScoreEntry
{
    char nom[50];       // Nom du joueur
    int score;          // Score
};

struct joueur player[MAX_PLAYERS];
struct Carte cartes_chance[MAX_CARTES];
struct Carte cartes_communaute[MAX_CARTES];
struct CasePlateau plateau[NB_CASES];

//---VARIABLES GLOBALES UTILISEES---
int nbjoueurs = 0;                   // nb total de joueurs
int iChance = 0;                     // compteur des cartes chance
int iCommunaute = 0;                 // compteur des cartes communauté
int debut = 1;                       // entier booléen utilisé en début de partie (pour l'initialisation)
int jeufini = 0;                     // Entier booléen pour marquer la fin de la partie
int tour = 0;                        // Entier du tour courant
int nbCartesChance = MAX_CARTES;     
int nbCartesCommunaute = MAX_CARTES; 
int mode;                            // Mode de la partie
int nb_tours_total = 0;              // Pour sauvegarder l'état de la partie à nbd e tours limités
int tour_actuel = 0; 
int objectif_monetaire = 0;          // Pour mode 3 : objectif monétaire à atteindre 
int session_finie = 0;               // 0 = session active, 1 = session terminée par utilisateur

//--- Fonctions/procédures utilitaires de base ---
void afficherLogoMonopoly();
int lancer_de();
void conv_maj(char chaine[]);
void viderBuffer();
void quitter();

// --- Affichage d'Informations Joueur/Jeu ---
void afficherJoueur(const struct joueur *p);
void afficherBilan(const struct joueur *p, struct CasePlateau plateau[], int nbCases);
void afficherTableauScore(const struct joueur players[], int nbjoueurs, const struct CasePlateau plateau[], int nbCases);
void afficher_constructions_par_couleur(struct joueur *p, struct CasePlateau plateau[], int nbCases, int *propriete_indices, int *nbProps);

// --- Débogage ---
void afficher_plateau();
void afficher_cartes();


// --- Gestion des Scores ---
void afficherMeilleursScores();
void mettreAJourMeilleursScores();

// --- Initialisation du Jeu ---
void initialisation_plateau();
void initialisation_cartes();
void melanger_cartes(struct Carte cartes[], int taille);

// --- Gestion des Cartes (Pioche, Effets) ---
struct Carte retirerCarteCommunaute();
struct Carte retirerCarteChance();
void reintegrerCarteSortieDePrison(struct Carte carte);
void appliquer_effet_carte(struct Carte carte, struct joueur *joueur, struct joueur player[], int nbJoueurs);

// --- Fonctions Utilitaires pour Logique de Jeu ---
int calcul_gares(struct CasePlateau plateau[], int joueurActuel);
int tous_couleurs(struct CasePlateau plateau[], int nbCases, char couleur, int player_id);
int totalMaisons(struct joueur *p, struct CasePlateau plateau[], int nbCases);
int totalHotels(struct joueur *p, struct CasePlateau plateau[], int nbCases);
int groupe_est_nu(char couleur, struct CasePlateau plateau[], int nbCases);
int propriete_est_transférable(int idx, struct CasePlateau plateau[], int nbCases);

// --- Logique de Case & Propriété ---
void enchere(struct CasePlateau *propriete, int indicePropriete, struct joueur joueurs[], int nbJoueurs);
void appliquer_effet_case(struct CasePlateau *caseCourante, struct joueur *joueurActuel, struct joueur player[], int d1, int d2);

// --- Gestion des Propriétés (Achat implicite dans appliquer_effet_case) ---
void hypothequerPropriete(struct joueur *p, struct CasePlateau plateau[], int nbCases);
void leverHypotheque(struct joueur *p, struct CasePlateau plateau[], int nbCases);
void construire(struct joueur *p, struct CasePlateau plateau[], int nbCases);
int vendreConstructions(struct joueur *p, struct CasePlateau board[]);
void liquiderToutesConstructions(struct joueur *p, struct CasePlateau board[]);

// --- Négociation ---
void menu_negociation(struct joueur *negociant, struct joueur *cible, struct CasePlateau plateau[], int nbCases);
void negocier(struct joueur *p, struct joueur players[], int nbJoueurs, struct CasePlateau plateau[], int nbCases);

// --- Gestion de la Prison ---
int menuPrison(struct joueur *p, struct joueur players[], struct CasePlateau plateau[], int nbCases);

// --- Gestion de Faillite / Abandon ---
void transfertBiens(struct joueur *p, struct joueur *dest, struct CasePlateau board[], int joueurIndex, int destIndex);
void gererFaillite(struct joueur *p, struct joueur players[], int num_players, struct CasePlateau board[], int creancierIndex);
void abandonnerPartie(struct joueur *p);
int terminerPartie();

// --- Menu d'Actions du Joueur ---
int menu_actions(struct joueur *p);

// --- Modes de jeu principaux ---
void partie_classique();
void partie_tours();
void partie_money();

// --- Sauvegarde / Chargement ---
void sauvegarderPartie(struct joueur player[], int nbjoueurs, struct CasePlateau plateau[], int nbCases);
void charger_partie();

// --- Menu Haut Niveau ---
void menujeu();
void menujoueurs();
void chargement_reg();
void verifierFinDePartie(struct joueur players[], int num_players, struct CasePlateau board[], int nbCases);

// --- Affichage du plateau ---
void afficher_plateau_paris(); 
void ligne_sep(int start, int end, int reverse);
void print_cell_center(const char *text);
void print_cell_center_bg(int idx, const char *text);
void print_horizontal_block(int start, int end, int reverse);
void get_joueurs_str(int idx, char *buf, int bufsize);
void get_case_info_str(int idx, char *buf, int bufsize);
void print_single_border();




//--- MAIN : LANCEMENT DU PROGRAMME ---

int main()
{

    time_t tp, nb_secondes;         
    int initialisation;             

    // Récupération du nombre de secondes écoulées depuis 1/1/1970 
    nb_secondes = time(&tp); // Appel à time() avec une adresse, stocké dans nb_secondes 

    // --- Initialisation de la fonction aléatoire ---
    initialisation = nb_secondes ; 
    srand(initialisation); 

    int choix = -1;
    int saisieOk;
    char c;
    while (choix != 0)
    {
        saisieOk = 0;
        while (!saisieOk)
        {
            printf("=========================================\n");
            printf("           Bienvenue au Monopoly         \n");
            printf("=========================================\n");
            printf("  1 - Jouer une nouvelle partie\n");
            printf("  2 - Charger une partie\n");
            printf("  3 - Consulter les règles\n");
            printf("  4 - Records : meilleurs résultats\n");
            printf("  0 - Quitter\n");
            printf("=========================================\n");
            printf("Entrez votre choix : ");
            if (scanf("%d", &choix) == 1)
            {
                if (choix >= 0 && choix <= 4)
                {
                    saisieOk = 1; // Entier lu et dans la plage valide
                }
                else
                {
                    printf("\nOption invalide. Le choix doit être entre 0 et 4.\n");
                }
            }
            else
            {
                printf("\nOption invalide. Veuillez entrer un nombre.\n");
                viderBuffer();
            }
        }

        viderBuffer();

        switch (choix)
        {
        case 1:
            menujeu();
            break;
        case 2:
            charger_partie();
            break;
        case 3:
            chargement_reg();
            break;
        case 4:
            afficherMeilleursScores();
            break;
        case 0:
            quitter();
            break;
        }
    }
}

// --- MENU DU JEU ---

void menujeu()
{
    int choix = -1; 
    int saisieOk;   
    int c;          

    while (choix != 0) // Boucle principale du menu de jeu
    {
        saisieOk = 0; // On réinitialise avant la boucle de saisie interne

        while (!saisieOk) 
        {
            // Affichage du menu
            printf("=========================================\n");
            printf("               Mode de jeu               \n");
            printf("=========================================\n");
            printf("\nChoisissez un mode de jeu :\n");
            printf("-----------------------------------------\n");
            printf("  1 - Partie classique\n");
            printf("  2 - Partie à nombre de tours limité\n");
            printf("  3 - Partie à objectif monétaire\n");
            printf("-----------------------------------------\n");
            printf("  0 - Retour au menu principal\n");
            printf("-----------------------------------------\n");
            printf("\nEntrez votre choix : "); 

            // Tentative de lecture
            if (scanf("%d", &choix) == 1)
            {
                if (choix >= 0 && choix <= 3)
                {
                    saisieOk = 1; 
                    mode = choix;
                }
                else
                {
                    printf("\nOption invalide. Le choix doit être entre 0 et 3.\n");
                }
            }
            else
            {
                // scanf a échoué (entrée non entière)
                printf("\nOption invalide. Veuillez entrer un nombre.\n");
            }

            viderBuffer();

        } 

        switch (choix)
        {
        case 1:
            printf("\n➡️  Vous avez choisi le mode **Partie classique**.\n");
            menujoueurs();
            jeufini = 0;
            debut = 1;
            partie_classique();
            break;
        case 2:
            printf("\n➡️  Vous avez choisi le mode **Partie à nombre de tours limité**.\n");
            menujoueurs();
            jeufini = 0;
            debut = 1;
            partie_tours();
            break;
        case 3:
            printf("\n➡️  Vous avez choisi le mode **Partie à objectif monétaire**.\n");
            menujoueurs();
            jeufini = 0;
            debut = 1;
            partie_money(); 
            break;
        case 0:
            printf("\nRetour au menu principal...\n");
            break;
        }
    } 
}

//--- MENU DES JOUEURS ---


void menujoueurs()
{

    nbjoueurs = 0;    // La variable globale nbjoeurs est réinitialisée 
    int saisieOk_num; 
    int saisieOk_nom; 
    int c;           
    int de1, de2;

    // --- Boucle pour obtenir le nombre de joueurs ---
    printf("--- Configuration des Joueurs ---\n");
    saisieOk_num = 0;
    while (!saisieOk_num)
    {

        printf("Entrez le nombre de joueurs (entre 2 et 6) : ");

        if (scanf("%d", &nbjoueurs) == 1)
        {
            if (nbjoueurs >= 2 && nbjoueurs <= 6)
            {
                saisieOk_num = 1; 
            }
            else
            {
                printf("Valeur non valide. Le nombre doit être entre 2 et 6.\n");
           
            }
        }
        else
        {
            printf("Valeur non valide. Veuillez entrer un nombre.\n");
        }

        viderBuffer();

    } 

    printf("Nombre de joueurs enregistré : %d\n", nbjoueurs);

    // --- Boucle pour saisir les noms des joueurs ---
    for (int i = 0; i < nbjoueurs; i++)
    {
        int saisieOk_nom = 0; // On réinitialise le flag de saisie pour chaque joueur
        while (!saisieOk_nom)
        {
            char buffer[16];
            printf("Entrez le nom du joueur %d (max 15 caractères, sans espaces) : ", i + 1);

            // On lit dans un buffer intermédiaire pour valider avant de copier
            if (scanf("%15s", buffer) == 1)
            {
                viderBuffer(); 

                // Vérifier qu’aucun joueur précédent n’a le même nom
                int dupliqué = 0;
                for (int k = 0; k < i; k++)
                {
                    if (strcmp(player[k].Nom, buffer) == 0)
                    {
                        dupliqué = 1;
                    }
                }

                if (dupliqué)
                {
                    printf("❌ Ce nom est déjà pris, veuillez en choisir un autre.\n");
                }
                else
                {
                    // Aucun doublon, on peut copier dans player[i].Nom
                    strcpy(player[i].Nom, buffer);
                    saisieOk_nom = 1;
                }
            }
            else
            {
                viderBuffer();
                printf("Erreur de lecture, réessayez.\n");
            }
        } 

        // Initialisation des autres champs du joueur
        player[i].argent = ARGENT_INITIAL;
        player[i].position = 0;
        player[i].nbProprietes = 0;
        player[i].enPrison = 0;
        player[i].tourEnPrison = 0;
        player[i].cartesSortiePrison = 0;
        player[i].enJeu = 1;
        player[i].libereChance = 0;
        player[i].libereCommunaute = 0;
        for (int j = 0; j < MAX_PROPRIETES; j++)
        {
            player[i].proprietes[j] = -1;
        }

        printf("Joueur %d enregistré : %s\n", i + 1, player[i].Nom);
    }

    // 1) Lancement des dés pour déterminer le score initial
    int score[nbjoueurs];
    printf("C'est le moment de déterminer l'ordre des joueurs\n");
    for (int i = 0; i < nbjoueurs; i++)
    {
        printf("%s, appuyez sur Entrée pour lancer les dés…", player[i].Nom);
        viderBuffer();
        de1 = lancer_de();
        de2 = lancer_de();
        score[i] = de1 + de2;
        printf(" 🎲 %d + %d = %d\n", de1, de2, score[i]);
    }

    // 2) On prépare l'indice d'ordre
    int ordre[nbjoueurs];
    for (int i = 0; i < nbjoueurs; i++)
    {
        ordre[i] = i;
    }

    // 3) Tri à bulle décroissant selon score (tri alphabétique en cas d'égalité)
    for (int pass = 0; pass < nbjoueurs - 1; pass++)
    {
        for (int j = 0; j < nbjoueurs - 1 - pass; j++)
        {
            int a = ordre[j];
            int b = ordre[j + 1];
            int swap = 0;
            if (score[a] < score[b])
            {
                swap = 1;
            }
            else if (score[a] == score[b])
            {
                // Si égalité de score, on compare les noms
                if (strcmp(player[a].Nom, player[b].Nom) > 0)
                {
                    swap = 1;
                }
            }
            if (swap)
            {
                ordre[j] = b;
                ordre[j + 1] = a;
            }
        }
    }

    // 4) On réordonne player[] selon l'indice ordre[]
    struct joueur tmp[nbjoueurs];
    for (int i = 0; i < nbjoueurs; i++)
    {
        tmp[i] = player[ordre[i]];
    }
    for (int i = 0; i < nbjoueurs; i++)
    {
        player[i] = tmp[i];
    }
}

//--- Appel en lecture du fichier "regles.txt" pour chargement et affichage des règles du jeu ---

void chargement_reg()
{
    char NomFichier[20] = "regles.txt";
    FILE *f1 = fopen(NomFichier, "r");
    char ligne[250];
    while (fgets(ligne, sizeof(ligne), f1))
    {
        printf("%s", ligne);
    }
    fclose(f1);
}


//--- Fonction de lancement des dés ---
int lancer_de()
{
    return (rand() % 6) + 1;
}

//--- Quitter le programme principal ---
void quitter()
{
    printf("Merci d'avoir utilisé le menu. À bientôt !\n");
}

//--- Affichage des informations d'un joueur
void afficherJoueur(const struct joueur *p)
{
    char estEnPrison[10];
    // "Non" ou "Oui" pour le statut en prison
    if ((*p).enPrison)
    {
        strcpy(estEnPrison, "Oui");
    }
    else
    {
        strcpy(estEnPrison, "Non");
    }

    printf("| %-15s | %-6d | %-8d | %-12d | %-9s |\n", (*p).Nom, (*p).argent, (*p).position, (*p).nbProprietes, estEnPrison);
}

// --- Met en majuscules les lettres d'une chaîne de caractères ---

void conv_maj(char chaine[])
{
    int i, taille;

    taille = strlen(chaine);
    for (i = 0; i < taille; i++)
    {
        chaine[i] = toupper(chaine[i]);
    }
}


// --- Dessine une ligne de séparation "+---------+...+" ---
void ligne_sep(int start, int end, int reverse)
{
    int i, j;
    if (reverse == 0)
    {
        for (i = start; i <= end; i++)
        {
            printf("+");
            for (j = 0; j < LARGEUR_CELL - 2; j++)
            {
                printf("-");
            }
        }
    }
    else
    {
        for (i = start; i >= end; i--)
        {
            printf("+");
            for (j = 0; j < LARGEUR_CELL - 2; j++)
            {
                printf("-");
            }
        }
    }
    printf("+");
}

// --- Centre le texte dans une cellule de LARGEUR_CELL et affiche "|  texte   " ---
void print_cell_center(const char *text)
{
    int len = strlen(text);
    int total = LARGEUR_CELL - 2;
    int pad = (total - len) / 2;
    int right = total - pad - len;
    int k;

    for (k = 0; k < pad; k++)
    {
        printf(" ");
    }
    printf("%s", text);
    for (k = 0; k < right; k++)
    {
        printf(" ");
    }
}

// --- Affiche un texte centré avec couleur de fond selon la case ---

void print_cell_center_bg(int idx, const char *text)
{
    int len = strlen(text);
    int total = LARGEUR_CELL - 2;  // espace disponible à l’intérieur de la cellule
    int pad = (total - len) / 2;   // espaces avant le texte
    int right = total - pad - len; // espaces après le texte
    printf(EFFACE_ATTR);

    // 2) on applique la couleur de fond + texte noir
    if (plateau[idx].couleur != 'M' && plateau[idx].couleur != 'B' && plateau[idx].couleur != 'R' && plateau[idx].couleur != 'V' && plateau[idx].couleur != 'J' && plateau[idx].couleur != 'C' && plateau[idx].couleur != 'S' && plateau[idx].couleur != 'O')
    {
        printf(TEXTE_BLANC);
    }
    else if (plateau[idx].couleur == 'M')
    {
        printf(FOND_NOIR);
        printf(TEXTE_BLANC);
    }
    else if (plateau[idx].couleur == 'B')
    {
        printf(FOND_BLUE);
        printf(TEXTE_BLANC);
    }
    else if (plateau[idx].couleur == 'R')
    {
        printf(FOND_ROUGE);
        printf(TEXTE_BLANC);
    }
    else
    {
        switch (plateau[idx].couleur)
        {
        case 'V':
            printf(FOND_VERT);
            break;
        case 'J':
            printf(FOND_JAUNE);
            break;
        case 'C':
            printf(FOND_CYAN);
            break;
        case 'S':
            printf(FOND_MAGENTA);
            break;
        case 'O':
            printf(FOND_BLANC);
            break;
        }
        printf(TEXTE_NOIR);
    }

    // 3) espaces de gauche
    for (int i = 0; i < pad; i++)
    {
        printf(" ");
    }

    // 4) texte
    printf("%s", text);

    // 5) espaces de droite
    for (int i = 0; i < right; i++)
    {
        printf(" ");
    }

    // 6) réinitialisation des attributs
    printf(EFFACE_ATTR);
}

// --- Remplit buf par l’état de la case (proprio, maisons, hypothèque) ---
void get_case_info_str(int idx, char *buf, int bufsize)
{

     // Si la case est libre (propriétaire −1), ne rien afficher
    if (plateau[idx].proprietaire == -1 || bufsize == 0)
    {
         if (bufsize > 0)
         {
            buf[0] = '\0';
         }        
                  
    }
    else
    {
        int p = plateau[idx].proprietaire;
        /* buf : tampon de sortie
        bufsize : taille disponible
        p      : indice du joueur
        idx    : indice de la case */                               

            char init = player[p].Nom[0];        

            if (plateau[idx].estHypothequee)
            {
                if (bufsize >= 3)             
                {
                    buf[0] = init;
                    buf[1] = '*';
                    buf[2] = '\0';
                }
            }

            // Hôtel ex: P H 
            else if (plateau[idx].Hotel)
            {
                if (bufsize >= 3)
                {
                    buf[0] = init;
                    buf[1] = 'H';
                    buf[2] = '\0';
                }
            }

            // 1 à 4 maisons : "<initiale><n>M"  => ex: P3M 
            else if (plateau[idx].nbMaisons > 0)
            {
                if (bufsize >= 4)              
                {
                    buf[0] = init;
                    buf[1] = (char)('0' + plateau[idx].nbMaisons);  
                    buf[2] = 'M';
                    buf[3] = '\0';
                }
            }

            // Aucune construction : "Prop. : <initiale>"  
            else
            {
                if (bufsize >= 9)
                {
                    strcpy(buf, "Prop. : ");
                    buf[8] = init;    
                    buf[9] = '\0';
                }
            }

    }
    
}

// Remplit buf par les joueurs présents sur la case (initiales avec un espace)
void get_joueurs_str(int idx, char *buf, int bufsize)
{
    int pos = 0, j;
    buf[0] = '\0';
    for (j = 0; j < nbjoueurs; j++)
    {
        if (pos + 2 < bufsize)
        {
            if (player[j].enJeu && player[j].position == idx)
            {
                buf[pos++] = player[j].Nom[0];
                buf[pos++] = ' ';
                buf[pos] = '\0';
            }
        }
    }
}

// Dessine une seule bordure "+---------+"
void print_single_border()
{
    int j;
    printf("+");
    for (j = 0; j < LARGEUR_CELL - 2; j++)
    {
        printf("-");
    }
    printf("+");
}

// --- Affiche un bloc horizontal du plateau avec les infos des cases et des joueurs ---
void print_horizontal_block(int start, int end, int reverse)
{
    char buf[LARGEUR_CELL];
    int i;

    // 1) bordure haute
    ligne_sep(start, end, reverse);

    // 2) noms abrégés
    printf("\n");
    if (!reverse)
    {
        printf("|");
        for (i = start; i <= end; i++)
        {

            print_cell_center_bg(i, plateau[i].nom_abrege);
            printf("|");
        }
    }
    else
    {
        printf("|");
        for (i = start; i >= end; i--)
        {
            print_cell_center_bg(i, plateau[i].nom_abrege);
            printf("|");
        }
    }
    printf("\n");

    // 3) proprio / maisons
    if (!reverse)
    {
        printf("|");
        for (i = start; i <= end; i++)
        {
            get_case_info_str(i, buf, sizeof buf);
            print_cell_center(buf);
            printf("|");
        }
    }
    else
    {
        printf("|");
        for (i = start; i >= end; i--)
        {
            get_case_info_str(i, buf, sizeof buf);
            print_cell_center(buf);
            printf("|");
        }
    }
    printf("\n");

    // 4) joueurs
    if (!reverse)
    {
        printf("|");
        for (i = start; i <= end; i++)
        {
            get_joueurs_str(i, buf, sizeof buf);
            print_cell_center(buf);
            printf("|");
        }
    }
    else
    {
        printf("|");
        for (i = start; i >= end; i--)
        {
            get_joueurs_str(i, buf, sizeof buf);
            print_cell_center(buf);
            printf("|");
        }
    }
    printf("\n");

    // 5) bordure basse
    ligne_sep(start, end, reverse);
    printf("\n");
}

// --- Affiche le plateau de jeu complet (version Paris) ---
void afficher_plateau_paris()
{
    int ligne, g, d;
    char bufL[LARGEUR_CELL], bufR[LARGEUR_CELL];

    // 1) ligne du haut (cases 20→30)
    print_horizontal_block(20, 30, 0);

    // 2) corps latéral : 9 rangées (cases gauche 19→11 et droite 31→39)
    for (ligne = 1; ligne <= NB_CASES_MILIEU; ligne++)
    {
        g = 20 - ligne; // case gauche
        d = 30 + ligne; // case droite

        // a) bordure gauche + espace central + bordure droite
        if (ligne != 1)
        {
            print_single_border();
            for (int k = 0; k < SIDE_SPACE; k++)
            {
                printf(" ");
            }
            print_single_border();
            printf("\n");
        }

        // b) noms abrégés avec fond
        printf("|");
        print_cell_center_bg(g, plateau[g].nom_abrege);
        printf("|");
        for (int k = 0; k < SIDE_SPACE; k++)
        {
            printf(" ");
        }

        printf("|");
        print_cell_center_bg(d, plateau[d].nom_abrege);
        printf("|\n");

        // c) proprio / maisons
        get_case_info_str(g, bufL, sizeof bufL);
        get_case_info_str(d, bufR, sizeof bufR);
        printf("|");
        print_cell_center(bufL);
        printf("|");
        for (int k = 0; k < SIDE_SPACE; k++)
        {
            printf(" ");
        }

        printf("|");
        print_cell_center(bufR);
        printf("|\n");

        // d) joueurs
        get_joueurs_str(g, bufL, sizeof bufL);
        get_joueurs_str(d, bufR, sizeof bufR);
        printf("|");
        print_cell_center(bufL);
        printf("|");
        for (int k = 0; k < SIDE_SPACE; k++)
        {
            printf(" ");
        }
        printf("|");
        print_cell_center(bufR);
        printf("|\n");
    }

    // 3) ligne du bas (cases 10→0 inversé)
    print_horizontal_block(10, 0, 1);
}

//--- Menu d'actions des joueurs ---

int menu_actions(struct joueur *p)
{
    int choix = -1; 
    int saisieOk;
    int fin = 0; 
    char response[10];
    int c; 

    while (!fin && !jeufini) // Boucle d'affichage du menu d'actions
    {
        printf("\n===========================================\n");
        printf("    Menu d'actions pour %s\n", (*p).Nom);
        printf("===========================================\n");
        printf("1. Négocier avec un autre joueur\n");
        printf("2. Construire (maison/hôtel)\n");
        printf("3. Vendre des constructions\n");
        printf("4. Hypothéquer une propriété\n");
        printf("5. Lever une hypothèque\n");
        printf("6. Consulter son bilan\n");
        printf("7. Lancer les dés\n");
        printf("8. 🔚 Se retirer en tant que joueur (abandon)\n");
        printf("9. 💾 Quitter le jeu (tous les joueurs) et sauvegarder\n");
        printf("10. Terminer la partie\n");
        printf("=========================================\n");

        // --- Boucle de saisie pour le choix ---
        saisieOk = 0;
        choix = -1; // On réinitialise le choix pour la validation de plage
        while (!saisieOk)
        {
            printf("Votre choix (1-10) : ");

            if (scanf("%d", &choix) == 1)
            {
                viderBuffer();

                // On vérifie la plage
                if (choix > 0 && choix <= 10) 
                {
                    saisieOk = 1; // Valide
                }
                else
                {
                    printf("Option invalide. Le choix doit être entre 1 et 10.\n");
                }
            }
            else
            {
                printf("Option invalide. Veuillez entrer un nombre.\n"); // Saisie différente d'un nb
                viderBuffer();
            }
        } 

        // --- Traitement du choix ---
        if (choix == 7) //Si le joueur veut lancer les dés, on sort du menu
        {
            fin = 1; 
            return (!jeufini && !session_finie); 
        }
        else
        {
            switch (choix)
            {
            case 1:
                negocier(p, player, nbjoueurs, plateau, NB_CASES);
                break;
            case 2:
                construire(p, plateau, NB_CASES);
                break;
            case 3:
                vendreConstructions(p, plateau);
                break;
            case 4:
                hypothequerPropriete(p, plateau, NB_CASES);
                break;
            case 5:
                leverHypotheque(p, plateau, NB_CASES);
                break;
            case 6:
                afficherBilan(p, plateau, NB_CASES);
                break;
            case 7:
                break;
            case 8: 
                saisieOk = 0;
                while (!saisieOk)
                {
                    printf("Vous êtes sûr de vouloir quitter la partie? (O/N) : ");
                    if (scanf("%9s", response) == 1)
                    {
                        viderBuffer(); 
                        conv_maj(response);
                        if (response[0] == 'O' || response[0] == 'N')
                        {
                            saisieOk = 1; 
                        }
                        else
                        {
                            printf("Réponse invalide. Entrez 'O' ou 'N'.\n");
                        }
                    }
                    else
                    {
                        printf("Erreur de saisie. Entrez 'O' ou 'N'.\n");
                        viderBuffer(); 
                    }
                } 

                if (response[0] == 'O') // Le joueur décide d'abandonner la partie
                {
                    abandonnerPartie(p);
                    fin = 1;
                    return 0;
                }
                break;
            case 9: // Quitter la session de jeu et sauvegarder 
                saisieOk = 0;
                while (!saisieOk)
                {
                    printf("ATTENTION : Cela va **terminer la session de jeu** pour tous les joueurs.\n");
                    printf("Souhaitez-vous sauvegarder la partie avant de quitter ? (O/N) : ");
                    fflush(stdout);
                    if (scanf("%9s", response) == 1)
                    {
                        viderBuffer(); 
                        conv_maj(response);
                        if (response[0] == 'O' || response[0] == 'N')
                        {
                            saisieOk = 1; 
                        }
                        else
                        {
                            printf("Réponse invalide. Entrez 'O' ou 'N'.\n");
                        }
                    }
                    else
                    {
                        printf("Erreur de saisie. Entrez 'O' ou 'N'.\n");
                        viderBuffer(); 
                    }
                } 

                if (response[0] == 'O')
                {
                    sauvegarderPartie(player, nbjoueurs, plateau, NB_CASES);
                    printf(" Partie sauvegardée avec succès !\n");
                
                    printf("👋 Fin de la session de jeu. À bientôt !\n");
                    session_finie = 1; // Marquer le jeu comme terminé
                    fin = 1;     // Sortir du menu d'actions
                    return 0;
                }
                else if (response[0] == 'N')
                 {
                    printf("👋 Fin de la session de jeu. À bientôt !\n");
                    session_finie = 1; // La session se termine sans sauvegarde
                    fin = 1;
                    return 0;
                 }
                break;
            case 10: // Terminer la partie
                if (terminerPartie() == 1)
                {
                    fin = 1;
                    return 0;
                }
                break;
            case 0:
                printf("Option 0 non valide dans ce menu.\n");
                break;
            } 
        } 
    } 
    return (!jeufini && !session_finie);

}

//--- PARTIE CLASSIQUE ---
void partie_classique()
{
    int de1, de2;
    int i, k; 
    int nbActifs = 0;

    // --- Initialisation de la partie ---
    if (debut)
    {
        printf("Initialisation de la partie classique...\n");
        initialisation_cartes();
        initialisation_plateau();
        debut = 0;
        tour_actuel = 0;
    }
    else // Reprise d'une partie antérieure (chargement)
    {
        printf("Reprise de la partie classique au tour %d...\n", tour_actuel + 1);
    }
    jeufini = 0;
    session_finie = 0; // Réinitialisation de la variable globale de session 

    // --- Boucle de Jeu Principale ---
    while (!jeufini && !session_finie) 
    {
        tour_actuel++;
        
        printf("\n====================== TOUR %d ======================\n", tour_actuel);

        // --- Affichage du Statut des Joueurs ---
        printf("+-----------------+--------+----------+--------------+-----------+\n");
        printf("| Nom             | Argent | Position | Propriétés   | En prison |\n");
        printf("+-----------------+--------+----------+--------------+-----------+\n");
        for (k = 0; k < nbjoueurs; k++)
        {
            if (player[k].enJeu)
            {
                afficherJoueur(&player[k]);
            }
        }
        printf("+-----------------+--------+----------+--------------+-----------+\n");

        afficher_plateau_paris(); // Affichage du plateau

        // --- Boucle des Tours des Joueurs ---
        for (i = 0; i < nbjoueurs && !jeufini && !session_finie; i++)
        {
            if (player[i].enJeu)
            {
                printf("\n----- Tour de %s (Argent: %d M) -----\n", player[i].Nom, player[i].argent);

                int compteurDoubles = 0;
                int TourNormal = 0;                               
                int joueurEstEnPrisonCeTour = player[i].enPrison; // État au début du tour

                // --- Gestion de la Prison ---
                if (joueurEstEnPrisonCeTour)
                {
                    printf("🚔 %s est en prison (Tour %d/3).\n", player[i].Nom, player[i].tourEnPrison + 1);
                    if (player[i].tourEnPrison > 2)
                    {
                        printf(" C'est votre dernière chance de sortir avant l'amende...\n");
                    }

                    TourNormal = menuPrison(&player[i], player, plateau, NB_CASES);
                    if (player[i].argent < 0 && player[i].enJeu)
                    {
                        gererFaillite(&player[i], player, nbjoueurs, plateau, -1);
                    }

                    if (player[i].enJeu)
                    {
                        if (player[i].enPrison && player[i].tourEnPrison >= 2)
                        {
                            printf(" Libération forcée après 3 tours ! Paiement de 50 M.\n");
                            player[i].argent -= 50;
                            if (player[i].argent < 0 && player[i].enJeu)
                            {
                                printf("%s est en faillite en sortant de prison!\n", player[i].Nom);
                                gererFaillite(&player[i], player, nbjoueurs, plateau, -1);
                            }
                            if (player[i].enJeu)
                            {
                                player[i].enPrison = 0;
                                player[i].tourEnPrison = 0;
                                TourNormal = 1;
                                printf("Vous êtes libéré et devez maintenant jouer votre tour.\n");
                            }
                        }
                        else if (!player[i].enPrison && TourNormal == 1)
                        {
                            printf("Jouez votre tour normalement\n");
                        }
                        else
                        {
                            player[i].tourEnPrison++;
                            TourNormal = 0;
                        }
                    }
                }
                else
                {
                    TourNormal = 1;
                }

                // --- Tour Normal ---
                if (TourNormal && player[i].enJeu)
                {
                    int continuerLancer = 1;       // Flag pour indiquer si le joueur peut rejouer en cas de double
                    int allerEnPrison3Doubles = 0; // Flag pour indiquer si le joeuur est envoyé en prison pour 3 doubles

                    while (continuerLancer && player[i].enJeu && !jeufini && !session_finie)
                    {
                        int flag = menu_actions(&player[i]);
                        
                        if(!jeufini && !session_finie)
                        {
                            if (flag == 1)
                            {
                                de1 = lancer_de();
                                de2 = lancer_de();
                                printf("🎲 Lancement des dés : %d et %d\n", de1, de2);

                                if (de1 == de2)
                                {
                                    compteurDoubles++;
                                    if (compteurDoubles >= 3)
                                    {
                                        printf("🚔 3 doubles consécutifs ! %s va en prison.\n", player[i].Nom);
                                        player[i].position = 10;
                                        player[i].enPrison = 1;
                                        player[i].tourEnPrison = 0;
                                        allerEnPrison3Doubles = 1;
                                        continuerLancer = 0;
                                    }
                                    else
                                    {
                                        printf("🔥 Double ! (%d/%d)\n", compteurDoubles, 3);
                                    }
                                }
                                else
                                {
                                    compteurDoubles = 0;
                                    continuerLancer = 0;
                                }

                                if (!allerEnPrison3Doubles)
                                {
                                    int anciennePosition = player[i].position;
                                    player[i].position += de1 + de2; // Mise à jour de la position du joueur
                                    if (player[i].position >= NB_CASES) 
                                    {
                                        player[i].position -= NB_CASES;
                                        if (player[i].position != 0 ||
                                            anciennePosition > player[i].position)
                                        {
                                            printf("Vous passez par la case départ. RECEVEZ M200!\n");
                                            player[i].argent += ARGENT_DEPART;
                                        }
                                    }
                                    // Effets déclenchés sur la case d'arrivée
                                    printf("➡️ %s avance à la case %d : %s\n", player[i].Nom, player[i].position, plateau[player[i].position].nom);
                                    appliquer_effet_case(&plateau[player[i].position], &player[i], player, de1, de2);
                                    if (player[i].argent < 0 && player[i].enJeu)
                                    {
                                        gererFaillite(&player[i], player, nbjoueurs, plateau, -1);

                                    }
                                    if (!player[i].enJeu || player[i].enPrison)
                                    {
                                        continuerLancer = 0;
                                    }
                                    else if (continuerLancer)
                                    {
                                        printf("Vous rejouez !\n");

                                    }
                                        
                                }
                            }
                            else
                            {
                                continuerLancer = 0;
                            }

                        }
                        
                    }
                }

                // --- Vérification de Fin de Partie (après chaque tour d'un joueur) ---
                if(!jeufini && !session_finie)
                {
                    verifierFinDePartie(player, nbjoueurs, plateau, NB_CASES);
                }
            }
        }
    } // Fin de la boucle principale du jeu 

    if (jeufini && !session_finie) 
    {
        printf("\n--- Partie Classique Terminée ---\n");
        // Affichage des scores des joueurs et mise à jour de la table de meilleurs scores
        afficherTableauScore(player, nbjoueurs, plateau, NB_CASES);
        mettreAJourMeilleursScores();
    }
    else if (session_finie) // Session de jeu interrompue
    {
        printf("\n--- Session de jeu terminée par l'utilisateur ---\n");
    }
   
    
}


//--- PARTIE À NB DE TOURS LIMITÉ ---
void partie_tours()
{
    int currentTurn, i;
    int saisieOk = 0;
    int de1, de2;
    int tour_depart; 
    
    // --- Initialisation de la partie ---
    if (debut)
    {
        while (!saisieOk)
        {
            printf("Entrez le nombre de tours de votre partie : ");
            saisieOk = scanf("%d", &nb_tours_total);
            viderBuffer();
            if (!saisieOk || nb_tours_total <= 0)
            {
                printf("Veuillez entrer un nombre valide\n");
                saisieOk = 0; 
            }
        }
        
        printf("Initialisation de la partie (mode tours)...\n");
        initialisation_cartes();
        initialisation_plateau();
        tour_depart = 1; 
        debut = 0;
    }
    else // Reprise d'une partie antérieure (chargement)
    {
        printf("Reprise de la partie (mode %d tours) au tour %d...\n", nb_tours_total, tour_actuel);
        tour_depart = tour_actuel;
    }
    jeufini = 0; // Réinitialisation des variables globales de fin de jeu et la session
    session_finie = 0;


    // Boucle principale sur un nombre fixe de tours
    for (int current_turn = tour_depart; current_turn <= nb_tours_total; current_turn++)
    {
        tour_actuel = current_turn; // Mise à jour de la variable globale (utilisée pour la sauvegarde de la partie)
        if (!jeufini && !session_finie)
        {
            printf("\n========== TOUR %d / %d ==========\n", tour_actuel, nb_tours_total);

            // Affichage du statut global des joueurs
            printf("+-----------------+--------+----------+--------------+-----------+\n");
            printf("| Nom             | Argent | Position | Propriétés   | En prison |\n");
            printf("+-----------------+--------+----------+--------------+-----------+\n");
            for (i = 0; i < nbjoueurs; i++)
            {
                if (player[i].enJeu)
                {
                    afficherJoueur(&player[i]);
                }
            }
            printf("+-----------------+--------+----------+--------------+-----------+\n");

            afficher_plateau_paris(); // Affichage du plateau

            // --- Boucle des Tours des Joueurs ---
            for (i = 0; i < nbjoueurs && !jeufini; i++)
            {
                if (player[i].enJeu)
                {
                    printf("\n----- Tour de %s (Argent: %d M) -----\n", player[i].Nom, player[i].argent);

                    int compteurDoubles = 0;
                    int TourNormal = 0;                               
                    int joueurEstEnPrisonCeTour = player[i].enPrison; // État au début du tour

                    // --- Gestion de la Prison ---
                    if (joueurEstEnPrisonCeTour)
                    {
                        printf("🚔 %s est en prison (Tour %d/3).\n",player[i].Nom, player[i].tourEnPrison + 1);

                        if (player[i].tourEnPrison > 2)
                        {
                            printf(" C'est votre dernière chance de sortir avant l'amende...\n");
                        }

                        TourNormal = menuPrison(&player[i], player, plateau, NB_CASES);

                        if (player[i].argent < 0 && player[i].enJeu)
                        {
                            gererFaillite(&player[i], player, nbjoueurs, plateau, -1);
                        }

                        if (player[i].enJeu)
                        {
                            if (player[i].enPrison && player[i].tourEnPrison >= 2)
                            {
                                printf(" Libération forcée après 3 tours ! Paiement de 50 M.\n");
                                player[i].argent -= 50;
                                if (player[i].argent < 0 && player[i].enJeu)
                                {
                                    printf("%s est en faillite en sortant de prison!\n", player[i].Nom);
                                    gererFaillite(&player[i], player, nbjoueurs, plateau, -1);
                                }
                                if (player[i].enJeu)
                                {
                                    player[i].enPrison = 0;
                                    player[i].tourEnPrison = 0;
                                    TourNormal = 1;
                                    printf("Vous êtes libéré et devez maintenant jouer votre tour.\n");
                                }
                            }
                            else if (!player[i].enPrison && TourNormal == 1)
                            {
                                printf("Jouez votre tour normalement\n");
                            }
                            else
                            {
                                player[i].tourEnPrison++;
                                TourNormal = 0;
                            }
                        }
                    }
                    else
                    {
                        TourNormal = 1;
                    }

                    // --- Tour Normal (Lancement des dés) ---
                    if (TourNormal && player[i].enJeu)
                    {
                        int continuerLancer = 1;       // 1 = continue le lancement des dés (en cas de double)
                        int allerEnPrison3Doubles = 0; // 1 = envoyé en prison pour 3 doubles

                        while (continuerLancer && player[i].enJeu && !session_finie && !jeufini)
                        {
                            int flag = menu_actions(&player[i]);
                           
                            if(!jeufini && !session_finie)
                            {
                                if (flag == 1)
                                {
                                    de1 = lancer_de();
                                    de2 = lancer_de();
                                    printf("🎲 Lancement des dés : %d et %d\n", de1, de2);

                                    if (de1 == de2)
                                    {
                                        compteurDoubles++;
                                        if (compteurDoubles >= 3)
                                        {
                                            printf("🚔 3 doubles consécutifs ! %s va en prison.\n", player[i].Nom);
                                            player[i].position = 10;
                                            player[i].enPrison = 1;
                                            player[i].tourEnPrison = 0;
                                            allerEnPrison3Doubles = 1;
                                            continuerLancer = 0;
                                        }
                                        else
                                        {
                                            printf("🔥 Double ! (%d/%d)\n", compteurDoubles, 3);
                                        }
                                    }
                                    else
                                    {
                                        compteurDoubles = 0;
                                        continuerLancer = 0;
                                    }

                                    if (!allerEnPrison3Doubles)
                                    {
                                        int anciennePosition = player[i].position;
                                        player[i].position += de1 + de2;
                                        if (player[i].position >= NB_CASES)
                                        {
                                            player[i].position -= NB_CASES;

                                            if (player[i].position != 0 || anciennePosition > player[i].position)
                                            {
                                                printf("Vous passez par la case départ. RECEVEZ M200!\n");
                                                player[i].argent += ARGENT_DEPART;
                                            }
                                        }

                                        printf("➡️ %s avance à la case %d : %s\n", player[i].Nom, player[i].position, plateau[player[i].position].nom);
                                        appliquer_effet_case(&plateau[player[i].position], &player[i], player, de1, de2);
                                        if (player[i].argent < 0 && player[i].enJeu)
                                        {
                                            gererFaillite(&player[i],player, nbjoueurs, plateau, -1);
                                        }
                                            
                                        if (!player[i].enJeu || player[i].enPrison)
                                        {
                                            continuerLancer = 0;
                                        }

                                        else if (continuerLancer)
                                        {
                                            printf("Vous rejouez !\n");
                                        }
                                            
                                    }
                                }
                                else
                                {
                                    continuerLancer = 0;
                                }

                            }
                            
                        }
                    }
                    
                    // --- Vérification de Fin de Partie (après chaque tour de joueur) ---

                    if (!session_finie && !jeufini) 
                    {
                        verifierFinDePartie(player, nbjoueurs, plateau, NB_CASES);
                    }
                }
            }
        }
    }
  
     // --- Affichage final et mise à jour des scores ---
    
    // Cas 1 : fin du jeu 
     if (jeufini && !session_finie)
    {
        printf("\n--- Partie terminée prématurément par élimination ---\n");
        afficherTableauScore(player, nbjoueurs, plateau, NB_CASES);
        mettreAJourMeilleursScores();
    }
    // Cas 2: Le nombre de tours est atteint san fin prématurée par faillite ET la session n'est pas finie

    else if (!jeufini && !session_finie && tour_actuel >= nb_tours_total)
    {
        printf("\n--- Partie terminée : Nombre de tours (%d) atteint ! ---\n", nb_tours_total);
        afficherTableauScore(player, nbjoueurs, plateau, NB_CASES); // Afficher le détail
        mettreAJourMeilleursScores(); // Mettre à jour les records
    }
    // Cas 3: La session a été terminée par l'utilisateur
    else if (session_finie)
    {
        printf("\n--- Session de jeu terminée par l'utilisateur ---\n");
    }
 
}
  

//--- PARTIE À OBJECTIF MONÉTAIRE ---
void partie_money()
{
    int saisieOk = 0;
    int de1, de2;
    int i;

    // --- Initialisation de la partie ---
    if (debut)
    {
        while (!saisieOk)
        {
            printf("Entrez l'objectif monétaire (montant > 1500 M) : ");
            saisieOk = scanf("%d", &objectif_monetaire);
            viderBuffer();
            if (!saisieOk)
            {
                printf("Veuillez entrer un nombre entier valide\n");
            }
            else if (objectif_monetaire <= ARGENT_INITIAL)
            {
                printf("Montant insuffisant pour un objectif valable. Veuillez entrer un montant supérieur à 1500 M : ");
                saisieOk = 0; 
            }
        }
        tour_actuel = 0; 
        printf("Initialisation de la partie (mode objectif monétaire)...\n");
        initialisation_cartes();
        initialisation_plateau();
        debut = 0;
    }
    else // Reprise d'une partie antérieure (chargement)
    {
        printf("Reprise de la partie (objectif %d M)...\n", objectif_monetaire);
    }
    jeufini = 0;
    session_finie = 0; 


    // --- Boucle principale du jeu : tant qu'aucun joueur n'atteint l'objectif et que le jeu n'est pas terminé ---
    while (!jeufini && !session_finie)
    {
        tour_actuel++;
        printf("\n========== TOUR %d ==========\n", tour_actuel);

        // Affichage du tableau récapitulatif des joueurs
        printf("+-----------------+--------+----------+--------------+-----------+\n");
        printf("| Nom             | Argent | Position | Propriétés   | En prison |\n");
        printf("+-----------------+--------+----------+--------------+-----------+\n");
        for (i = 0; i < nbjoueurs; i++)
        {
            if (player[i].enJeu)
            {
                afficherJoueur(&player[i]);
            }
        }
        printf("+-----------------+--------+----------+--------------+-----------+\n");

        afficher_plateau_paris(); // Affichage du plateau

        // --- Boucle des Tours des Joueurs ---
        for (i = 0; i < nbjoueurs && !jeufini && !session_finie; i++)
        {
            if (player[i].enJeu)
            {
                printf("\n----- Tour de %s (Argent: %d M) -----\n", player[i].Nom, player[i].argent);

                int compteurDoubles = 0;
                int TourNormal = 0;                               // Flag: 0=non, 1=oui
                int joueurEstEnPrisonCeTour = player[i].enPrison; // État au début du tour

                // --- Gestion de la Prison ---
                if (joueurEstEnPrisonCeTour)
                {
                    printf("🚔 %s est en prison (Tour %d/3).\n", player[i].Nom, player[i].tourEnPrison + 1);
                    if (player[i].tourEnPrison > 2)
                    {
                        printf(" C'est votre dernière chance de sortir avant l'amende...\n");
                    }

                    TourNormal = menuPrison(&player[i], player, plateau, NB_CASES);

                    if (player[i].argent < 0 && player[i].enJeu)
                    {
                        gererFaillite(&player[i], player, nbjoueurs, plateau, -1);
                    }

                    if (player[i].enJeu)
                    {
                        if (player[i].enPrison && player[i].tourEnPrison >= 2)
                        {
                            printf(" Libération forcée après 3 tours ! Paiement de 50 M.\n");
                            player[i].argent -= 50;
                            if (player[i].argent < 0 && player[i].enJeu)
                            {
                                printf("%s est en faillite en sortant de prison!\n", player[i].Nom);
                                gererFaillite(&player[i], player, nbjoueurs, plateau, -1);
                            }
                            if (player[i].enJeu)
                            {
                                player[i].enPrison = 0;
                                player[i].tourEnPrison = 0;
                                TourNormal = 1;
                                printf("Vous êtes libéré et devez maintenant jouer votre tour.\n");
                            }
                        }
                        else if (!player[i].enPrison && TourNormal == 1)
                        {
                            printf("Jouez votre tour normalement\n");
                        }
                        else
                        {
                            player[i].tourEnPrison++;
                            TourNormal = 0;
                        }
                    }
                }
                else
                {
                    TourNormal = 1;
                }

                // --- Tour Normal (Lancer de dés) ---
                if (TourNormal && player[i].enJeu)
                {
                    int continuerLancer = 1;       // 1 = Le joueur peut rejouer en cas de double
                    int allerEnPrison3Doubles = 0; // 1 = Le joueur est envoyé en prison pour 3 doubles

                    while (continuerLancer && player[i].enJeu && !jeufini && !session_finie)
                    {
                        int flag = menu_actions(&player[i]);
                        if (jeufini || session_finie)
                        {
                            continuerLancer = 0; 
                        }
                        else if (flag == 1)
                        {
                            de1 = lancer_de();
                            de2 = lancer_de();
                            printf("🎲 Lancement des dés : %d et %d\n", de1, de2);

                            if (de1 == de2)
                            {
                                compteurDoubles++;
                                if (compteurDoubles >= 3)
                                {
                                    printf("🚔 3 doubles consécutifs ! %s va en prison.\n", player[i].Nom);
                                    player[i].position = 10;
                                    player[i].enPrison = 1;
                                    player[i].tourEnPrison = 0;
                                    allerEnPrison3Doubles = 1;
                                    continuerLancer = 0;
                                }
                                else
                                {
                                    printf("🔥 Double ! (%d/%d)\n", compteurDoubles, 3);
                                }
                            }
                            else
                            {
                                compteurDoubles = 0;
                                continuerLancer = 0;
                            }

                            if (!allerEnPrison3Doubles)
                            {
                                int anciennePosition = player[i].position;
                                player[i].position += de1 + de2;
                                if (player[i].position >= NB_CASES)
                                {
                                    player[i].position -= NB_CASES;
                                    if (player[i].position != 0 || anciennePosition > player[i].position)
                                    {
                                        printf("Vous passez par la case départ. RECEVEZ M200!\n");
                                        player[i].argent += ARGENT_DEPART;
                                    }
                                }
                                printf("➡️ %s avance à la case %d : %s\n", player[i].Nom, player[i].position, plateau[player[i].position].nom);
                                appliquer_effet_case(&plateau[player[i].position], &player[i], player, de1, de2);
                                if (player[i].argent < 0 && player[i].enJeu)
                                {
                                    gererFaillite(&player[i], player, nbjoueurs, plateau, -1);
                                }
                                    
                                if (!player[i].enJeu || player[i].enPrison || (de1 != de2) || allerEnPrison3Doubles )
                                {
                                    continuerLancer = 0;
                                }
                                else
                                {
                                    printf("Vous rejouez !\n");
                                }
                                   
                            }
                        }
                        else
                        {
                            continuerLancer = 0;
                        }
                    }


                }

                // 1. Vérifier si l'objectif monétaire est atteint
                if (!jeufini && !session_finie && player[i].enJeu && player[i].argent >= objectif_monetaire)
                {
                    printf("\n🎉 %s a atteint l'objectif monétaire de %d M !\n", player[i].Nom, objectif_monetaire);
                    jeufini = 1; // Met fin au jeu
                }

                // 2. Si objectif non atteint, vérifier la faillite (fin de la partie)
                if (!jeufini && !session_finie)
                {
                    verifierFinDePartie(player, nbjoueurs, plateau, NB_CASES);
                }

            }
        }
       
    }
      // --- Affichage Final ---
        if (jeufini && !session_finie)
        {
            printf("\n--- Partie à objectif monétaire terminée ---\n");
            afficherTableauScore(player, nbjoueurs, plateau, NB_CASES);
            mettreAJourMeilleursScores();
        }
        else if (session_finie)
        {
            printf("\n--- Session de jeu terminée par l'utilisateur ---\n");
        }
       
}

// --- Vérification et affichage de la fin de partie ---
void verifierFinDePartie(struct joueur players[], int num_players, struct CasePlateau board[], int nbCases)
{
    int actifs = 0;
    int gagnantIndex = -1;
    int i;

    // Compter les joueurs actifs
    for (i = 0; i < num_players; i++)
    {
        if (players[i].enJeu)
        {
            actifs++;
            gagnantIndex = i;
        }
    }

    // Vérifier si la partie est terminée (1 ou 0 joueur actif)
    if (actifs <= 1)
    {
        // Mise à jour de l'état global et affichage de message de fin 
        if (!jeufini)
        {
            jeufini = 1; 
            printf("\n================== FIN DE PARTIE ==================\n");
            if (gagnantIndex != -1)
            { 
                printf("🎉 Victoire de %s ! 🎉\n", players[gagnantIndex].Nom);
            }
            else
            { 
                printf(" Partie terminée sans vainqueur clair...\n");
            }
             printf("===================================================\n");
        }
    }
}


//--- Initialisation du plateau --- 

void initialisation_plateau()
{
    char nomFichier[20] = "plateau.txt";
    FILE *f = fopen(nomFichier, "r");
    if (!f)
    {
        printf("Erreur : impossible d'ouvrir le fichier %s.\n", nomFichier);
    }
    else
    {
        char ligne[256];
        int i = 0;

        // On lit jusqu'à 40 lignes (NB_CASES) ou la fin du fichier
        while (fgets(ligne, sizeof(ligne), f) != NULL && i < NB_CASES) 
        {
            // Parcours manuel de la ligne pour supprimer le saut de ligne
            int j = 0;
            while (ligne[j] != '\0')
            {
                if (ligne[j] == '\n')
                {
                    ligne[j] = '\0';
                }
                j++;
            }


            char tmpNom[50], tmpAbrege[15];
            char tmpCouleur;
            int prixAchat, prixMaison, prixHotel;
            int loyerBase, loyer1m, loyer2m, loyer3m, loyer4m, loyerHotel, estHypothequee;
            int hypoth, type, proprietaire;
            int nbMaisons, Hotel;

            int nbChamps = sscanf(ligne, "%[^;];%[^;];%c;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d",
                                tmpNom, 
                                tmpAbrege,
                                &tmpCouleur, 
                                &prixAchat,  
                                &prixMaison, 
                                &loyerBase,  
                                &loyer1m,    
                                &loyer2m,    
                                &loyer3m,    
                                &loyer4m,    
                                &loyerHotel, 
                                &hypoth,     
                                &type,       
                                &proprietaire,
                                &estHypothequee,
                                &nbMaisons,
                                &Hotel);
            
            // Stockage des informations nécessaires

            if (nbChamps == 17)
            {
                strncpy(plateau[i].nom, tmpNom, sizeof(plateau[i].nom) - 1);
                strncpy(plateau[i].nom_abrege, tmpAbrege, sizeof(plateau[i].nom_abrege) - 1);
                plateau[i].nom[sizeof(plateau[i].nom) - 1] = '\0';
                plateau[i].couleur = tmpCouleur;
                plateau[i].prix_achat = prixAchat;
                plateau[i].prix_maison = prixMaison;
                plateau[i].loyer = loyerBase;
                plateau[i].loyer1m = loyer1m;
                plateau[i].loyer2m = loyer2m;
                plateau[i].loyer3m = loyer3m;
                plateau[i].loyer4m = loyer4m;
                plateau[i].loyerh = loyerHotel;
                plateau[i].hypotheque = hypoth;
                plateau[i].Type = type;
                plateau[i].proprietaire = proprietaire;
                plateau[i].estHypothequee = estHypothequee;
                plateau[i].nbMaisons = nbMaisons;
                plateau[i].Hotel = Hotel;
                i++;
            }

            else
            {
                printf("Erreur de format sur la ligne %d (\"%s\").\n", i + 1, ligne);
            }
        }

        fclose(f);
        /* Débogage : vérification de l'initialisation du plateau : 
        printf("Chargement du plateau : %d cases lues.\n", i);
        afficher_plateau(); */
    }
}


// --- Terminer partie : Demande une confirmation O/N au joueur pour mettre fin à la partie. Renvoie 1 si la partie doit s’arrêter, 0 sinon ---

int terminerPartie()
{
    char response[10];    
    int saisieOk = 0;     
    int confirmation = 0; 
    char c;               

    while (!saisieOk)
    {
        printf("Vous êtes sûr de vouloir terminer la partie? (O/N) : ");

        if (scanf("%9s", response) == 1)
        {
            viderBuffer();
            conv_maj(response); 

            // Vérifier si c'est 'O' ou 'N'
            if (response[0] == 'O' || response[0] == 'N')
            {
                saisieOk = 1; 
                if (response[0] == 'O')
                {
                    confirmation = 1; // L'utilisateur a confirmé
                }
                else
                {
                    confirmation = 0; // L'utilisateur a refusé
                }
            }
            else
            {
                printf("Réponse invalide. Veuillez entrer 'O' ou 'N'.\n");
            }
        }
        else
        {
            printf("Erreur de saisie. Veuillez entrer 'O' ou 'N'.\n");
            viderBuffer();
        }
    } 

    // Traitement basé sur la confirmation valide
    if (confirmation == 1)
    {
        printf("Vous avez choisi de terminer la partie.\n");
        jeufini = 1; // Mise à jour dela variable globale pour arrêter la boucle principale
        return 1;                     
    }
    else
    {
        printf("Reprise du jeu...\n");
        return 0; 
    }
}


// --- Calcule pour chaque joueur encore en jeu la valeur nette (cash + propriétés + constructions) puis affiche un tableau récapitulatif ---

void afficherTableauScore(const struct joueur players[], int nbjoueurs, const struct CasePlateau plateau[], int nbCases)
{
    int valeurProprietes, valeurConstructions;
    int score;
    // Affichage de l'en-tête du tableau
    printf("\n+-----------------------------------------------------------------------------------------------+\n");
    printf("| %-15s | %-8s | %-27s | %-25s | %-8s |\n",
           "Nom", "Argent", "Valeur des propriétés", "Valeur des constructions", "Score");
    printf("+-----------------------------------------------------------------------------------------------+\n");

    // Calcul et affichage pour chaque joueur en jeu
    for (int i = 0; i < nbjoueurs; i++)
    {
        if (players[i].enJeu)
        {
            valeurProprietes = 0;
            valeurConstructions = 0;

            // Parcours des propriétés du joueur
            for (int j = 0; j < players[i].nbProprietes; j++)
            {
                int idx = players[i].proprietes[j];

                // Calcul de la valeur de la propriété
                if (plateau[idx].estHypothequee)
                {
                    valeurProprietes += (plateau[idx].prix_achat - plateau[idx].hypotheque);
                }
                else
                {
                    valeurProprietes += plateau[idx].prix_achat;
                    // Calcul de la valeur des constructions
                    valeurConstructions += plateau[idx].nbMaisons * plateau[idx].prix_maison;
                    if (plateau[idx].Hotel)
                    {
                        valeurConstructions += plateau[idx].prix_maison;
                    }
                }
            }

            score = players[i].argent + valeurProprietes + valeurConstructions;

            // Affichage d'une ligne pour le joueur
            printf("| %-15s | %-8d | %-25d | %-25d | %-8d |\n", players[i].Nom, players[i].argent, valeurProprietes, valeurConstructions, score);
        }
    }
    printf("+-----------------------------------------------------------------------------------------------+\n");
}

// --- Gère l'abandon de la partie de la part d'un joueur --- 

void abandonnerPartie(struct joueur *p)
{
    int index_joueur = -1; 
    int c;                 

    printf("\n-----------------------------------------------\n");
    printf(">>> %s a décidé d'abandonner la partie. <<<\n", (*p).Nom);
    printf("-----------------------------------------------\n");

    (*p).enJeu = 0; 
    // 1. Vendre les constructions 
    printf("Vente des constructions de %s à la banque...\n", (*p).Nom);
    liquiderToutesConstructions(p, plateau); 

    // 2. Trouver l'index du joueur (Nécessaire car transfertBiens prend l'index numérique)
    for (int idx = 0; idx < nbjoueurs; idx++)
    {
        if (&player[idx] == p)
        {
            index_joueur = idx;
        }
    }

    

    // 3. Transfert des propriétés à la banque
    if (index_joueur != -1)
    {
        printf("Retour des propriétés de %s à la banque...\n", (*p).Nom);
        transfertBiens(p, NULL, plateau, index_joueur, -1); 
    }
    else
    {
        printf("Erreur critique : Impossible de trouver l'index du joueur %s !\n", (*p).Nom);
        // Au minimum, vider manuellement la liste du joueur par sécurité
        (*p).nbProprietes = 0;
        for (c = 0; c < MAX_PROPRIETES; c++)
        {                            
            (*p).proprietes[c] = -1; // Utiliser -1 pour vide
        }
    }

    // 4. Remettre les cartes "Sortie de Prison"
   if ((*p).cartesSortiePrison > 0) 
   {
       printf("Retour des %d carte(s) 'Sortie de prison' de %s...\n", (*p).cartesSortiePrison, (*p).Nom);
        
       // Vérifier et remettre la carte Chance si elle était possédée
       if ((*p).libereChance > 0) 
       {
           struct Carte carteRetourChance;
           strcpy(carteRetourChance.type, "Chance"); 
           carteRetourChance.effet = 2;
           carteRetourChance.valeur = 0;
           strcpy(carteRetourChance.description, "VOUS ÊTES LIBÉRÉ DE PRISON. Cette carte peut être conservée jusqu'à ce qu'elle soit utilisée, échangée ou vendue.");

           reintegrerCarteSortieDePrison(carteRetourChance); // Remise dans la bonne pioche
           printf(" -> Une carte 'Sortie de Prison' (Chance) remise dans la pioche.\n");

           (*p).libereChance = 0; 
       }

       // Vérifier et remettre la carte Communauté si elle était possédée
       if ((*p).libereCommunaute > 0) 
       {
           struct Carte carteRetourCommunaute;
           strcpy(carteRetourCommunaute.type, "Communauté"); 
           carteRetourCommunaute.effet = 2;
           carteRetourCommunaute.valeur = 0;
           strcpy(carteRetourCommunaute.description, "Ouaf, Ouaf ! Vous adoptez un chiot dans un refuge ! VOUS ÊTES LIBÉRÉ DE PRISON. Conservez cette carte jusqu'à ce qu'elle soit utilisée, échangée ou vendue.");

           reintegrerCarteSortieDePrison(carteRetourCommunaute); // Remise dans la bonne pioche
           printf(" -> Une carte 'Sortie de Prison' (Communaute) remise dans la pioche.\n");

           (*p).libereCommunaute = 0; 
       }

       (*p).cartesSortiePrison = 0;
   }

    printf("Argent restant de %s (%d M) remis à la banque.\n", (*p).Nom, (*p).argent);
    (*p).argent = 0;


    printf("\n***********************************************\n");
    printf("*       %s a officiellement abandonné !        *\n", (*p).Nom);
    printf("***********************************************\n\n");
}



// --- Liquide toutes les constructions — maisons et hôtels — d’un joueur sur chacune de ses propriétés et remet les compteurs à zéro dans le plateau ---

void liquiderToutesConstructions(struct joueur *p, struct CasePlateau board[])
{
    int constructionsVendues = 0; // Juste pour le feedback

    printf("Liquidation de toutes les constructions de %s...\n", (*p).Nom);

    for (int i = 0; i < (*p).nbProprietes; i++) 
    {
        int propIndex = (*p).proprietes[i];

        // Vérifier si l'index est valide et si la case appartient bien ENCORE au joueur
        if (propIndex >= 0 && propIndex < NB_CASES && board[propIndex].proprietaire == (p - player)) // Comparer l'index réel
        {
            // Vente de l'hôtel (s'il existe)
            if (board[propIndex].Hotel > 0)
            {
                printf("...Vente d'un Hôtel sur %s.\n", board[propIndex].nom);
                board[propIndex].Hotel = 0; // Mise à jour du plateau
                constructionsVendues++;
            }

            // Vente des maisons (s'il en existe)
            if (board[propIndex].nbMaisons > 0)
            {
                printf("...Vente de %d Maison(s) sur %s.\n", board[propIndex].nbMaisons, board[propIndex].nom);
                constructionsVendues += board[propIndex].nbMaisons; 
                board[propIndex].nbMaisons = 0;                     // Mise à jour du plateau
            }
            
        }
    } 

    if (constructionsVendues == 0)
    {
        printf("...%s n'avait aucune construction à liquider.\n", (*p).Nom);
    }
    else
    {
        printf("...Liquidation terminée.\n");
    }
}


// --- Fonction permettant la vente des constructions --- 

int vendreConstructions(struct joueur *p, struct CasePlateau board[])
{
    int fondsLeves = 0;
    int nbOptions, choix = -1, i; 
    int sousChoix, montant;
    int optionsIndex[MAX_PROPRIETES];

    // --- Boucle pour la vente volontaire (argent > 0 et choix != 0) ---
    while ((*p).argent > 0 && choix != 0)
    {
        // 1. Refaire la liste des options vendables 
        nbOptions = 0;
        printf("\n--- Vente de constructions (Solde = %d M) ---\n", (*p).argent);
        printf("Propriétés avec constructions vendables :\n");
        for (i = 0; i < (*p).nbProprietes; i++)
        {
            int idx = (*p).proprietes[i];
            if (idx >= 0) // Vérifier si l'entrée propriete est valide
            {
                struct CasePlateau *prop_ptr = &board[idx];
                if ((*prop_ptr).Hotel > 0 || (*prop_ptr).nbMaisons > 0)
                {
                    optionsIndex[nbOptions] = idx; // Stocker l'index réel du plateau
                    nbOptions++;
                    printf("%2d) %-20s (%d maisons, %d hôtel)\n", nbOptions, (*prop_ptr).nom_abrege, (*prop_ptr).nbMaisons, (*prop_ptr).Hotel);
                }
            }
        }

        // 2. Vérifier s'il y a des options 
        if (nbOptions == 0)
        {
            printf("Aucune construction à vendre.\n");
            choix = 0; 
        }
        else
        {
            // 3. Menu de sortie volontaire 
            printf(" 0) Retour\n");
            printf("Choisissez la propriété pour vendre (1-%d) : ", nbOptions);

            if (scanf("%d", &choix) != 1)
            {
                viderBuffer();
                printf("Entrée non valide. Réessayez.\n");
                choix = -1; 
            }
            else
            {
                viderBuffer();
                if (choix >= 1 && choix <= nbOptions)
                {
                    // --- Propriété choisie ---
                    int idxPropChoisie = optionsIndex[choix - 1]; 
                    struct CasePlateau *prop = &board[idxPropChoisie]; // Pointeur vers la propriété choisie
                    char couleurGroupe = (*prop).couleur;
                    int playerIndex = (int)(p - player); // Index du joueur

                    printf("Sur %s, que voulez‑vous vendre ?\n", (*prop).nom_abrege);
                    if ((*prop).Hotel > 0)
                    {
                        printf(" 1) Hôtel (Revenu: %d M)\n", (*prop).prix_maison / 2);
                    }      
                    if ((*prop).nbMaisons > 0)
                    {
                        printf(" 2) Maison (Revenu: %d M)\n", (*prop).prix_maison / 2);
                    } 
                    printf(" 0) Annuler pour cette propriété\n");
                    printf("Votre choix (0-2) : ");

                    if (scanf("%d", &sousChoix) != 1)
                    {
                        viderBuffer(); printf("Entrée non valide.\n");
                    }
                    else
                    {
                        viderBuffer();
                        montant = (*prop).prix_maison / 2; 

                        // --- Logique de Vente Hôtel ---
                        if (sousChoix == 1 && (*prop).Hotel > 0)
                        {
                             (*prop).Hotel = 0;
                             // Règle officielle: un hôtel vendu remet 4 maisons
                             (*prop).nbMaisons = 4;
                             (*p).argent += montant;
                             fondsLeves += montant;
                             printf("🏨 Hôtel vendu sur %s pour %d M (devient 4 maisons).\n", (*prop).nom_abrege, montant);
                        }
                        // --- Logique de Vente Maison ---
                        else if (sousChoix == 2 && (*prop).nbMaisons > 0)
                        {
                            // Vérifier l'équilibre de vente
                            int maxNiveauGroupe = 0; // 0=nu, 1-4=maisons, 5=hôtel
                            int venteMaisonPossible = 0;
                            int k; 

                            for(k=0; k < NB_CASES; k++) 
                            {
                                 if(board[k].couleur == couleurGroupe && board[k].proprietaire == playerIndex) 
                                 {
                                    int currentLevel = 0;
                                    if (board[k].Hotel > 0) 
                                    {
                                        currentLevel = 5;
                                    } 
                                    else 
                                    {
                                        currentLevel = board[k].nbMaisons;
                                    }
                                    if(currentLevel > maxNiveauGroupe) 
                                    {
                                        maxNiveauGroupe = currentLevel;
                                    }
                                 }
                            }
                            
                            // S'il n'y a pas d'hôtels dans le groupe
                            if (maxNiveauGroupe <= 4) 
                            {
                                if ((*prop).nbMaisons == maxNiveauGroupe) 
                                {
                                    venteMaisonPossible = 1;
                                } 
                                else 
                                {
                                     printf("Vente impossible : vous devez vendre des maisons sur les propriétés plus développées (niveau %d) du groupe d'abord.\n", maxNiveauGroupe);
                                }
                            } 
                            // Il y a au moins un hôtel dans le groupe
                            else 
                            { 
                                printf("Vente impossible : vous devez vendre le(s) hôtel(s) du groupe avant de vendre des maisons.\n");
                            }


                            if (venteMaisonPossible) 
                            {
                                (*prop).nbMaisons--;
                                (*p).argent += montant;
                                fondsLeves += montant;
                                printf("🏠 Maison vendue sur %s pour %d M. (%d restantes)\n", (*prop).nom_abrege, montant, (*prop).nbMaisons);
                            }
                        }
                        // --- Annulation ou Choix Invalide ---
                        else if (sousChoix == 0) 
                        {
                            printf("Vente annulée pour cette propriété.\n");
                        } 
                        else 
                        {
                            printf("Option invalide ou construction inexistante.\n");
                        }
                    } 
                } 
                else if (choix == 0) 
                {
                    printf("Retour au menu précédent.\n");
                } 
                else 
                { 
                    printf("Choix hors‑bornes (0 à %d).\n", nbOptions);
                }
            } 
        } 
    } 

    // --- Boucle pour la vente forcée (argent <= 0) ---
    // La boucle continue tant que le joueur est à découvert ET qu'il y a des options
    int continuerVenteForcee = ((*p).argent <= 0); 
    while (continuerVenteForcee)
    {
        // 1. Refaire la liste des options vendables
        nbOptions = 0;
        printf("\n--- VENTE FORCÉE (Solde = %d M) ---\n", (*p).argent);
        printf("Propriétés avec constructions vendables :\n");
        for (i = 0; i < (*p).nbProprietes; i++)
        {
            int idx = (*p).proprietes[i];
            if (idx >= 0)
            {
                struct CasePlateau *prop_ptr = &board[idx];
                if ((*prop_ptr).Hotel > 0 || (*prop_ptr).nbMaisons > 0)
                {
                    optionsIndex[nbOptions] = idx;
                    nbOptions++;
                    printf("%2d) %-20s (%d maisons, %d hôtel)\n",nbOptions,(*prop_ptr).nom_abrege,(*prop_ptr).nbMaisons,(*prop_ptr).Hotel);
                }
            }
        }

        // 2. Si rien à vendre -> sortir (faillite gérée ailleurs) 
        if (nbOptions == 0)
        {
            printf("❌ Aucune construction restante à vendre !\n");
            continuerVenteForcee = 0; // Arrêter la boucle while
        }
        else
        {
            // 3. Demander au joueur quoi vendre (pas d'option 0 ici) 
            printf("Choisissez la propriété pour vendre (1-%d) : ", nbOptions);

            if (scanf("%d", &choix) != 1)
            {
                viderBuffer();
                printf("Entrée non valide. Réessayez.\n");
                // On continue la boucle while
            }
            else
            {
                viderBuffer();
                if (choix >= 1 && choix <= nbOptions)
                {
                    // --- Propriété choisie ---
                    int idxPropChoisie = optionsIndex[choix - 1];
                    struct CasePlateau *prop = &board[idxPropChoisie];
                    char couleurGroupe = (*prop).couleur;
                    int playerIndex = (int)(p - player);

                    printf("Sur %s, que vendre ?\n", (*prop).nom_abrege);
                    if ((*prop).Hotel > 0)
                    {
                        printf(" 1) Hôtel (Revenu: %d M)\n", (*prop).prix_maison / 2);
                    }
                    if ((*prop).nbMaisons > 0)
                    {
                        printf(" 2) Maison (Revenu: %d M)\n", (*prop).prix_maison / 2);
                    }
                    printf("Votre choix (1-2) : ");

                    if (scanf("%d", &sousChoix) != 1) 
                    {
                        viderBuffer(); printf("Entrée non valide.\n");
                    } 
                    else 
                    {
                        viderBuffer();
                        montant = (*prop).prix_maison / 2;

                        // --- Logique Vente Hôtel (forcée) ---
                        if (sousChoix == 1 && (*prop).Hotel > 0) 
                        {
                            (*prop).Hotel = 0; (*prop).nbMaisons = 4; // Remettre 4 maisons
                            (*p).argent += montant; fondsLeves += montant;
                            printf("🏨 Hôtel vendu sur %s pour %d M (devient 4 maisons).\n", (*prop).nom_abrege, montant);
                        }
                        // --- Logique Vente Maison (forcée) ---
                        else if (sousChoix == 2 && (*prop).nbMaisons > 0) 
                        {
                            int maxNiveauGroupe = 0; int venteMaisonPossible = 0; int k;
                            for(k=0; k < NB_CASES; k++) 
                            {
                                if (board[k].couleur == couleurGroupe && board[k].proprietaire == playerIndex)
                                {
                                    int lvl; 
                                    if(board[k].Hotel)
                                    {
                                        lvl = 5;
                                    }
                                    else
                                    {
                                        lvl = board[k].nbMaisons; 
                                    }
                                    if (lvl > maxNiveauGroupe)
                                    {
                                        maxNiveauGroupe = lvl;
                                    }
                                   
                                }
                            }
                            int niveauPropChoisie = (*prop).nbMaisons; // Pas d'hôtel ici sinon on aurait choisi 1
                            if (maxNiveauGroupe <= 4 && niveauPropChoisie == maxNiveauGroupe) 
                            { 
                                venteMaisonPossible = 1; 
                            }
                            else 
                            { 
                                printf("Vente impossible : équilibrez d'abord le groupe.\n"); 
                            }

                            if(venteMaisonPossible) 
                            {
                                (*prop).nbMaisons--; (*p).argent += montant; fondsLeves += montant;
                                printf("🏠 Maison vendue sur %s pour %d M. (%d restantes)\n", (*prop).nom_abrege, montant, (*prop).nbMaisons);
                            }
                        } 
                        else 
                        {
                            printf("Option invalide.\n");
                        }
                    } 
                } 
                else 
                { 
                    printf("Choix hors‑bornes (1 à %d).\n", nbOptions);
                }
            } 

             // Mise à jour de la condition de boucle pour sortir si l'argent est maintenant positif
             if ((*p).argent > 0) 
             {
                  continuerVenteForcee = 0;
             }

        } 
    } 

    // Message final si rien n'a été vendu 
    if (fondsLeves == 0 && choix == 0) 
    {
        printf("Vous n'avez rien vendu.\n");
    }
    return fondsLeves;
}



// --- Procédure gérant le transfert des biens d'un joueur à son créancier (banque ou un autre joueur) ---
void transfertBiens(struct joueur *pSource, struct joueur *pDest, struct CasePlateau board[], int joueurIndex, int destIndex)
{
    int i; 

    // Message initial
    if (pDest != NULL) 
    {
        printf("Transfert des biens de %s vers %s.\n", (*pSource).Nom, (*pDest).Nom);
    } 
    else 
    {
        printf("Transfert des biens de %s vers la Banque.\n", (*pSource).Nom);
    }

    // 1. Parcourir TOUT le plateau pour trouver les propriétés du joueur source
    for (i = 0; i < NB_CASES; i++)
    {
        // Vérifier si la case est une propriété appartenant au joueur source
        if (board[i].proprietaire == joueurIndex)
        {
            // --- Affectation initiale du nouveau propriétaire ---
            board[i].proprietaire = destIndex;

            // --- Réinitialisation Maisons/Hôtels ---
            board[i].nbMaisons = 0;
            board[i].Hotel = 0;

            // --- Gestion spécifique au destinataire ---
            if (destIndex == -1) // Transfert à la Banque
            {
                printf(" -> Propriété '%s' retournée à la Banque.\n", board[i].nom_abrege);
                board[i].estHypothequee = 0;
                 // Lancement de l’enchère auprès des joueurs encore en jeu 
                enchere(&board[i],i,player,nbjoueurs);
       
            }
            else // Transfert à un autre joueur
            {
                // Affichage initial
                if (board[i].estHypothequee)
                {
                    printf(" -> '%s' transférée (HYPOTHÉQUÉE) à %s\n",board[i].nom_abrege,(*pDest).Nom);
                }
                else
                {
                    printf(" -> '%s' transférée à %s\n",board[i].nom_abrege,(*pDest).Nom);
                }
            
                // Si la propriété est hypothéquée, proposer au destinataire de lever ou non
                if (board[i].estHypothequee)
                {
                    int hypo = board[i].hypotheque;
                    int interet  = (hypo + 9) / 10;       
                    int total = hypo + interet;
                    int choixHypo = 0;
                    int saisieOkHypo = 0;
            
                    printf("    %s, pour '%s' :\n", (*pDest).Nom, board[i].nom_abrege);
                    printf("      1. Lever l'hypothèque (Coût total %d M = %d + %d d'intérêt)\n", total, hypo, interet);
                    printf("      2. Conserver l'hypothèque (Payer %d M d'intérêt)\n", interet);
            
                    // Boucle de saisie du choix
                    while (!saisieOkHypo)
                    {
                        printf("    Votre choix (1-2) ? ");
                        if (scanf("%d", &choixHypo) == 1)
                        {
                            viderBuffer();
                            if (choixHypo == 1 || choixHypo == 2)
                            {
                                saisieOkHypo = 1;

                            }
                            else
                            {
                                printf("    Choix invalide. Entrez 1 ou 2.\n");

                            }
                        }
                        else
                        {
                            viderBuffer();
                            printf("    Entrée invalide. Veuillez entrer 1 ou 2.\n");
                        }
                    }
            
                    // Application du choix
                    if (choixHypo == 1)
                    {
                        // Lever l'hypothèque
                        if ((*pDest).argent >= total)
                        {
                            (*pDest).argent -= total;
                            board[i].estHypothequee = 0;
                            printf("    Hypothèque levée pour %d M. Solde de %s : %d M\n", total, (*pDest).Nom, (*pDest).argent);
                        }
                        else
                        {
                            printf("    Fonds insuffisants pour lever (%d M requis). Vous conservez l'hypothèque.\n", total);
                            choixHypo = 2;
                        }
                    }
            
                    if (choixHypo == 2) // Payer uniquement l'intérêt et conserver l'hypothèque
                    {
                        
                        (*pDest).argent -= interet;
                        printf("    %s a payé %d M d'intérêt. Nouveau solde : %d M\n", (*pDest).Nom, interet, (*pDest).argent);
                        if ((*pDest).argent < 0 && (*pDest).enJeu)
                        {
                            printf("    ATTENTION : %s est en difficulté après paiement de l'intérêt !\n", (*pDest).Nom);
                            gererFaillite(pDest, player, nbjoueurs, board, -1);
                        }
                    }
                }
            
                // Mise à jour de la liste des propriétés du destinataire
                if ((*pDest).enJeu)
                {
                    if ((*pDest).nbProprietes < MAX_PROPRIETES)
                    {
                        (*pDest).proprietes[(*pDest).nbProprietes] = i;
                        (*pDest).nbProprietes++;
                    }
                    else
                    {
                        printf("    Erreur : %s ne peut pas détenir plus de propriétés !\n", (*pDest).Nom);
                    }
                }
            }
            
        } 
    } 

    // 2. Nettoyer la liste des propriétés du joueur source (pFailli)
    if (pSource != NULL)
    {
        printf("Nettoyage de la liste des propriétés de %s.\n", (*pSource).Nom);
        (*pSource).nbProprietes = 0; 
        int j;
        for (j = 0; j < MAX_PROPRIETES; j++)
        {
            (*pSource).proprietes[j] = -1; 
        }
    }
} 


// --- Réintégration d'une carte de sortie de prison dans la pioche correspondante ---

void reintegrerCarteSortieDePrison(struct Carte carte)
{
    if (strcmp(carte.type, "Chance") == 0) // Si le type de la carte est "Chance"
    {
        if (nbCartesChance < MAX_CARTES)
        {
            cartes_chance[nbCartesChance] = carte; // On rajoute la carte à la fin 
            nbCartesChance++;                      
        }
        else
        {
            printf("Erreur: Impossible de réintégrer la carte, la pile Chance est pleine !\n");
        }
    }

    else if (strcmp(carte.type, "Communauté") == 0) // Si le type est "Communauté"
    {
        if (nbCartesCommunaute < MAX_CARTES)
        {
            cartes_communaute[nbCartesCommunaute] = carte; // On rajoute la carte à la fin 
            nbCartesCommunaute++;                          
        }
        else
        {
            printf("Erreur: Impossible de réintégrer la carte, la pile Communauté est pleine !\n");
        }
    }
    else
    {
        // Gérer un type de carte inconnu ou inattendu
        printf("Erreur: Type de carte '%s' inconnu pour la réintégration de la carte 'Sortie de Prison'.\n", carte.type);
    }
}



// --- Propose à un joueur de bâtir des maisons ou un hôtel sur l’une de ses propriétés éligibles --- 

void construire(struct joueur *p, struct CasePlateau plateau[], int nbCases)
{
    int choix, indice_propriete, nb_maisons;
    int compteur = 0;
    int saisieOk = 0;
    int constructionEquilibree;
    int propriete_indices[40]; // indices des propriétés éligibles

    // Affiche et remplit propriete_indices[], compteur = nombre de choix
    afficher_constructions_par_couleur(p, plateau, nbCases, propriete_indices, &compteur);

    if (compteur == 0)
    {
        printf("Aucune propriété éligible pour construction.\n");
    }
    else
    {
        // Choix de la propriété
        saisieOk = 0;
        while (!saisieOk)
        {
            printf("Sélectionnez une propriété (1 à %d, 0 pour annuler) : ", compteur);
            saisieOk = scanf("%d", &choix);
            viderBuffer();
            if (!saisieOk)
            {
                printf("Entrée invalide. Veuillez entrer un nombre.\n");
            }
            else if (choix < 0 || choix > compteur)
            {
                printf("Choix invalide.\n");
                saisieOk = 0;
            }
        }

        if (choix != 0)
        {
            indice_propriete = propriete_indices[choix - 1];
            char couleurGroupe = plateau[indice_propriete].couleur;
            int playerIndex = p - player; 

            // 2) MAISON (si nbMaisons < 4)
            if (plateau[indice_propriete].nbMaisons < 4)
            {
                int minMaisons = 1000;
                int groupCount = 0;
                for (int i = 0; i < nbCases; i++)
                {
                    if (plateau[i].Type == 0 && plateau[i].couleur == couleurGroupe && plateau[i].proprietaire == playerIndex)
                    {
                        groupCount++;
                        if (plateau[i].nbMaisons < minMaisons)
                        {
                            minMaisons = plateau[i].nbMaisons;
                        }
                    }
                }

                // 2.b) règle d'équilibre des constructions du groupe

                constructionEquilibree = 1;

                if (groupCount > 1 && plateau[indice_propriete].nbMaisons > minMaisons)
                {
                    printf("Construction impossible : vous devez d'abord équilibrer les maisons du groupe.\n");
                    constructionEquilibree = 0;
                }

                // 2.c) calcul de la borne haute 

                int allowedAddition = 0;

                if (constructionEquilibree)
                {
                    if (groupCount > 1)
                    {
                        allowedAddition = (minMaisons + 1) - plateau[indice_propriete].nbMaisons;
                    }
                    else
                    {
                        allowedAddition = 4 - plateau[indice_propriete].nbMaisons;

                    }

                    if (allowedAddition < 1)
                    {
                        printf("Vous ne pouvez pas construire davantage sur '%s'.\n", plateau[indice_propriete].nom);
                        constructionEquilibree = 0;
                    }
                }

                if (constructionEquilibree)
                {
                    saisieOk = 0;
                    nb_maisons = 1;
                    int cout_total = nb_maisons * plateau[indice_propriete].prix_maison;

                    if ((*p).argent < cout_total)
                    {
                        printf("Fonds insuffisants. Coût %d M, vous avez %d M.\n",cout_total, (*p).argent);
                    }

                    else
                    {
                        (*p).argent -= cout_total;
                        plateau[indice_propriete].nbMaisons += nb_maisons;
                        printf("%d maison ajoutée sur %s.\n", nb_maisons, plateau[indice_propriete].nom);
                    }
                }
            }

            // 3) HOTEL si nbMaisons == 4

            else if (plateau[indice_propriete].nbMaisons == 4 && plateau[indice_propriete].Hotel == 0)
            {
                int hot_construction = 1;
                for (int i = 0; i < nbCases; i++)
                {
                    if (plateau[i].Type == 0 && plateau[i].couleur == plateau[indice_propriete].couleur && plateau[i].proprietaire == playerIndex)
                    {

                        if (!(plateau[i].nbMaisons == 4 || plateau[i].Hotel == 1))
                        {
                            hot_construction = 0;
                        }
                    }
                }

                if (!hot_construction)
                {
                    printf("Équilibrez la construction des maisons/hôtels sur chaque rue du groupe.\n");
                }

                else
                {
                    saisieOk = 0;
                    char reponse[10];
                    while (!saisieOk)
                    {
                        printf("Construire un hôtel sur %s ? (O/N) : ", plateau[indice_propriete].nom);
                        saisieOk = scanf("%9s", reponse);
                        viderBuffer();
                        conv_maj(reponse);

                        if (reponse[0] != 'O' && reponse[0] != 'N')
                        {
                            printf("Réponse invalide. Entrez 'O' ou 'N'.\n");
                            saisieOk = 0;
                        }
                    }

                    if (reponse[0] == 'O')
                    {
                        int coutH = plateau[indice_propriete].prix_maison;
                        if ((*p).argent < coutH)
                        {
                            printf("Fonds insuffisants pour hôtel (%d M).\n", coutH);
                        }
                        else
                        {
                            (*p).argent -= coutH;
                            plateau[indice_propriete].nbMaisons = 0;
                            plateau[indice_propriete].Hotel = 1;
                            printf("Hôtel construit sur %s !\n", plateau[indice_propriete].nom);
                        }
                    }
                    else
                    {
                        printf("Construction d'hôtel annulée.\n");
                    }
                }
            }
            // 4) Ailleurs on ne peut plus rien faire 
            else
            {
                printf("Cette propriété ne peut plus être améliorée.\n");
            }
        }
    }
}


// Affiche, par groupe de couleur, les propriétés du joueur où une construction est encore possible et renvoie leurs indices

void afficher_constructions_par_couleur(struct joueur *p, struct CasePlateau plateau[], int nbCases, int *propriete_indices, int *nbProps)
{
    char couleursDejaAffichees[10]; 
    int nbCouleurs = 0;
    *nbProps = 0;

    printf("\n=== Propriétés disponibles pour construction ===\n");

    for (int i = 0; i < (*p).nbProprietes; i++)
    {
        int idx = (*p).proprietes[i];

        if (plateau[idx].Type == 0 && plateau[idx].Hotel == 0 && tous_couleurs(plateau, nbCases, plateau[idx].couleur, p - player))
        {
            int dejaAffichee = 0;
            int c = 0;
            while (c < nbCouleurs && couleursDejaAffichees[c] != plateau[idx].couleur)
            {
                c++;
            }
            dejaAffichee = (c < nbCouleurs);

            if (!dejaAffichee && plateau[idx].couleur != 'P' && plateau[idx].couleur != 'G') // Il ne faut pas considérer les services publics ni les gares
            {
                couleursDejaAffichees[nbCouleurs++] = plateau[idx].couleur;

                // Réafficher toutes les propriétés du groupe
                for (int j = 0; j < (*p).nbProprietes; j++)
                {
                    int idx2 = (*p).proprietes[j];

                    if (plateau[idx2].Type == 0 && plateau[idx2].couleur == plateau[idx].couleur && plateau[idx2].Hotel == 0 && tous_couleurs(plateau, nbCases, plateau[idx2].couleur, p - player))
                    {
                        printf("%d. %-25s (Maisons: %d, Hôtel: %d, Prix maison: M%d)\n", *nbProps + 1, plateau[idx2].nom, plateau[idx2].nbMaisons, plateau[idx2].Hotel, plateau[idx2].prix_maison);

                        propriete_indices[*nbProps] = idx2;

                        (*nbProps)++;
                    }
                }
            }
        }
    }
}


// --- Fonction permettant de vérifier si le joueur détient toutes les propriétés d'une couleur donnée ---

int tous_couleurs(struct CasePlateau plateau[], int nbCases, char couleur, int player_id)
{
    int response = 0;
    // On ignore la couleur 'A' qui ne correspond pas à un groupe immobilier
    if (couleur == 'A')
    {
        return response;
    }
    for (int i = 0; i < nbCases; i++)
    {
        // On ne considère que les cases qui correspondent à la couleur recherchée
        if (plateau[i].couleur == couleur)
        {
            // Si une case n'est pas possédée par le joueur, le monopole n'est pas obtenu
            if (plateau[i].proprietaire != player_id)
            {
                return response;
            }
        }
    }
    response = 1;
    return response; 
}

// --- MENU PRISON ---

int menuPrison(struct joueur *p, struct joueur players[], struct CasePlateau plateau[], int nbCases)
{
    int choix = -1;
    int saisieOk;
    char c;
    int prisonResolved = 0;

    while ((*p).enPrison && !prisonResolved)
    {
        choix = -1;
        saisieOk = 0;

        while (!saisieOk)
        {
            printf("\n--- Menu Prison pour %s (Tour %d/3) ---\n", (*p).Nom, (*p).tourEnPrison + 1);
            printf("Argent actuel : %d M\n", (*p).argent);
            printf("Que voulez-vous faire ?\n");
            printf("1. Utiliser une carte 'Sortie de prison'\n");
            printf("2. Payer 50 M pour sortir\n");
            printf("3. Tenter votre chance\n");
            printf("4. Proposer une négociation\n");
            printf("5. Vendre ou hypothèquer des propriétés\n");
            printf("Entrez votre choix (1-5) : ");
            saisieOk = scanf("%d", &choix);
            if (saisieOk)
            {
                viderBuffer(); 

                if (choix >= 1 && choix >= 6)
                {
                    saisieOk = 1;
                }
                else
                {
                    printf("Choix invalide. Entrez un nombre entre 1 et 5.\n");
                }
            }
            else
            {
                printf("Entrée invalide. Veuillez entrer un nombre.\n");
                viderBuffer();
            }
        }

        switch (choix)
        {
        case 1: // Utiliser la carte de sortie de prison
            if ((*p).cartesSortiePrison > 0)
            {
                printf("Vous utilisez une carte 'Sortie de prison' !\n");
                struct Carte carteUtilisee;
                if ((*p).libereChance > 0) // Remise de la carte chance
                {
                    strcpy(carteUtilisee.type, "Chance"); 
                    carteUtilisee.effet = 2;
                    carteUtilisee.valeur = 0;
                    strcpy(carteUtilisee.description, "VOUS ÊTES LIBÉRÉ DE PRISON. Cette carte peut être conservée jusqu'à ce qu'elle soit utilisée, échangée ou vendue.");
                    reintegrerCarteSortieDePrison(carteUtilisee); 
                    (*p).cartesSortiePrison--;
                    (*p).libereChance--;
                    (*p).enPrison = 0;
                    (*p).tourEnPrison = 0;
                    printf("%s est libéré de prison !\n", (*p).Nom);
                    prisonResolved = 1;
                }
                else // Remise de la carte commuanuté
                {
                    strcpy(carteUtilisee.type, "Communauté"); 
                    carteUtilisee.effet = 2;
                    carteUtilisee.valeur = 0;
                    strcpy(carteUtilisee.description, "Ouaf, Ouaf ! Vous adoptez un chiot dans un refuge ! VOUS ÊTES LIBÉRÉ DE PRISON. Conservez cette carte jusqu'à ce qu'elle soit utilisée, échangée ou vendue.");
                    reintegrerCarteSortieDePrison(carteUtilisee); 
                    (*p).cartesSortiePrison--;
                    (*p).libereCommunaute--;
                    (*p).enPrison = 0;
                    (*p).tourEnPrison = 0; 
                    printf("%s est libéré de prison !\n", (*p).Nom);
                    prisonResolved = 1;
                }
                return 1;
            }
            else
            {
                printf("Vous n'avez pas de carte \"Sortie de prison\"\n");
            }
            break;

        case 2: // Payer M 50 pour sortir de prison
            if ((*p).argent >= 50)
            {

                printf("%s est libéré de prison !\n", (*p).Nom);
                (*p).tourEnPrison = 0;
                (*p).enPrison = 0;
                (*p).argent -= 50;
                prisonResolved = 1;
                return 1;
            }
            else
            {
                printf("Vous n'avez pas assez d'argent!\n");
            }
            break;

        case 3: // Tenter sa chance pour sortir de prison
            printf("Vous tentez de faire un double pour sortir...\n");
            int de1 = lancer_de();
            int de2 = lancer_de();
            printf("Lancer des dés : %d et %d\n", de1, de2);

            if (de1 == de2)
            {
                printf("Bravo, vous avez fait un double ! Vous êtes libéré.\n");
                (*p).enPrison = 0;
                (*p).tourEnPrison = 0;
                (*p).position += de1 + de2;
                printf("➡️ %s avance à la case %d : %s\n", (*p).Nom, (*p).position, plateau[(*p).position].nom);
                appliquer_effet_case(&plateau[(*p).position], p, players, de1, de2);
                return 0;
            }
            else
            {
                printf("Pas de double... Vous devez rester en prison ce tour\n");
            }
            prisonResolved = 1;
            break;
        case 4:
            printf("Ouverture du menu de négociation...\n");
            negocier(p, players, nbjoueurs, plateau, NB_CASES);
            printf("Retour au menu de la prison.\n");
            break;
        case 5:
        {
            int sub_choix = -1;
            int sub_saisieOk = 0;

            while (sub_choix != 0)
            {
                sub_saisieOk = 0;
                sub_choix = -1; 

                // Affichage du sous-menu
                printf("\n--- Gestion des Propriétés (en prison) ---\n");
                printf("  1. Vendre des constructions (maisons/hôtels)\n");
                printf("  2. Hypothéquer une propriété\n");
                printf("  3. Lever une hypothèque\n");
                printf("  0. Retour au menu précédent (Prison)\n");

                while (!sub_saisieOk)
                {
                    printf("Votre choix (0-3) : ");
                    sub_saisieOk = scanf("%d", &sub_choix);
                    if (sub_saisieOk)
                    {
                        viderBuffer(); 

                        if (sub_choix >= 0 && sub_choix <= 3)
                        {
                            sub_saisieOk = 1; 
                        }
                        else
                        {
                            printf("Choix invalide. Entrez un nombre entre 0 et 3.\n");
                        }
                    }
                    else
                    {
                        printf("Entrée invalide. Veuillez entrer un nombre.\n");
                        viderBuffer(); 
                    }
                } 

                // Traitement du sous-choix
                switch (sub_choix)
                {
                case 1:
                    vendreConstructions(p, plateau);
                    break;
                case 2:
                    hypothequerPropriete(p, plateau, nbCases);
                    break;
                case 3:
                    leverHypotheque(p, plateau, nbCases);
                    break;
                case 0:
                    printf("Retour au menu principal de la prison...\n");
                    break;
                default:
                    printf("Entrez une option valide\n");
                    break;
                }
            }
        }
        }
    }
    return 0;
}


// --- Hypothèque : propose les propriétés non hypothéquées du joueur, applique la mise en gage et crédite 50 % du prix d’achat ---

void hypothequerPropriete(struct joueur *p, struct CasePlateau plateau[], int nbCases)
{
    int choix = -1; 
    int saisieOk = 0;
    int compteur_eligibles = 0;            // Compte combien de propriétés sont affichées
    int indices_eligibles[MAX_PROPRIETES]; // Stocke les indices des propriétés affichées
    int i;                                 

    if ((*p).nbProprietes == 0)
    {
        printf("Vous ne possédez aucune propriété.\n");
    }
    else 
    {

        // 1. Afficher la liste et stocker les indices éligibles

        printf("\n--- Hypothéquer une Propriété ---\n");
        printf("Propriétés non hypothéquées que vous possédez :\n");

        // Boucle d'affichage
        for (i = 0; i < (*p).nbProprietes; i++)
        {
            int idx_plateau = (*p).proprietes[i]; 

            // Vérifier si l'index est valide dans le tableau proprietes ET si la propriété sur le plateau n'est pas déjà hypothéquée

            if (idx_plateau >= 0 && idx_plateau < NB_CASES && !plateau[idx_plateau].estHypothequee)
            {
                printf("  %d. %-25s (Valeur hypothèque: %d M)\n", compteur_eligibles + 1, plateau[idx_plateau].nom, plateau[idx_plateau].hypotheque);
                if (compteur_eligibles < MAX_PROPRIETES)
                {
                    indices_eligibles[compteur_eligibles] = idx_plateau;
                }
                else // Vérifier si on ne dépasse pas la taille de indices_eligibles (sécurité)
                {
                    printf("Erreur: Trop de propriétés éligibles à stocker temporairement.\n");
                }
                compteur_eligibles++; 
            }
        } 

        if (compteur_eligibles == 0)
        {
            printf("Vous n'avez aucune propriété pouvant être hypothéquée actuellement (non-hypothéquée et sans constructions).\n");
        }
        else // --- Saisie du choix ---
        {
            saisieOk = 0;
            choix = -1; 
            while (!saisieOk)
            {
                printf("Entrez le numéro de la propriété à hypothéquer (1 à %d, ou 0 pour annuler) : ", compteur_eligibles);
                saisieOk = scanf("%d", &choix);
                if (saisieOk)
                {
                    viderBuffer(); 
                    if (choix < 0 || choix > compteur_eligibles)
                    {
                        printf("Choix invalide. Entrez un numéro entre 0 et %d.\n", compteur_eligibles);
                        saisieOk = 0; 
                    }
                }
                else
                {
                    printf("Entrée invalide. Veuillez entrer un nombre.\n");
                    viderBuffer(); 
                    saisieOk = 0;  
                }
            } 

            // --- Traitement du choix ---

            if (choix > 0) 
            {
                int indice_reel_a_hypothequer = indices_eligibles[choix - 1];

                if (plateau[indice_reel_a_hypothequer].estHypothequee)
                {
                    printf("Erreur : La propriété sélectionnée (%s) est déjà hypothéquée.\n", plateau[indice_reel_a_hypothequer].nom);
                }
                // Vérification si constructions présentes sur la prorpiété 
                else if (plateau[indice_reel_a_hypothequer].nbMaisons > 0 || plateau[indice_reel_a_hypothequer].Hotel > 0)
                {
                    printf("Action impossible : Vous devez d'abord vendre les constructions sur %s avant de l'hypothéquer.\n", plateau[indice_reel_a_hypothequer].nom);
                }
                else
                {
                    /* 4‑c. Vérifier le groupe de couleur ----------------- */
                    char couleurGroupe = plateau[indice_reel_a_hypothequer].couleur;
                    int  joueurIndex   = (int)(p - player);
                    int  constructionsDansGroupe = 0;

                    for (i = 0; i < NB_CASES; i++)
                    {
                        if (plateau[i].couleur == couleurGroupe &&plateau[i].proprietaire == joueurIndex   && (plateau[i].nbMaisons > 0 || plateau[i].Hotel > 0))
                        {
                            constructionsDansGroupe = 1;
                        }
                    }
                     /* 4‑d. Si d’autres terrains du groupe ont des bâtis -- */
                    if (constructionsDansGroupe)
                    {
                        printf("Impossible : un terrain du même groupe possède encore des maisons ou un hôtel. Vendez‑les d’abord!\n");
                    }
                    else
                    {
                        plateau[indice_reel_a_hypothequer].estHypothequee = 1;
                        (*p).argent += plateau[indice_reel_a_hypothequer].hypotheque;
                        printf("✅ La propriété '%s' a été hypothéquée. Vous recevez %d M.\n", plateau[indice_reel_a_hypothequer].nom, plateau[indice_reel_a_hypothequer].hypotheque);

                    }

                }
            }
            else
            {
                printf("Hypothèque annulée.\n");
            }
        }
    }
}


// --- Affichage du bilan détaillé d'un joueur ---

void afficherBilan(const struct joueur *p, struct CasePlateau plateau[], int nbCases)
{
    printf("\n=========================================\n");
    printf("            Bilan de %s\n", (*p).Nom);
    printf("=========================================\n");
    printf("Argent       : %d M\n", (*p).argent);
    printf("Position     : %d\n", (*p).position);
    printf("Propriétés   : %d\n", (*p).nbProprietes);
    if ((*p).enPrison)
    {
        printf("En prison    : Oui\n");
    }
    else
    {
        printf("En prison    : Non\n");
    }
    if ((*p).enPrison)
    {
        printf("Tour en prison : %d\n", (*p).tourEnPrison);
    }
    printf("-----------------------------------------\n\n");

    // Affichage détaillé des propriétés possédées
    if ((*p).nbProprietes == 0)
    {
        printf("Aucune propriété possédée.\n");
        printf("Cartes de sortie de prison possédées : %d\n", (*p).cartesSortiePrison);
    }
    else
    {
        printf("Liste des propriétés possédées :\n");
        printf("+----------------------+--------------+--------------+--------------+-----------------+-----------------+-------------------+-------------------+-------------------+-------------------+-----------------+\n");
        printf("| %-20s | %-12s | %-12s | %-12s  | %-15s | %-15s | %-17s | %-17s | %-17s | %-17s | %-15s  |\n",
               "Nom", "Prix d'achat", "Prix maison", "Hypothèque", "Construction", "Loyer de base",
               "Loyer 1 maison", "Loyer 2 maisons", "Loyer 3 maisons", "Loyer 4 maisons", "Loyer hôtel");
        printf("+----------------------+--------------+--------------+--------------+-----------------+-----------------+-------------------+-------------------+-------------------+-------------------+-----------------+\n");
        for (int i = 0; i < (*p).nbProprietes; i++)
        {
            int indice = (*p).proprietes[i];
            char construction[30];
            if (plateau[indice].Hotel)
            {
                strcpy(construction, "Hôtel ");
            }
            else if (plateau[indice].nbMaisons == 4)         
            {
                strcpy(construction, "4 maisons");
            }
            else if (plateau[indice].nbMaisons == 3)        
            {
                strcpy(construction, "3 maisons");
            }
            else if (plateau[indice].nbMaisons == 2)        
            {
                strcpy(construction, "2 maisons");
            }
            else if (plateau[indice].nbMaisons == 1)        
            {
                strcpy(construction, "1 maison");
            }
            else                                             
            {
                strcpy(construction, "Aucune");
            }
            printf("| %-20s | %-12d | %-12d | %-12d | %-15s | %-15d | %-17d | %-17d | %-17d | %-17d | %-15d |\n",
                   plateau[indice].nom_abrege,
                   plateau[indice].prix_achat,
                   plateau[indice].prix_maison,
                   plateau[indice].hypotheque,
                   construction,
                   plateau[indice].loyer,
                   plateau[indice].loyer1m,
                   plateau[indice].loyer2m,
                   plateau[indice].loyer3m,
                   plateau[indice].loyer4m,
                   plateau[indice].loyerh);
        }
        printf("+----------------------+--------------+--------------+--------------+-----------------+-----------------+-------------------+-------------------+-------------------+-------------------+-----------------+\n");
        printf("Cartes de sortie de prison possédées : %d\n", (*p).cartesSortiePrison);
    }
}



// --- Applique l’effet complet de la case sur laquelle le joueur vient d’atterrir ---

void appliquer_effet_case(struct CasePlateau *caseCourante, struct joueur *joueurActuel, struct joueur player[], int d1, int d2)
{
    char response[10];
    int plateauIndex = (int)(caseCourante - plateau);     // Index de la case actuelle
    int joueurActuelIndex = (int)(joueurActuel - player); // Index du joueur actuel
    struct Carte cartePiochee;
    int loyer = 0;
    int c;        
    int saisieOk; 

    switch ((*caseCourante).Type)
    {
        case 0: // Arrivée sur une propriété
        {
            printf("\n📍 %s arrive sur %s. Veuillez appuyer sur Enter pour continuer... ", (*joueurActuel).Nom, (*caseCourante).nom);
            viderBuffer();

            // --- Cas 1: Personne ne possède la case ---
            if ((*caseCourante).proprietaire == -1)
            {
                printf("Cette propriété n'appartient à personne.\n");
                printf("Prix d'achat : %d M. Argent disponible : %d M.\n", (*caseCourante).prix_achat, (*joueurActuel).argent);

                saisieOk = 0;

                while (!saisieOk)
                {
                    printf("Voulez-vous l'acheter ? (O/N) : ");

                    if (scanf("%9s", response) == 1)
                    {                  
                        viderBuffer();

                        conv_maj(response);
                        if (response[0] == 'O' || response[0] == 'N')
                        {
                            saisieOk = 1;
                        }
                        else
                        {
                            printf("Réponse invalide. Entrez 'O' ou 'N'.\n");
                        }
                    }
                    else
                    {
                        printf("Erreur de saisie. Entrez 'O' ou 'N'.\n");
                        viderBuffer(); 
                    }
                } 

                if (response[0] == 'O')
                {
                    if ((*joueurActuel).argent < (*caseCourante).prix_achat)
                    {
                        printf("❌ Fonds insuffisants pour acheter! \n");
                        printf("La propriété %s va être mise aux enchères.\n", (*caseCourante).nom);
                        enchere(&plateau[plateauIndex], plateauIndex, player, nbjoueurs);
                    }
                    else
                    {
                        // Achat de la prorpiété
                        (*joueurActuel).argent -= (*caseCourante).prix_achat;
                        (*caseCourante).proprietaire = joueurActuelIndex;                        
                        (*joueurActuel).proprietes[(*joueurActuel).nbProprietes] = plateauIndex; // Ajout de la propriété à la liste du joueur
                        (*joueurActuel).nbProprietes++;
                        printf("✅ %s a acheté %s !\n", (*joueurActuel).Nom, (*caseCourante).nom);
                    }
                }
                else
                { 
                    printf("%s n'achète pas %s.\n", (*joueurActuel).Nom, (*caseCourante).nom);
                    printf("La propriété va être mise aux enchères.\n");
                    enchere(&plateau[plateauIndex], plateauIndex, player, nbjoueurs);
                }
            }
            // --- Cas 2: La case a déjà un propriétaire ---
            else
            {
                int proprioIndex = (*caseCourante).proprietaire;

                // 2a: C'est le joueur lui-même
                if (proprioIndex == joueurActuelIndex)
                {
                    printf("Vous êtes sur votre propre propriété : %s.\n", (*caseCourante).nom);
                }
                // 2b: Appartient à quelqu'un d'autre
                else
                {

                    if ((*caseCourante).estHypothequee)
                    {
                        printf("La propriété %s est hypothéquée par %s. Pas de loyer.\n", (*caseCourante).nom, player[proprioIndex].Nom);
                    }
                    // Si non hypothéquée et propriétaire en jeu il faut calculer le loyer
                    else
                    {
                        loyer = 0; 
                        char couleur = (*caseCourante).couleur;

                        // Calcul pour Gares ('G')
                        if (couleur == 'G')
                        {
                            int nbGares = calcul_gares(plateau, proprioIndex);
                            if (nbGares > 0)
                            {
                                loyer = 25;
                                for (int k = 1; k < nbGares; k++)
                                {
                                    loyer *= 2;
                                }
                            }
                        }
                        // Calcul pour Services Publics ('P')
                        else if (couleur == 'P')
                        {
                            int nbServices = 0;
                            if (plateau[12].proprietaire == proprioIndex)
                            {
                                nbServices++;
                            }
                            if (plateau[28].proprietaire == proprioIndex)
                            {
                                nbServices++;
                            }

                            if (nbServices == 1)
                            {
                                loyer = (d1 + d2) * 4; 
                            }
                            else if (nbServices == 2)
                            {
                                loyer = (d1 + d2) * 10; 
                            }
                        }

                        // Calcul pour Terrains Normaux (avec constructions ou monopole)

                        else
                        {
                            if ((*caseCourante).Hotel > 0)
                            {
                                loyer = (*caseCourante).loyerh;
                            }
                            else if ((*caseCourante).nbMaisons > 0)
                            {
                                switch ((*caseCourante).nbMaisons)
                                {
                                case 1:
                                    loyer = (*caseCourante).loyer1m;
                                    break;
                                case 2:
                                    loyer = (*caseCourante).loyer2m;
                                    break;
                                case 3:
                                    loyer = (*caseCourante).loyer3m;
                                    break;
                                case 4:
                                    loyer = (*caseCourante).loyer4m;
                                    break;
                                default:
                                    loyer = (*caseCourante).loyer;
                                }
                            }
                            else // Si la propriété n'a pas de constructions
                            { 
                                if (tous_couleurs(plateau, NB_CASES, couleur, proprioIndex))
                                {
                                    loyer = (*caseCourante).loyer * 2; // Double loyer si monopole du groupe
                                }
                                else
                                {
                                    loyer = (*caseCourante).loyer; // Loyer de base
                                }
                            }
                        } 

                        if (loyer > 0)
                        {
                            printf("💰 %s doit payer %d M de loyer à %s.\n", (*joueurActuel).Nom, loyer, player[proprioIndex].Nom);
                            (*joueurActuel).argent -= loyer;
                            player[proprioIndex].argent += loyer;

                            // Vérification de la faillite
                        
                            if ((*joueurActuel).argent < 0 && (*joueurActuel).enJeu)
                            {
                                printf("!! Faillite potentielle en payant le loyer !!\n");
                                gererFaillite(joueurActuel, player, nbjoueurs, plateau, proprioIndex);
                            }
                        }
                        else
                        {
                            printf("Aucun loyer à payer pour %s.\n", (*caseCourante).nom);
                        }
                    } 
                } 
            } 
        } 
        break; 

        case 1: // carte chance
            printf("\n📍 %s arrive sur une case chance\n", (*joueurActuel).Nom);
            printf("Appuyez sur Entrée pour piocher une carte Chance...");
            viderBuffer(); 
            cartePiochee = retirerCarteChance();
            if (cartePiochee.effet != -1)
            { 
                appliquer_effet_carte(cartePiochee, joueurActuel, player, nbjoueurs);
                // Remise de la carte (sauf effet 2 = sortie de prison)
                if (cartePiochee.effet != 2)
                {
                    if (nbCartesChance < MAX_CARTES)
                    {
                        cartes_chance[nbCartesChance] = cartePiochee;
                        nbCartesChance++;
                    }
                }
                else
                {
                    printf("Carte Sortie Prison gardée.\n");
                    (*joueurActuel).libereChance = 1;
                } 
            }
            break;

        case 2: // caisse de communauté
            printf("\n📍 %s arrive sur la caisse de communauté\n", (*joueurActuel).Nom);
            printf("Appuyez sur Entrée pour piocher une carte Communauté...");
            viderBuffer(); 
            cartePiochee = retirerCarteCommunaute();
            if (cartePiochee.effet != -1)
            { 
                appliquer_effet_carte(cartePiochee, joueurActuel, player, nbjoueurs);
                // Remise de la carte (sauf effet 2 = sortie de prison)
                if (cartePiochee.effet != 2)
                {
                    if (nbCartesCommunaute < MAX_CARTES)
                    {
                        cartes_communaute[nbCartesCommunaute] = cartePiochee;
                        nbCartesCommunaute++;
                    }

                } //
                else
                {
                    printf("Carte Sortie Prison gardée.\n");
                    (*joueurActuel).libereCommunaute = 1;
                }
            }
            break;

        case 3: // Taxes
            printf("\n📍 %s arrive sur %s\n", (*joueurActuel).Nom, (*caseCourante).nom);
            printf("Vous devez payer %d M !\n", (*caseCourante).loyer);
            (*joueurActuel).argent -= (*caseCourante).loyer; 
            if ((*joueurActuel).argent < 0 && (*joueurActuel).enJeu)
            {
                printf("!! Faillite potentielle en payant la taxe !!\n");
                gererFaillite(joueurActuel, player, nbjoueurs, plateau, -1); // Créancier = banque
            }
            break;

        case 4: // Allez en Prison
            printf("\n📍 %s arrive sur %s\n", (*joueurActuel).Nom, (*caseCourante).nom);
            printf("Direction immédiate vers la prison...\n");
            printf("Ne passez pas par la case Départ, ne recevez pas M200\n");
            printf("⛓️ Vous resterez en prison jusqu'à ce que :\n");
            printf("🎲 - Vous fassiez un double\n");
            printf("🃏 - Vous utilisiez une carte 'Sortie de prison'\n");
            printf("💰 - Vous payiez une amende\n");
            printf("⏳ - Ou AUTOMATIQUEMENT après **3 tours**, avec paiement de l'amende si nécessaire\n\n");
            (*joueurActuel).position = 10;
            (*joueurActuel).enPrison = 1;
            (*joueurActuel).tourEnPrison = 0;
            break;

        case 5: // Case Départ 
            printf("\n📍 %s arrive sur %s\n", (*joueurActuel).Nom, (*caseCourante).nom);
            afficherLogoMonopoly(); 
            break;
        case 6: // Parc Gratuit
            printf("\n📍 %s arrive sur %s\n", (*joueurActuel).Nom, (*caseCourante).nom);
            printf("Profitez d'un moment de détente, vous êtes en sécurité ici.\n");
            printf("Aucun loyer ni taxe à payer. Reprenez votre souffle avant le prochain tour !\n");
            break;
        case 7: // Simple Visite Prison
            printf("\n📍 %s est de simple visite en prison\n", (*joueurActuel).Nom);
            break;

        default:
            printf("Type de case inconnu: %d\n", (*caseCourante).Type);
            break;

    } 
}

// --- Retire la première carte Chance du paquet et recale le tableau ---
struct Carte retirerCarteChance()
{
    // Récupérer la carte du sommet
    struct Carte cartePiochee = cartes_chance[0];

    // Décaler toutes les cartes d'une position vers la gauche
    for (int i = 0; i < nbCartesChance - 1; i++)
    {
        cartes_chance[i] = cartes_chance[i + 1];
    }
    nbCartesChance--; 

    return cartePiochee;
}


// --- Retire la première carte Communauté du paquet et recale le tableau ---
struct Carte retirerCarteCommunaute()
{
    struct Carte cartePiochee = cartes_communaute[0];

    for (int i = 0; i < nbCartesCommunaute - 1; i++)
    {
        cartes_communaute[i] = cartes_communaute[i + 1];
    }
    nbCartesCommunaute--; 

    return cartePiochee;
}

// --- Propose au joueur actif de choisir un adversaire encore en jeu ; si un choix valide est fait, le sous-menu de négociation est ouvert ---
void negocier(struct joueur *p, struct joueur players[], int nbJoueurs, struct CasePlateau plateau[], int nbCases)
{
    int cibleIndex = -1; 
    int i;
    char c;
    int saisieOk = 0;
    int flag = 0;            
    int nbOptionsValides = 0; 

    printf("\n--- Négociation ---");
    printf("\nListe des joueurs avec qui négocier :\n");

    for (i = 0; i < nbJoueurs; i++)
    {
        if (&players[i] != p && players[i].enJeu)
        {
            printf("  %d. %s (Argent : %d M)\n", i + 1, players[i].Nom, players[i].argent);
            nbOptionsValides++;
        }
    }

    if (nbOptionsValides == 0)
    {
        printf("Il n'y a aucun autre joueur en jeu avec qui négocier.\n");
    }

    else
    {

        saisieOk = 0; 
        flag = 0;     
        while (!saisieOk)
        {
            printf("Entrez le numéro du joueur (1 à %d, ou 0 pour annuler) : ", nbJoueurs); 

            if (scanf("%d", &cibleIndex) == 1) 
            {
                viderBuffer(); 

                if (cibleIndex == 0)
                {
                    printf("Négociation annulée.\n");
                    flag = 0;     
                    saisieOk = 1; 
                }
                // Traitement d'un choix de joueur potentiel 
                else
                {
                    int cibleIndex_reel = cibleIndex - 1; 

                    if (cibleIndex_reel >= 0 && cibleIndex_reel < nbJoueurs && &players[cibleIndex_reel] != p && players[cibleIndex_reel].enJeu)
                    {
                        saisieOk = 1;                 
                        flag = 1;                    
                        cibleIndex = cibleIndex_reel; 
                    }
                    else
                    {
                        printf("Choix de joueur invalide ou non éligible. Veuillez réessayer.\n");
                        saisieOk = 0; 
                        flag = 0;     
                    }
                }
            }
            else 
            {
                printf("Entrée invalide. Veuillez entrer un nombre.\n");
                viderBuffer(); 
                saisieOk = 0;  
                flag = 0;      
            }
        } 

        if (flag == 1)
        {
            printf("Vous avez choisi de négocier avec %s.\n", players[cibleIndex].Nom);
            menu_negociation(p, &players[cibleIndex], plateau, nbCases);
        }
    }
}



// --- Ouvre un dialogue interactif entre deux joueurs pour acheter, échanger ou négocier des propriétés ou cartes « Sortie de prison » ---

void menu_negociation(struct joueur *negociant, struct joueur *cible, struct CasePlateau plateau[], int nbCases)
{

    int choix = -1;
    int nbOffertes = 0;
    int nbDemandees = 0;
    char reponse[10];
    int proprieteIndex = 0;
    int saisieOk = 0;

    while (choix != 0)
    {
        // --- Menu de négociation ---
        printf("\n===== Menu de négociation =====\n");
        printf("Vous négociez avec %s.\n", (*cible).Nom);
        printf("Que souhaitez-vous proposer ?\n");
        printf("0. Annuler la négociation et retourner au jeu\n");
        printf("1. Achat direct (offrir argent pour propriété)\n");
        printf("2. Échange de propriété\n");
        printf("3. Achat avec échange (offrir argent + propriété pour propriété)\n");
        printf("4. Acheter/Vendre carte 'Libéré de prison'\n");

        saisieOk = 0;
        choix = -1; 

        while (!saisieOk)
        {
            printf("Votre choix (0-4) : ");
            saisieOk = scanf("%d", &choix);
            viderBuffer(); 
            if (saisieOk)
            {
                if (choix >= 0 && choix <= 4)
                {
                    saisieOk = 1; 
                }
                else
                {
                    printf("Choix invalide. Entrez un nombre entre 0 et 4.\n");
                    saisieOk = 0;
                }
            }
            else
            {
                printf("Entrée invalide. Veuillez entrer un nombre.\n");
                saisieOk = 0;
            }
        }

        // --- TRAITEMENT ---

        switch (choix)
        {
        case 0:
            printf("Négociation annulée.\n");
            break;
        case 1: // Option 1 : Achat direct
            if ((*cible).nbProprietes == 0)
            {
                printf("Le joueur choisi n'a pas de propriétés à vendre ! \n");
            }
            else
            {

                printf("\nVoici les propriétés de %s :\n", (*cible).Nom);
                for (int i = 0; i < (*cible).nbProprietes; i++)
                {
                    int idx = (*cible).proprietes[i];
                    printf("%d. %s (Prix d'achat : M %d)\n", i, plateau[idx].nom, plateau[idx].prix_achat);
                }
                saisieOk = 0;

                while (!saisieOk)
                {
                    printf("Entrez le numéro de la propriété que vous souhaitez acheter : ");
                    saisieOk = scanf("%d", &proprieteIndex);
                    viderBuffer();

                    if (!saisieOk)
                    {
                        printf("Veuillez entrer une saisie valide\n");
                    }
                }

                if (proprieteIndex < 0 || proprieteIndex >= (*cible).nbProprietes)
                {
                    printf("Choix de propriété invalide.\n");
                }
                else
                {
                    int idxProp = (*cible).proprietes[proprieteIndex];

                    if (!propriete_est_transférable(idxProp, plateau, nbCases)) 
                    {
                        printf("La propriété %s ne peut pas être vendue car elle ou une autre propriété du groupe possède une construction.\n", plateau[idxProp].nom);
                    }
                    else
                    {
                        int offre;
                        saisieOk = 0;
                        while (!saisieOk)
                        {
                            printf("Entrez votre offre (en M) pour la propriété %s : ", plateau[idxProp].nom);
                            if (scanf("%d", &offre) == 1 && offre > 0)
                            {
                                saisieOk = 1;
                            }
                            else
                            {
                                printf("Veuillez entrer une saisie valide (entier > 0).\n");
                            }
                            viderBuffer();
                        }

                        if ((*negociant).argent < offre)
                        {
                            printf("Vous n'avez pas assez d'argent pour cette offre.\n");
                        }

                        else
                        {
                            reponse[0] = ' ';

                            while (reponse[0] != 'O' && reponse[0] != 'N')
                            {

                                printf("C'est le moment de la décision, %s! Souhaitez-vous accepter l'offre proposée? (O/N) ", (*cible).Nom);
                                scanf("%s", reponse);
                                viderBuffer();
                                conv_maj(reponse);
                                if (reponse[0] == 'O')
                                {
                                    // Transfert d'argent et de propriété
                                    (*negociant).argent -= offre;
                                    (*cible).argent += offre;
                                    (*negociant).proprietes[(*negociant).nbProprietes] = idxProp;
                                    (*negociant).nbProprietes++;

                                    // Retirer la propriété du joueur cible 
                                    for (int i = proprieteIndex; i < (*cible).nbProprietes - 1; i++)
                                    {
                                        (*cible).proprietes[i] = (*cible).proprietes[i + 1];
                                    }
                                    (*cible).nbProprietes--;

                                    // Mettre à jour le propriétaire dans le plateau
                                    plateau[idxProp].proprietaire = negociant - player; 

                                    printf("Transaction réussie ! Vous avez acheté %s pour %d M.\n", plateau[idxProp].nom, offre);
                                }
                                else if (reponse[0] == 'N')
                                {
                                    printf("%s a refusé votre offre.\n", (*cible).Nom);
                                }
                                else
                                {
                                    printf("Saisie non valide, veuillez entrer O(Oui) ou N(Non)\n");
                                }
                            }
                        }

                    }
                    
                }
            }
            break;

        case 2:
            if ((*negociant).nbProprietes == 0)
            {
                printf("Vous ne possédez aucune propriété à offrir pour un échange.\n");
            }
            else if ((*cible).nbProprietes == 0)
            {
                printf("%s ne possède aucune propriété à échanger.\n", (*cible).Nom);
            }
            else
            {
                int nbOffertes = 0, nbDemandees = 0, saisieOk = 0, i;
                int indicesReelsNegociant[MAX_PROPRIETES];
                int indicesReelsCible[MAX_PROPRIETES];

                // Nombre de propriétés offertes 
                while (saisieOk == 0)
                {
                    printf("\nCombien de propriétés souhaitez-vous offrir (1-%d) ? ", (*negociant).nbProprietes);
                    if (scanf("%d", &nbOffertes) == 1)
                    {
                        viderBuffer();
                        if (nbOffertes > 0 && nbOffertes <= (*negociant).nbProprietes)
                        {
                            saisieOk = 1;
                        }
                        else
                            printf("Nombre invalide. Doit être entre 1 et %d.\n", (*negociant).nbProprietes);
                    }
                    else
                    {
                        printf("Saisie invalide. Veuillez entrer un nombre.\n");
                        viderBuffer();
                    }
                }
                saisieOk = 0;

                // Nombre de propriétés demandées 
                while (saisieOk == 0)
                {
                    printf("Combien de propriétés souhaitez-vous obtenir (1-%d) ? ",(*cible).nbProprietes);
                    if (scanf("%d", &nbDemandees) == 1)
                    {
                        viderBuffer();

                        if (nbDemandees > 0 && nbDemandees <= (*cible).nbProprietes)
                        {
                            saisieOk = 1;
                        }
                        else
                        {
                            printf("Nombre invalide. Doit être entre 1 et %d.\n",(*cible).nbProprietes);
                        }
                    }
                    else
                    {
                        printf("Saisie invalide. Veuillez entrer un nombre.\n");
                        viderBuffer();
                    }
                }

                // Filtrer les propriétés sans constructions du négociant
                int listeOffrablesNeg[MAX_PROPRIETES], nbListeOffrablesNeg = 0;
                for (i = 0; i < (*negociant).nbProprietes; i++)
                {
                    int idx = (*negociant).proprietes[i];
                    if (propriete_est_transférable(idx, plateau, nbCases))
                    {
                        listeOffrablesNeg[nbListeOffrablesNeg++] = i;
                    }
                }
                if (nbListeOffrablesNeg == 0)
                {
                    printf("Vous n'avez aucune propriété sans constructions à offrir.\n");
                }
                else
                {
                    printf("\n--- Vos propriétés sans constructions ---\n");
                    for (i = 0; i < nbListeOffrablesNeg; i++)
                    {
                        int idx = (*negociant).proprietes[listeOffrablesNeg[i]];
                        printf("  %d. %s\n", i, plateau[idx].nom);
                    }
                    // Sélection des offres 
                    for (i = 0; i < nbOffertes; i++)
                    {
                        int choix = -1;
                        saisieOk = 0;
                        while (saisieOk == 0)
                        {
                            printf("Entrez le numéro de la propriété n°%d à offrir (0-%d) : ", i + 1, nbListeOffrablesNeg - 1);
                            if (scanf("%d", &choix) == 1)
                            {
                                viderBuffer();
                                if (choix >= 0 && choix < nbListeOffrablesNeg)
                                {
                                    saisieOk = 1;

                                }
                                else
                                {
                                    printf("Numéro invalide. Doit être entre 0 et %d.\n", nbListeOffrablesNeg - 1);
                                }
                                   
                            }
                            else
                            {
                                printf("Saisie invalide. Veuillez entrer un nombre.\n");
                                viderBuffer();
                            }
                        }

                        indicesReelsNegociant[i] = (*negociant).proprietes[listeOffrablesNeg[choix]];
                    }

                    // Filtrer les propriétés sans constructions de la cible 
                    int listeOffrablesCib[MAX_PROPRIETES], nbListeOffrablesCib = 0;
                    for (i = 0; i < (*cible).nbProprietes; i++)
                    {
                        int idx = (*cible).proprietes[i];
                        if (propriete_est_transférable(idx, plateau, nbCases))
                        {
                            listeOffrablesCib[nbListeOffrablesCib++] = i;
                        }
                    }
                    if (nbListeOffrablesCib == 0)
                    {
                        printf("%s n'a aucune propriété sans constructions à proposer.\n", (*cible).Nom);
                    }
                    else
                    {
                        printf("\n--- Propriétés de %s sans constructions ---\n", (*cible).Nom);
                        for (i = 0; i < nbListeOffrablesCib; i++)
                        {
                            int idx = (*cible).proprietes[listeOffrablesCib[i]];
                            printf("  %d. %s\n", i, plateau[idx].nom);
                        }
                        // Sélection des demandes
                        for (i = 0; i < nbDemandees; i++)
                        {
                            int choix = -1;
                            saisieOk = 0;
                            while (saisieOk == 0)
                            {
                                printf("Entrez le numéro de la propriété n°%d à obtenir (0-%d) : ", i + 1, nbListeOffrablesCib - 1);
                                if (scanf("%d", &choix) == 1)
                                {
                                    viderBuffer();

                                    if (choix >= 0 && choix < nbListeOffrablesCib)
                                    {
                                        saisieOk = 1;
                                    }
                                    else
                                    {
                                        printf("Numéro invalide. Doit être entre 0 et %d.\n", nbListeOffrablesCib - 1);
                                    }
                                }
                                else
                                {
                                    printf("Saisie invalide. Veuillez entrer un nombre.\n");
                                    viderBuffer();
                                }
                            }
                            indicesReelsCible[i] = (*cible).proprietes[listeOffrablesCib[choix]];
                        }

                        // Récapitulatif 

                        printf("\nRécapitulatif de l'échange proposé :\n");
                        int maxLignes;
                        if(nbOffertes > nbDemandees)
                        {
                            maxLignes = nbOffertes;
                        }
                        else
                        {
                            maxLignes = nbDemandees;
                        }
                        for (i = 0; i < maxLignes; i++)
                        {
                            char nomOff[100] = "(rien)";
                            char nomCib[100] = "(rien)";
                            if (i < nbOffertes)
                            {
                                strcpy(nomOff, plateau[indicesReelsNegociant[i]].nom);
                            }
                            if (i < nbDemandees)
                            {
                                strcpy(nomCib, plateau[indicesReelsCible[i]].nom);
                            }
                            printf("  %d. Votre '%s' contre '%s' de %s\n", i + 1, nomOff, nomCib, (*cible).Nom);
                        }

                        char reponse[10] = " ";
                        while (reponse[0] != 'O' && reponse[0] != 'N')
                        {
                            printf("%s, acceptez-vous cet échange global ? (O/N) : ",(*cible).Nom);
                            if (scanf("%9s", reponse) == 1)
                            {
                                viderBuffer();
                                conv_maj(reponse);
                                if (reponse[0] != 'O' && reponse[0] != 'N')
                                {
                                    printf("Saisie non valide, veuillez entrer O ou N.\n");
                                }
                            }
                            else
                            {
                                printf("Erreur de saisie. Veuillez entrer O ou N.\n");
                                viderBuffer();
                            }
                        }

                        // Exécution de l'échange si accepté
                        if (reponse[0] == 'O')
                        {
                            int nSwap;
                            if (nbOffertes < nbDemandees)
                            {
                                nSwap = nbOffertes;
                            }
                            else
                            {
                                nSwap = nbDemandees;
                            }
                            int j, k;
                            // Échange 1-à-1 
                            for (i = 0; i < nSwap; i++)
                            {
                                int idxNeg = indicesReelsNegociant[i];
                                int idxCib = indicesReelsCible[i];
                                plateau[idxCib].proprietaire = (int)(negociant - player);
                                plateau[idxNeg].proprietaire = (int)(cible - player);
                                int trouve = 0;
                                for (j = 0; j < (*negociant).nbProprietes && !trouve; j++)
                                {
                                    if ((*negociant).proprietes[j] == idxNeg)
                                    {
                                        (*negociant).proprietes[j] = idxCib;
                                        trouve = 1;
                                    }

                                }
                                trouve = 0;
                                for (j = 0; j < (*cible).nbProprietes && !trouve; j++)
                                {
                                    if ((*cible).proprietes[j] == idxCib)
                                    {
                                        (*cible).proprietes[j] = idxNeg;
                                        trouve = 1;
                                    }

                                }
                                  
                            }
                            // Surplus du négociant vers la cible 
                            for (; i < nbOffertes; i++)
                            {
                                int idxOff = indicesReelsNegociant[i];
                                int pos = -1;
                                for (j = 0; j < (*negociant).nbProprietes; j++)
                                {
                                    if ((*negociant).proprietes[j] == idxOff)
                                    {
                                        pos = j;
                                    }

                                }  
                                for (k = pos; k < (*negociant).nbProprietes - 1; k++)
                                {
                                    (*negociant).proprietes[k] = (*negociant).proprietes[k + 1];
                                }
                                (*negociant).nbProprietes--;
                                (*cible).proprietes[(*cible).nbProprietes++] = idxOff;
                                plateau[idxOff].proprietaire = (int)(cible - player);
                            }
                            // Surplus de la cible vers le négociant
                            for (i = nSwap; i < nbDemandees; i++)
                            {
                                int idxDem = indicesReelsCible[i];
                                int pos = -1;
                                for (j = 0; j < (*cible).nbProprietes; j++)
                                {
                                    if ((*cible).proprietes[j] == idxDem)
                                    {
                                        pos = j;
                                        
                                    }

                                }
                                if(pos != -1)
                                {
                                    for (k = pos; k < (*cible).nbProprietes - 1; k++)
                                    {
                                        (*cible).proprietes[k] = (*cible).proprietes[k + 1];
                                    }
                                    (*cible).nbProprietes--;
                                    (*negociant).proprietes[(*negociant).nbProprietes] = idxDem;
                                    (*negociant).nbProprietes++;
                                    plateau[idxDem].proprietaire = (int)(negociant - player);
                                }

                            }
                                
                            printf("Échange réussi !\n");
                        }
                        else
                        {
                            printf("L'échange a été refusé par %s.\n", (*cible).Nom);
                        }
                    }
                }
                break;
        case 3: // Achat avec échange : le négociant offre une propriété et une somme d'argent pour obtenir une propriété du joueur cible.
            {
                if ((*negociant).nbProprietes == 0)
                {
                    printf("Vous ne possédez aucune propriété à offrir pour un échange combiné.\n");
                }
                else if ((*cible).nbProprietes == 0)
                {
                    printf("%s ne possède aucune propriété à échanger.\n", (*cible).Nom);
                }
                else
                {
                    int proprieteOfferte = -1;
                    int proprieteDemande = -1;
                    int offreSomme;

                    // Afficher les propriétés du négociant
                    printf("\nVos propriétés :\n");
                    for (int i = 0; i < (*negociant).nbProprietes; i++)
                    {
                        int idx = (*negociant).proprietes[i];
                        printf("%d. %s (Prix d'achat : M %d)\n", i, plateau[idx].nom, plateau[idx].prix_achat);
                    }
                    saisieOk = 0; 
                    while (proprieteOfferte < 0 || proprieteOfferte >= (*negociant).nbProprietes)
                    {
                        printf("Entrez le numéro de la propriété que vous souhaitez offrir : ");
                        scanf("%d", &proprieteOfferte);
                        viderBuffer();
                        if (proprieteOfferte < 0 || proprieteOfferte >= (*negociant).nbProprietes)
                        {
                            printf("Choix invalide.\n");
                        }
                    }

                    // Afficher les propriétés du joueur cible
                    printf("\nPropriétés de %s :\n", (*cible).Nom);
                    for (int i = 0; i < (*cible).nbProprietes; i++)
                    {
                        int idx = (*cible).proprietes[i];
                        printf("%d. %s (Prix d'achat : M %d)\n", i, plateau[idx].nom, plateau[idx].prix_achat);
                    }

                    while (proprieteDemande < 0 || proprieteDemande >= (*cible).nbProprietes)
                    {
                        printf("Entrez le numéro de la propriété que vous souhaitez obtenir : ");
                        scanf("%d", &proprieteDemande);
                        viderBuffer();
                        if (proprieteDemande < 0 || proprieteDemande >= (*cible).nbProprietes)
                        {
                            printf("Choix invalide.\n");
                        }
                    }

                    // Demander la somme d'argent que le négociant souhaite offrir en complément
                    printf("Entrez le montant en M que vous offrez en plus de votre propriété : ");
                    scanf("%d", &offreSomme);
                    while (offreSomme <= 0)
                    {
                        printf("Montant invalide. Entrez un montant positif : ");
                        scanf("%d", &offreSomme);
                        viderBuffer();
                    }


                    if ((*negociant).argent < offreSomme)
                    {
                        printf("Fonds insuffisants pour offrir cette somme.\n");
                    }
                    else
                    {
                        // Récapitulatif de l'offre
                        int idxOfferte = (*negociant).proprietes[proprieteOfferte];
                        int idxDemande = (*cible).proprietes[proprieteDemande];
                        if (!propriete_est_transférable(idxOfferte, plateau, nbCases)) 
                        {
                            printf("Votre propriété '%s' ne peut pas être échangée car elle ou une autre du groupe a une construction.\n", plateau[idxOfferte].nom);

                        }
                        else if (!propriete_est_transférable(idxDemande, plateau, nbCases)) {
                            printf("La propriété '%s' ne peut pas être obtenue car elle ou une autre du groupe a une construction.\n", plateau[idxDemande].nom);
                        }
                        else
                        {
                            printf("\nRécapitulatif de l'offre:\n");
                            printf("Vous offrez la propriété '%s' et %d M en échange de la propriété '%s' de %s.\n", plateau[idxOfferte].nom, offreSomme, plateau[idxDemande].nom, (*cible).Nom);

                            reponse[0] = ' ';

                            while (reponse[0] != 'O' && reponse[0] != 'N')
                            {
                                printf("%s, acceptez-vous cette offre ? (O/N) : ", (*cible).Nom);
                                scanf("%s", reponse);
                                conv_maj(reponse);
                                viderBuffer();

                                if (reponse[0] == 'O')
                                {
                                    // Transfert d'argent et échange des propriétés
                                    (*negociant).argent -= offreSomme;
                                    (*cible).argent += offreSomme;

                                    int temp = (*negociant).proprietes[proprieteOfferte];
                                    (*negociant).proprietes[proprieteOfferte] = (*cible).proprietes[proprieteDemande];
                                    (*cible).proprietes[proprieteDemande] = temp;

                                    // Mise à jour du propriétaire dans le plateau
                                    plateau[(*negociant).proprietes[proprieteOfferte]].proprietaire = (int)(negociant - player);
                                    plateau[(*cible).proprietes[proprieteDemande]].proprietaire = (int)(cible - player);

                                    printf("Transaction réussie ! L'échange a été effectué.\n");
                                }
                                else if (reponse[0] == 'N')
                                {
                                    printf("L'offre a été refusée par %s.\n", (*cible).Nom);
                                }
                                else
                                {
                                    printf("Saisie non valide, veuillez entrer O(Oui) ou N(Non)\n");
                                }
                            }

                        }
                        
                    }
                }

                break;
            }
        case 4: // Négociation carte sortie de prison
            {
                int sousChoix;
                printf("\n=== Négociation pour la carte 'Libéré de prison' ===\n");
                printf("Voulez-vous :\n");
                printf("1. Acheter une carte 'Libéré de prison'\n");
                printf("2. Vendre une carte 'Libéré de prison'\n");
                printf("3. Annuler\n");
                printf("Votre choix : ");
                scanf("%d", &sousChoix);
                viderBuffer(); 

                switch (sousChoix)
                {
                case 1: // Acheter une carte
                    if (((*cible).cartesSortiePrison) <= 0)
                    {
                        printf("%s ne possède pas de carte 'Libéré de prison'.\n", (*cible).Nom);
                    }
                    else
                    {
                        int offre;
                        printf("Entrez votre offre (en M) pour acheter la carte 'Libéré de prison' : ");
                        scanf("%d", &offre);
                        viderBuffer();
                        if (((*negociant).argent) < offre)
                        {
                            printf("Fonds insuffisants pour cette offre.\n");
                        }
                        else
                        {
                            reponse[0] = ' ';
                            while (reponse[0] != 'O' && reponse[0] != 'N')
                            {
                                printf("%s, acceptez-vous de vendre votre carte 'Libéré de prison' pour %d M ? (O/N) : ", (*cible).Nom, offre);
                                scanf("%s", reponse);
                                conv_maj(reponse);
                                viderBuffer();
                                if (reponse[0] == 'O')
                                {
                                    (*negociant).argent -= offre;
                                    (*cible).argent += offre;
                                    (*cible).cartesSortiePrison--;
                                    (*negociant).cartesSortiePrison++;
                                    if ((*cible).libereChance)
                                    {
                                        (*negociant).libereChance = 1;
                                        (*cible).libereChance = 0;
                                    }
                                    else
                                    {
                                        (*negociant).libereCommunaute = 1;
                                        (*cible).libereCommunaute = 0;
                                    }

                                    printf("Transaction réussie ! %s a acheté la carte 'Libéré de prison' à %s pour %d M.\n", (*negociant).Nom, (*cible).Nom, offre);
                                }
                                else if (reponse[0] == 'N')
                                {
                                    printf("Transaction annulée par %s.\n", (*cible).Nom);
                                }
                                else
                                {
                                    printf("Saisie non valide, veuillez entrer O(Oui) ou N(Non)\n");
                                }
                            }
                        }
                    }
                    break;
                    case 2: // Vendre une carte
                        if (((*negociant).cartesSortiePrison) <= 0)
                        {
                            printf("Vous ne possédez pas de carte 'Libéré de prison' à vendre.\n");
                        }
                        else
                        {
                            int demande;
                            int typeCarte = 0;
                    
                            if ((*negociant).libereChance && (*negociant).libereCommunaute)
                            {
                                while (typeCarte != 1 && typeCarte != 2)
                                {
                                    printf("Quelle carte souhaitez-vous vendre ?\n");
                                    printf("1. Carte Chance\n");
                                    printf("2. Carte Communauté\n");
                                    printf("Votre choix : ");
                                    scanf("%d", &typeCarte);
                                    viderBuffer();
                                }
                            }
                            else if ((*negociant).libereChance)
                            {
                                typeCarte = 1;
                            }
                            else if ((*negociant).libereCommunaute)
                            {
                                typeCarte = 2;
                            }
                    
                            printf("Entrez le montant demandé (en M) pour vendre votre carte 'Libéré de prison' : ");
                            scanf("%d", &demande);
                            viderBuffer();
                            if (((*cible).argent) < demande)
                            {
                                printf("%s n'a pas assez d'argent pour acheter votre carte.\n", (*cible).Nom);
                            }
                            else
                            {
                                reponse[0] = ' ';
                                while (reponse[0] != 'O' && reponse[0] != 'N')
                                {
                                    printf("%s, acceptez-vous d'acheter la carte 'Libéré de prison' pour %d M ? (O/N) : ", (*cible).Nom, demande);
                                    scanf("%s", reponse);
                                    conv_maj(reponse);
                                    viderBuffer();
                                    if (reponse[0] == 'O')
                                    {
                                        (*negociant).argent += demande;
                                        (*cible).argent -= demande;
                                        (*negociant).cartesSortiePrison--;
                                        (*cible).cartesSortiePrison++;
                    
                                        if (typeCarte == 1)
                                        {
                                            (*negociant).libereChance = 0;
                                            (*cible).libereChance = 1;
                                        }
                                        else
                                        {
                                            (*negociant).libereCommunaute = 0;
                                            (*cible).libereCommunaute = 1;
                                        }
                    
                                        printf("Transaction réussie ! %s a vendu sa carte 'Libéré de prison' à %s pour %d M.\n", (*negociant).Nom, (*cible).Nom, demande);
                                    }
                                    else if (reponse[0] == 'N')
                                    {
                                        printf("Transaction annulée par %s.\n", (*cible).Nom);
                                    }
                                    else
                                    {
                                        printf("Saisie non valide, veuillez entrer O(Oui) ou N(Non)\n");
                                    }
                                }
                            }
                        }
                    break;                
                case 3:
                    printf("Négociation pour la carte 'Libéré de prison' annulée.\n");
                    break;
                default:
                    printf("Choix invalide. Négociation annulée.\n");
                    break;
                }
                choix = -1; 
            }
            break;
            default:
                printf("Choix invalide.\n");
                break;
            }
        }
    }
}



// --- Fonction permettant d'xécuter les conséquences d’une carte Chance ou Caisse de communauté ---

void appliquer_effet_carte(struct Carte carte, struct joueur *joueur, struct joueur player[], int nbJoueurs)
{
    printf("\n🃏 Carte tirée : %s\n", carte.description);
    int montant;
    int destination_case = -1;
    int joueur_passe_depart = 0;
    int joueurActuelIndex = joueur - player; // Obtenir l'index du joueur actuel 

    switch (carte.effet)
    {
    case 0: // Gain ou paiement direct
        (*joueur).argent += carte.valeur;
        printf("💰 %s reçoit %d M.\n", (*joueur).Nom, carte.valeur);
        break;

    case 1: // Aller en prison
        (*joueur).enPrison = 1;
        (*joueur).position = 10; 
        (*joueur).tourEnPrison = 0;
        printf("🚔 %s est envoyé en prison !\n", (*joueur).Nom);
        break;

    case 2: // Carte "Libéré de prison"
        (*joueur).cartesSortiePrison++;
        printf(" %s reçoit une carte 'Libéré de prison' !\n", (*joueur).Nom);
        break;

    case 3: // Avancer jusqu'à la case départ
        (*joueur).position = 0;
        (*joueur).argent += carte.valeur;
        printf("🚶 %s avance jusqu'à la case départ\n", (*joueur).Nom);
        appliquer_effet_case(&plateau[(*joueur).position], joueur, player, 0, 0);
        break;

    case 4: // Amende
        (*joueur).argent -= carte.valeur;
        printf("💰 %s doit payer M%d!\n", (*joueur).Nom, carte.valeur);
        if ((*joueur).argent < 0 && (*joueur).enJeu)
        {
            printf("!! Faillite potentielle après amende de carte !!\n");
            gererFaillite(joueur, player, nbJoueurs, plateau, -1);
        }
        break;
    case 5: // Reculer de X cases
        (*joueur).position -= carte.valeur;
        printf("🔄 %s recule de %d cases et est maintenant à la case %d.\n", (*joueur).Nom, carte.valeur, (*joueur).position + 1);
        appliquer_effet_case(&plateau[(*joueur).position], joueur, player, 0, 0);
        if ((*joueur).argent < 0 && (*joueur).enJeu)
        {
            gererFaillite(joueur, player, nbJoueurs, plateau, -1);
        }
        break;
    case 6: // Payer un montant à chaque joueur
    {
        int joueurActuelIndex = joueur - player; 
        for (int i = 0; i < nbJoueurs; i++)      
        {
            // Si ce n'est pas le joueur actuel ET si le joueur cible est en jeu
            if (i != joueurActuelIndex && player[i].enJeu)
            {
                // On crédite le joueur cible 
                player[i].argent += carte.valeur;
                // On débite le joueur actuel 
                (*joueur).argent -= carte.valeur;

                printf("💰 %s paie M %d à %s\n", (*joueur).Nom, carte.valeur, player[i].Nom);

                if ((*joueur).argent < 0 && (*joueur).enJeu)
                {
                    printf("!! Faillite potentielle de %s en payant %s !!\n", (*joueur).Nom, player[i].Nom);
                    gererFaillite(joueur, player, nbJoueurs, plateau, i);
                    i = nbJoueurs;
                }
            }
        }
        break;
    }
    case 10: // Avancer jusqu'au prochain service public
        joueur_passe_depart = 0;

        // On détermine la destination et si le joueur passe par la case départ
        if ((*joueur).position < 12 || (*joueur).position > 28) 
        {
            destination_case = 12; 
            if ((*joueur).position > 28)
            { 
                joueur_passe_depart = 1;
            }
        }
        else 
        {
            destination_case = 28; 
        }

        printf("🚶 %s se rend à %s (case %d)\n", (*joueur).Nom, plateau[destination_case].nom, destination_case);

        // Gérer l'argent si on passe par Départ
        if (joueur_passe_depart)
        {
            printf("... et passe par la case Départ. RECEVEZ M200 !\n");
            (*joueur).argent += ARGENT_DEPART;
        }

        (*joueur).position = destination_case;
        struct CasePlateau *caseCible = &plateau[(*joueur).position];
        int proprioIndex = (*caseCible).proprietaire;
        int joueurActuelIndex = joueur - player; 

        // Vérification de l'état de la case cible
        if (proprioIndex == -1) 
        {
            printf("Le service public n'appartient à personne.\n");
            appliquer_effet_case(caseCible, joueur, player, 0, 0);
        }
        else if (proprioIndex == joueurActuelIndex) 
        {
            printf("Vous arrivez sur votre propre service public.\n");
        }
        else if (!(*caseCible).estHypothequee)
        {
            printf("Ce service public appartient à %s.\n", player[proprioIndex].Nom);
            // Logique spécifique de la carte : nouveau lancer de dés 
            int d1_nouveau = lancer_de();
            int d2_nouveau = lancer_de();
            int somme_des = d1_nouveau + d2_nouveau;
            int loyer_special = 10 * somme_des;

            printf("🎲 Lancer pour loyer service public : %d et %d (Total: %d)\n", d1_nouveau, d2_nouveau, somme_des);
            printf("💰 %s doit payer %d M (10 x %d) de loyer spécial à %s.\n", (*joueur).Nom, loyer_special, somme_des, player[proprioIndex].Nom);

            (*joueur).argent -= loyer_special;
            player[proprioIndex].argent += loyer_special;

            if ((*joueur).argent < 0 && (*joueur).enJeu)
            {
                printf("!! Faillite potentielle en payant le loyer spécial !!\n");
                gererFaillite(joueur, player, nbJoueurs, plateau, proprioIndex);
            }
        }
        else if (player[proprioIndex].enJeu && (*caseCible).estHypothequee)
        {
            printf("Ce service public appartient à %s mais est hypothéqué. Pas de loyer.\n", player[proprioIndex].Nom);
        }
        break;

    case 11: // Aller à Henri-Martin
        destination_case = 24;
        joueur_passe_depart = 0;
        if ((*joueur).position > destination_case)
        {
            (*joueur).position = destination_case;
            printf("Le joueur %s passe par la case de départ et reçoit M200 ! \n", (*joueur).Nom);
            (*joueur).argent += carte.valeur;
            appliquer_effet_case(&plateau[destination_case], joueur, player, 0, 0);
        }
        else
        {
            (*joueur).position = destination_case;
            appliquer_effet_case(&plateau[destination_case], joueur, player, 0, 0);
        }
        printf("%s se rend à l'Avenue Henri-Martin\n", (*joueur).Nom);
        break;
    case 12: // Aller à la Rue de la Paix
        destination_case = 39;
        (*joueur).position = destination_case;
        printf(" %s se rend à la Rue de la Paix.\n", (*joueur).Nom);
        appliquer_effet_case(&plateau[destination_case], joueur, player, 0, 0);
        break;
    case 13: // Recevoir un paiement de chaque joeur
        joueurActuelIndex = (int)(joueur - player); 
        int i; 
        for (int i = 0; i < nbJoueurs; i++)
        {
            if (i != joueurActuelIndex && player[i].enJeu)
            {
                player[i].argent -= carte.valeur;
                (*joueur).argent += carte.valeur;
                printf("💰 %s paie M %d à %s.\n", player[i].Nom, carte.valeur, (*joueur).Nom);
            }
        }
        break;
    case 14: // Avancer jusqu'à la prochaine gare
    {        
        int destination_case = -1;
        int joueur_passe_depart = 0;
        int ancienne_position = (*joueur).position; // Mémorisation de la position de départ

        // --- Déterminer la prochaine gare ---
        if (ancienne_position >= 35 || ancienne_position < 5)
        {
            destination_case = 5;
            if (ancienne_position >= 35)
            { 
                joueur_passe_depart = 1;
            }
        }
        else if (ancienne_position >= 5 && ancienne_position < 15)
        {
            destination_case = 15;
        }
        else if (ancienne_position >= 15 && ancienne_position < 25)
        {
            destination_case = 25;
        }
        else
        { 
            destination_case = 35;
        }

        printf("🚶 %s avance jusqu'à la prochaine gare: %s (case %d)\n", (*joueur).Nom, plateau[destination_case].nom, destination_case);

        // --- Gérer le passage par Départ ---
        if (joueur_passe_depart)
        {
            printf("... et passe par la case Départ. RECEVEZ M200 !\n");
            (*joueur).argent += 200;
        }

        (*joueur).position = destination_case;

        // --- Logique Spécifique à l'Atterrissage sur la Gare (via carte) ---
        struct CasePlateau *caseGare = &plateau[(*joueur).position];
        int proprioGare = (*caseGare).proprietaire;

        if (proprioGare == -1)
        {
            printf("La gare est libre.\n");
            appliquer_effet_case(caseGare, joueur, player, 0, 0);
        }
        else if (proprioGare == joueurActuelIndex)
        {
            printf("Vous arrivez sur votre propre gare.\n");
        }
        else if (player[proprioGare].enJeu && (*caseGare).estHypothequee && proprioGare != joueurActuelIndex)
        {
            printf("La gare appartient à %s mais est hypothéquée. Pas de loyer.\n", player[proprioGare].Nom);
        }
        else if (player[proprioGare].enJeu && !(*caseGare).estHypothequee && proprioGare != joueurActuelIndex)
        {
            printf("La gare appartient à %s.\n", player[proprioGare].Nom);
            int nbGaresProprio = calcul_gares(plateau, proprioGare);
            int loyer_normal = 0;
            if (nbGaresProprio > 0)
            {
                loyer_normal = 25;
                int k; 
                for (k = 1; k < nbGaresProprio; k++)
                {
                    loyer_normal *= 2;
                }
            }
            // Calcul et application du loyer doublé

            int loyer_a_payer = loyer_normal * 2;
            printf("💰 %s doit payer un loyer de %d M à %s.\n", (*joueur).Nom, loyer_a_payer, player[proprioGare].Nom);
            (*joueur).argent -= loyer_a_payer;
            player[proprioGare].argent += loyer_a_payer;

            if ((*joueur).argent < 0 && (*joueur).enJeu)
            {
                printf("!! Faillite potentielle en payant le loyer de gare !!\n");
                gererFaillite(joueur, player, nbJoueurs, plateau, proprioGare);
            }
        }
        else if (!player[proprioGare].enJeu)
        {
            printf("La gare appartient à %s mais ce joueur n'est plus en jeu. Pas de loyer.\n", player[proprioGare].Nom);
        }
    }
    break;
    case 15: // Aller au Boulevard de la Villette
        if ((*joueur).position > 11)
        {
            printf("%s passe par la case de départ et reçoit M200 !\n", (*joueur).Nom);
            (*joueur).argent += carte.valeur;
            (*joueur).position = 11;
            appliquer_effet_case(&plateau[(*joueur).position], joueur, player, 0, 0);
        }
        else
        {
            (*joueur).position = 11;
            appliquer_effet_case(&plateau[(*joueur).position], joueur, player, 0, 0);
        }
        break; 
    case 17: // Avancer jusqu'à la Gare Montparnasse
        (*joueur).position = 5;
        (*joueur).argent += carte.valeur;
        printf("%s se rend à la Gare Montparnasse!\n", (*joueur).Nom);
        appliquer_effet_case(&plateau[(*joueur).position], joueur, player, 0, 0);
        break;
    case 18: // Communauté paiement pour maisons/hôtels
    case 19: // Chanche paiement pour maisons/hotels
    {
        int maisons = totalMaisons(joueur, plateau, NB_CASES);
        int hotels = totalHotels(joueur, plateau, NB_CASES);
        int coutReparation;
        if (carte.effet == 18)
        { 
            coutReparation = (carte.valeur * maisons) + (115 * hotels);
            printf("Taxe Communauté: %d maisons * %d + %d hôtels * 115 = %d M\n", maisons, carte.valeur, hotels, coutReparation);
        }
        else
        { 
            coutReparation = (carte.valeur * maisons) + (100 * hotels);
            printf("Taxe Chance: %d maisons * %d + %d hôtels * 100 = %d M\n", maisons, carte.valeur, hotels, coutReparation);
        }
        if (coutReparation > 0)
        {
            printf("💰 %s doit payer %d M \n", (*joueur).Nom, coutReparation);
            (*joueur).argent -= coutReparation;
            if ((*joueur).argent < 0 && (*joueur).enJeu)
            {
                printf("!! Faillite potentielle après taxe sur constructions !!\n");
                gererFaillite(joueur, player, nbJoueurs, plateau, -1);
            }
        }
        else
        {
            printf("Vous n'avez pas de constructions, pas de taxe à payer.\n");
        }
        break;
    }

    } 
}

// --- Calcule le nombre total de maisons possédées par un joueur --- 
int totalMaisons(struct joueur *p, struct CasePlateau plateau[], int nbCases)
{
    int total = 0;
    for (int i = 0; i < (*p).nbProprietes; i++)
    {
        int indice = (*p).proprietes[i];
        total += plateau[indice].nbMaisons;
    }
    return total;
}

// --- Calcule le nombre total d'hôtels possédés par un joueur ---
int totalHotels(struct joueur *p, struct CasePlateau plateau[], int nbCases)
{
    int total = 0;
    for (int i = 0; i < (*p).nbProprietes; i++)
    {
        int indice = (*p).proprietes[i];
        total += plateau[indice].Hotel;
    }
    return total;
}

// --- Calcule le nombre total de gares possédées par un joueur ---
int calcul_gares(struct CasePlateau plateau[], int joueurId)
{
    int nbGares = 0;
    for (int i = 0; i < NB_CASES; i++)
    {
        if (plateau[i].proprietaire == joueurId && plateau[i].couleur == 'G')
        {
            nbGares++;
        }
    }

    return nbGares;
}

// --- Lever l'hypothèque d'une propriété ---

void leverHypotheque(struct joueur *p, struct CasePlateau plateau[], int nbCases)
{
    int saisieOk;
    // Vérifier que le joueur possède au moins une propriété
    if ((*p).nbProprietes == 0)
    {
        printf("Vous ne possédez aucune propriété.\n");
    }
    else
    {
        // Affichage de la liste des propriétés hypothéquées
        int found = 0;
        for (int i = 0; i < (*p).nbProprietes; i++)
        {
            int idx = (*p).proprietes[i];
            if (plateau[idx].estHypothequee)
            {
                int coutLevee = (plateau[idx].hypotheque * 11) / 10;
                printf("%d. %s (Coût de levée: %d M)\n", i + 1, plateau[idx].nom, coutLevee);
                found = 1;
            }
        }

        if (!found)
        {
            printf("Aucune propriété hypothéquée.\n");
        }

        else
        {

            int choix;
            saisieOk = 0;

            while (!saisieOk)
            {
                printf("Entrez le numéro de la propriété pour lever l'hypothèque : ");
                if (scanf("%d", &choix) == 1)
                {
                    viderBuffer();
                    choix -= 1;                       

                    if (choix >= 0 && choix < (*p).nbProprietes)
                    {
                        int idx = (*p).proprietes[choix];
                        if (plateau[idx].estHypothequee)
                        {
                            saisieOk = 1;             
                        }
                        else
                        {
                            printf("Cette propriété n'est pas hypothéquée.\n");
                        }
                    }
                    else
                    {
                        printf("Choix hors plage.\n");
                    }
                }
                else
                {
                    printf("Entrée invalide, veuillez entrer un nombre.\n");
                    viderBuffer();
                }
            }

            int idx = (*p).proprietes[choix];
            if (!plateau[idx].estHypothequee)
            {
                printf("Cette propriété n'est pas hypothéquée.\n");
            }

            else
            {

                int coutLevee = (plateau[idx].hypotheque * 11) / 10;
                if ((*p).argent < coutLevee)
                {
                    printf("Fonds insuffisants pour lever l'hypothèque. Il vous faut %d M.\n", coutLevee);
                }
                else
                {

                    (*p).argent -= coutLevee;
                    plateau[idx].estHypothequee = 0;
                    printf("L'hypothèque de la propriété %s a été levée pour M %d .\n", plateau[idx].nom, coutLevee);
                }
            }
        }
    }
}

// --- Affichage du message de la case DÉPART ---
void afficherLogoMonopoly()
{

    printf("          🎩 Bienvenue sur la case DÉPART ! 🚀\n");
    printf("          💰 Vous recevez M200 ! 🎉\n");
    printf("\n");
}


// --- Mélange aléatoire d’un paquet de cartes ---

void melanger_cartes(struct Carte cartes[], int taille)
{
    struct Carte temp;
    int j;

    for (int i = taille - 1; i > 0; i--)
    {
        j = rand() % (i + 1);
        // Échange de la carte i avec la carte j
        temp = cartes[i];
        cartes[i] = cartes[j];
        cartes[j] = temp;
    }
}

// --- Affichage du plateau pour débogage ---
void afficher_plateau()
{
    printf("\n===== AFFICHAGE DU PLATEAU =====\n\n");

    printf("----------------------------------------------------------------------------------------\n");
    printf("| Idx |               Nom               | Couleur |  Type  | Prix | Loyer | Proprio |\n");
    printf("----------------------------------------------------------------------------------------\n");

    for (int i = 0; i < NB_CASES; i++)
    {
        printf("| %2d  | %-30s |    %c    |   %d    | %4d |  %4d |    %3d   |\n",
               i,
               plateau[i].nom,
               plateau[i].couleur,
               plateau[i].Type,
               plateau[i].prix_achat,
               plateau[i].loyer,
               plateau[i].proprietaire);
    }
    printf("----------------------------------------------------------------------------------------\n\n");
}

// --- Initialisation des cartes spéciales ---
void initialisation_cartes()
{
    FILE *f;
    char NomFichier[20] = "cartes.txt";
    char ligne[512];
    int effet, valeur;
    int iComm = 0;
    int iChance = 0;

    f = fopen(NomFichier, "r");

    while (fgets(ligne, sizeof(ligne), f) != NULL)
    {
        char type[30];
        char description[300];
        // Utilisation de sscanf pour découper la ligne en deux parties séparées par ';'
        if (sscanf(ligne, "%[^;];%d;%d;%[^\n]", type, &effet, &valeur, description))
        {
            // Vérifier le type de carte et le stocker dans le tableau approprié
            if (strcmp(type, "Communauté") == 0)
            {
                if (iComm < MAX_CARTES)
                {
                    strncpy(cartes_communaute[iComm].type, type, sizeof(cartes_communaute[iComm].type) - 1); 
                    cartes_communaute[iComm].type[sizeof(cartes_communaute[iComm].type) - 1] = '\0';
                    cartes_communaute[iComm].effet = effet;
                    cartes_communaute[iComm].valeur = valeur;
                    strncpy(cartes_communaute[iComm].description, description, sizeof(cartes_communaute[iComm].description) - 1);
                    cartes_communaute[iComm].description[sizeof(cartes_communaute[iComm].description) - 1] = '\0';

                    iComm++;
                }
            }
            else if (strcmp(type, "Chance") == 0)
            {
                if (iChance < MAX_CARTES)
                {
                    strncpy(cartes_chance[iChance].type, type, sizeof(cartes_chance[iChance].type) - 1);
                    cartes_chance[iChance].type[sizeof(cartes_chance[iChance].type) - 1] = '\0';
                    cartes_chance[iChance].effet = effet;
                    cartes_chance[iChance].valeur = valeur;
                    strncpy(cartes_chance[iChance].description, description, sizeof(cartes_chance[iChance].description) - 1);
                    cartes_chance[iChance].description[sizeof(cartes_chance[iChance].description) - 1] = '\0';
                    iChance++;
                }
            }
        }
    }
    fclose(f);

    melanger_cartes(cartes_communaute, iComm);
    melanger_cartes(cartes_chance, iChance);

    nbCartesCommunaute = iComm; 
    nbCartesChance = iChance;   
    /* Débogage : vérification de l'initialisation des cartes : 
    afficher_cartes();*/
  
}

// --- Affichage des cartes pour débogage --- 
void afficher_cartes()
{
    printf("\n🃏 Cartes Communauté :\n");
    for (int i = 0; i < MAX_CARTES; i++)
    {
        printf("Effet: %d | Valeur: %d | %s\n", cartes_communaute[i].effet, cartes_communaute[i].valeur, cartes_communaute[i].description);
    }

    printf("\n🃏 Cartes Chance :\n");
    for (int i = 0; i < MAX_CARTES; i++)
    {
        printf("Effet: %d | Valeur: %d | %s\n", cartes_chance[i].effet, cartes_chance[i].valeur, cartes_chance[i].description);
    }
}

// --- Processus d’enchère multijoueur sur une propriété ---
void enchere(struct CasePlateau *propriete, int indicePropriete, struct joueur joueurs[], int nbJoueurs)
{
    int offreCourante = 0;
    int meilleurOffrant = -1;    
    int offreFaitCeTour, i;      
    int continuer_les_tours = 1; 
    int nouvelleOffre;
    int saisieOk_scanf; 
    int action_valide;  
    char c;             

    printf("\n=================================================\n");
    printf("=== 🛎️ Lancement de l'enchère pour : %s ===\n", (*propriete).nom);
    printf("=================================================\n");
    printf("L'offre commence à M 0.\n");
    printf("Entrez 0 pour passer votre tour.\n");

    // --- Boucle principale des tours d'enchère ---
    while (continuer_les_tours)
    {
        offreFaitCeTour = 0;

        printf("\n--- Nouveau Tour d'Enchère (Offre actuelle: M %d ) ---\n", offreCourante);

        // --- Boucle des joueurs ---
        for (i = 0; i < nbJoueurs; i++)
        {
            if (joueurs[i].enJeu && joueurs[i].argent > offreCourante)
            {
                printf("\n--- Tour de %s (Argent: %d M) ---\n", joueurs[i].Nom, joueurs[i].argent);

                action_valide = 0; 

                // Boucle interne pour obtenir une action valide (passer ou enchérir)
                while (!action_valide)
                {
                    saisieOk_scanf = 0; 
                    nouvelleOffre = -1;

                    while (!saisieOk_scanf)
                    {
                        printf("Votre offre (0=Passer, >%d) : ", offreCourante);
                        if (scanf("%d", &nouvelleOffre) == 1)
                        {
                            saisieOk_scanf = 1; 
                        }
                        else
                        {
                            printf("Entrée invalide. Veuillez entrer un nombre.\n");
                        }
                        viderBuffer();
                    } 
                    if (nouvelleOffre == 0)
                    {
                        printf("%s passe.\n", joueurs[i].Nom);
                        action_valide = 1; 
                    }
                    // Si offre valide 
                    else if (nouvelleOffre > offreCourante && nouvelleOffre <= joueurs[i].argent)
                    {
                        printf("Nouvelle offre : M %d par %s.\n", nouvelleOffre, joueurs[i].Nom);
                        offreCourante = nouvelleOffre;
                        meilleurOffrant = i;
                        action_valide = 1;   
                        offreFaitCeTour = 1; // Une offre a été faite dans ce tour de la boucle for
                    }
                    else if (nouvelleOffre > joueurs[i].argent) 
                    {
                        printf("Fonds insuffisants (vous avez %d M). Entrez 0 ou une offre inférieure.\n", joueurs[i].argent);
                        action_valide = 0; 
                    }
                    else
                    {
                        printf("Offre invalide. Elle doit être supérieure à M %d. Entrez 0 pour passer.\n", offreCourante);
                        action_valide = 0; 
                    }
                
                } 

            } 
            else if (joueurs[i].enJeu) 
            {
                printf("--- %s ne peut pas surenchérir (Argent: %d M vs Offre: %d M) ---\n", joueurs[i].Nom, joueurs[i].argent, offreCourante);
            }

        } 

        // Décider s'il faut continuer les tours d'enchère
        if (offreFaitCeTour == 0) // Si personne n'a fait d'offre dans ce tour, on arrête la boucle principale 
        {
            continuer_les_tours = 0; 
        }

    } 

    printf("\n=================================================\n");
    // --- Fin de l'enchère : attribution ---
    if (meilleurOffrant != -1)
    {
        printf("🏆 Enchère terminée ! %s remporte %s pour %d M.\n", joueurs[meilleurOffrant].Nom, (*propriete).nom, offreCourante);
        joueurs[meilleurOffrant].argent -= offreCourante;
        (*propriete).proprietaire = meilleurOffrant;

        if (joueurs[meilleurOffrant].nbProprietes < MAX_PROPRIETES)
        {
            joueurs[meilleurOffrant].proprietes[joueurs[meilleurOffrant].nbProprietes] = indicePropriete;
            joueurs[meilleurOffrant].nbProprietes++;
        }
        else
        {
            printf("Erreur: %s a déjà trop de propriétés !\n", joueurs[meilleurOffrant].Nom);
        }

    }
    else
    {
        printf(" Aucune enchère n'a été faite pour %s. La propriété reste à la banque.\n", (*propriete).nom);
    }
    printf("=================================================\n");
}

//--- Sauvegarde d'une partie ---
void sauvegarderPartie(struct joueur player[], int nbjoueurs, struct CasePlateau plateau[], int nbCases)
{
    FILE *fichier;
    char nomFichier[50];

    printf("Entrez le nom du fichier de sauvegarde : ");
    scanf("%s", nomFichier);
    viderBuffer();
    if (strcmp(nomFichier, "regles.txt") == 0 || strcmp(nomFichier, "cartes.txt") == 0)
    {
        printf("Ce fichier n'est pas valide\n");
    }
    else
    {
        fichier = fopen(nomFichier, "w");
        if (fichier == NULL)
        {
            printf("Erreur lors de l'ouverture du fichier de sauvegarde.\n");
        }
        else
        {
            // Sauvegarde du mode de jeu (1=classique, 2=tours, 3=monétaire)
            fprintf(fichier, "%d\n", mode);
            if (mode == 2) 
            { 
                fprintf(fichier, "%d\n", nb_tours_total); // Nb total de tours
            } 
            else if (mode == 3) 
            {                   
                fprintf(fichier, "%d\n", objectif_monetaire); // Objectif monétaire
            }
           
            fprintf(fichier, "%d\n", tour_actuel);   // Tour actuel

            // Nombre de joueurs
            fprintf(fichier, "%d\n", nbjoueurs);

            // Informations des joueurs 
            for (int i = 0; i < nbjoueurs; i++)
            {
                fprintf(fichier, "%s;%d;%d;%d;%d;%d;%d;%d;%d;%d\n",
                        player[i].Nom,
                        player[i].argent,
                        player[i].position,
                        player[i].nbProprietes,
                        player[i].enPrison,
                        player[i].tourEnPrison,
                        player[i].cartesSortiePrison,
                        player[i].enJeu,
                        player[i].libereChance,
                        player[i].libereCommunaute);

                // Tableau des propriétés
                for (int j = 0; j < MAX_PROPRIETES; j++)
                {
                    fprintf(fichier, "%d ", player[i].proprietes[j]);
                }
                fprintf(fichier, "\n");
            }

            // Sauvegarde du plateau 
            for (int i = 0; i < NB_CASES; i++)
            {
                fprintf(fichier, "%s;%s;%c;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d\n",
                        plateau[i].nom,
                        plateau[i].nom_abrege,
                        plateau[i].couleur,
                        plateau[i].prix_achat,
                        plateau[i].prix_maison,
                        plateau[i].loyer,
                        plateau[i].loyer1m,
                        plateau[i].loyer2m,
                        plateau[i].loyer3m,
                        plateau[i].loyer4m,
                        plateau[i].loyerh,
                        plateau[i].hypotheque,
                        plateau[i].Type,
                        plateau[i].proprietaire,
                        plateau[i].estHypothequee,
                        plateau[i].nbMaisons,
                        plateau[i].Hotel);
            }

            // Sauvegarde des cartes Chance
            for (int i = 0; i < MAX_CARTES; i++)
            {
                fprintf(fichier, "%s;%d;%d;%s\n",
                        cartes_chance[i].type,
                        cartes_chance[i].effet,
                        cartes_chance[i].valeur,
                        cartes_chance[i].description);
            }

            // Sauvegarde des cartes Communauté
            for (int i = 0; i < MAX_CARTES; i++)
            {
                fprintf(fichier, "%s;%d;%d;%s\n",
                        cartes_communaute[i].type,
                        cartes_communaute[i].effet,
                        cartes_communaute[i].valeur,
                        cartes_communaute[i].description);
            }

            fclose(fichier);
            printf("Partie sauvegardée avec succès dans le fichier %s.\n", nomFichier);
        }
    }
}

// --- Charge une partie depuis un fichier de sauvegarde et lance le jeu. L'exécution est interrompue (avec des return) dès qu'une erreur critique est détectée ---
void charger_partie()
{
    FILE *fichier;
    char nomFichier[50];
    int c; 

    printf("Entrez le nom du fichier de sauvegarde : ");
    if (scanf("%s", nomFichier) != 1) 
    {
         viderBuffer();
         printf("Erreur de saisie du nom.\n");
         return; // Sortir de la fonction si la saisie échoue
    }

    viderBuffer();

    if (strcmp(nomFichier, "regles.txt") == 0 || strcmp(nomFichier, "cartes.txt") == 0 || strcmp(nomFichier, "plateau.txt") == 0 || strcmp(nomFichier, "meilleurs_scores.txt") == 0)
    {
        printf("Ce nom de fichier n'est pas valide pour une sauvegarde.\n");
        return; // Sortir si nom invalide
    }

    fichier = fopen(nomFichier, "r");
    if (fichier == NULL)
    {
        printf("Erreur lors de l'ouverture du fichier de sauvegarde\n");
        return; // Sortir si ouverture échoue
    }

    // --- Variables locales pour stocker temporairement avant d'affecter les globales ---
    int local_mode;
    int local_nbtours_total = 0;
    int local_tour_actuel = 0;
    int local_objectif_monetaire = 0;
    int local_nbjoueurs;

    // 1. Lecture du mode de jeu
    if (fscanf(fichier, "%d", &local_mode) != 1) 
    { 
        printf("Erreur de lecture du mode de jeu.\n"); fclose(fichier); 
        return; 
    }
    while ((c = fgetc(fichier)) != '\n' && c != EOF);

   // 2. Lecture des infos spécifiques au mode (SI mode 2 ou 3)
    if (local_mode == 2) 
    { 
        if (fscanf(fichier, "%d", &local_nbtours_total) != 1) 
        {
            printf("Erreur lecture nbtours_total.\n"); 
            fclose(fichier); 
            return;
        }
        while ((c = fgetc(fichier)) != '\n' && c != EOF);

        if (fscanf(fichier, "%d", &local_tour_actuel) != 1) 
        {
            printf("Erreur lecture tour_actuel.\n"); 
            fclose(fichier); 
            return;
        }
        while ((c = fgetc(fichier)) != '\n' && c != EOF);
    } 
    else if (local_mode == 3) 
    { 
        if (fscanf(fichier, "%d", &local_objectif_monetaire) != 1) 
        {
            printf("Erreur lecture objectif_monetaire.\n"); 
            fclose(fichier); 
            return;
        }
        while ((c = fgetc(fichier)) != '\n' && c != EOF);

        if (fscanf(fichier, "%d", &local_tour_actuel) != 1) { // Lecture tour_actuel aussi
            printf("Erreur lecture tour_actuel (mode argent).\n"); 
            fclose(fichier); 
            return;
        }
        while ((c = fgetc(fichier)) != '\n' && c != EOF);
    }
    else if (local_mode == 1) 
    {
        if (fscanf(fichier, "%d", &local_tour_actuel) != 1) 
        {
            printf("Erreur lecture tour_actuel (mode classique).\n"); 
            fclose(fichier); 
            return;
        }
        while ((c = fgetc(fichier)) != '\n' && c != EOF);
    }


    // 3. Lecture du nombre de joueurs
    if (fscanf(fichier, "%d", &local_nbjoueurs) != 1) 
    { 
        printf("Erreur de lecture du nombre de joueurs.\n"); fclose(fichier); return;
    }
    while ((c = fgetc(fichier)) != '\n' && c != EOF);

    // --- Vérifier si nbjoueurs est raisonnable avant d'allouer/utiliser player ---
    if (local_nbjoueurs < 2 || local_nbjoueurs > MAX_PLAYERS) 
    {
        printf("Nombre de joueurs (%d) invalide dans le fichier de sauvegarde.\n", local_nbjoueurs);
        fclose(fichier);
        return;
    }

    // --- Affecter les variables globales principales ---
    mode = local_mode;
    nb_tours_total = local_nbtours_total;
    tour_actuel = local_tour_actuel;
    objectif_monetaire = local_objectif_monetaire;
    nbjoueurs = local_nbjoueurs; 

    // 4. Lecture des joueurs (Infos + Propriétés)
    for (int i = 0; i < nbjoueurs; i++) 
    { 
        if (fscanf(fichier, "%[^;];%d;%d;%d;%d;%d;%d;%d;%d;%d",player[i].Nom, &player[i].argent, &player[i].position, &player[i].nbProprietes, &player[i].enPrison, &player[i].tourEnPrison, &player[i].cartesSortiePrison, &player[i].enJeu, &player[i].libereChance, &player[i].libereCommunaute) != 10) 
        {
            printf("Erreur lecture infos joueur %d (index i=%d).\n", i + 1, i); 
            fclose(fichier); 
            return;
        }
        while ((c = fgetc(fichier)) != '\n' && c != EOF);

        for (int j = 0; j < MAX_PROPRIETES; j++) 
        {
            if (fscanf(fichier, "%d", &player[i].proprietes[j]) != 1) 
            {
                printf("Erreur lecture propriétés joueur %d (index i=%d, prop j=%d).\n", i + 1, i, j); 
                fclose(fichier); 
                return;
            }
        }
        while ((c = fgetc(fichier)) != '\n' && c != EOF);
    }

    // 5. Lecture du plateau 
    for (int i = 0; i < NB_CASES; i++) 
    {
        if (fscanf(fichier, "%[^;];%[^;];%c;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d",
                   plateau[i].nom, 
                   plateau[i].nom_abrege, 
                   &plateau[i].couleur,
                   &plateau[i].prix_achat, 
                   &plateau[i].prix_maison, 
                   &plateau[i].loyer,
                   &plateau[i].loyer1m, 
                   &plateau[i].loyer2m, 
                   &plateau[i].loyer3m,
                   &plateau[i].loyer4m, 
                   &plateau[i].loyerh, 
                   &plateau[i].hypotheque, 
                   &plateau[i].Type,
                   &plateau[i].proprietaire, 
                   &plateau[i].estHypothequee,
                   &plateau[i].nbMaisons,
                   &plateau[i].Hotel) != 17) 
        {
            printf("Erreur lecture case %d plateau.\n", i); 
            fclose(fichier); 
            return;
        }
        while ((c = fgetc(fichier)) != '\n' && c != EOF);
    }

    // 6. Lecture des cartes Chance
    for (int i = 0; i < MAX_CARTES; i++) 
    {
        if (fscanf(fichier, "%[^;];%d;%d;%[^\n]\n", cartes_chance[i].type, &cartes_chance[i].effet,&cartes_chance[i].valeur, cartes_chance[i].description) != 4) 
        {
            printf("Erreur lecture carte Chance %d.\n", i); 
            fclose(fichier); 
            return;
        }
    }

    // 7. Lecture des cartes Communauté
    for (int i = 0; i < MAX_CARTES; i++) 
    {
        if (fscanf(fichier, "%[^;];%d;%d;%[^\n]\n",cartes_communaute[i].type, &cartes_communaute[i].effet, &cartes_communaute[i].valeur, cartes_communaute[i].description) != 4) 
        {
            printf("Erreur lecture carte Communauté %d.\n", i); 
            fclose(fichier); 
            return;
        }
    }

    fclose(fichier);
    printf("Partie chargée avec succès depuis le fichier %s.\n", nomFichier);

    // --- Préparer et lancer le jeu ---
    jeufini = 0; 
    debut = 0;   

    printf("Lancement de la partie chargée (Mode: %d)...\n", mode);
    switch (mode)
    {
        case 1:
            partie_classique();
            break;
        case 2:
            partie_tours();
            break;
        case 3:
            partie_money();
            break;
        default:
            printf("Mode de jeu chargé inconnu (%d), lancement classique.\n", mode);
            mode = 1;
            partie_classique();
            break;
    }
}

// --- Vide le tampon d’entrée standard jusqu’au prochain caractère de fin de ligne ---
void viderBuffer()
{
    char c;
    while ((c = getchar()) != '\n' && c != EOF);
}


/* --- Calcule les scores de la partie, fusionne-les avec le classement existant, trie le tout par ordre décroissant 
et réécrit le fichier meilleurs_scores.txt avec les 10 meilleures performances --- */
void mettreAJourMeilleursScores()
{
    struct ScoreEntry bestScores[MAX_BEST_SCORE * 2]; // scores existants + nouveaux scores
    int count = 0;
    int oldBest = 0; 
    FILE *fp = fopen("meilleurs_scores.txt", "r");
    char ligne[100];

    // Lecture du fichier de scores existant (si présent)
    if (fp != NULL)
    {
        while (fgets(ligne, sizeof(ligne), fp) != NULL && count < MAX_BEST_SCORE)
        {
            sscanf(ligne, "%[^;];%d", bestScores[count].nom, &bestScores[count].score);
            if (bestScores[count].score > oldBest)
            {
                oldBest = bestScores[count].score;
            }
            count++;
        }
        fclose(fp);
    }

    // Calcul et ajout des scores de la partie actuelle pour chaque joueur en jeu
    for (int i = 0; i < nbjoueurs; i++)
    {
        if (player[i].enJeu)
        {
            int valeurProprietes = 0;                
            int valeurProprietesHypothequees = 0;    
            int valeurProprietesNonHypothequees = 0; 
            int valeurConstructions = 0;             
            int valeurMaisons = 0;                   
            int valeurHotels = 0;                    

            printf("Calcul du score pour %s...\n", player[i].Nom);

            // Calcul de la valeur des propriétés et constructions du joueur
            for (int j = 0; j < player[i].nbProprietes; j++)
            {
                int idx = player[i].proprietes[j];
                if (idx >= 0 && idx < NB_CASES)
                {
                    if (plateau[idx].estHypothequee)
                    {
                        int val_prop_hyp = (plateau[idx].prix_achat - plateau[idx].hypotheque); 
                        valeurProprietesHypothequees += val_prop_hyp;
                        valeurProprietes += val_prop_hyp; 
                    }
                    else 
                    {
                        valeurProprietesNonHypothequees += plateau[idx].prix_achat;
                        valeurProprietes += plateau[idx].prix_achat; 

                        // Calcul valeur constructions
                        valeurMaisons += plateau[idx].nbMaisons * plateau[idx].prix_maison;
                        if (plateau[idx].Hotel)
                        {
                            valeurHotels += plateau[idx].prix_maison * 5; 
                        }
                    }
                }
            }
            valeurConstructions = valeurMaisons + valeurHotels;

            // Calcul du score final
            int score = player[i].argent + valeurProprietes + valeurConstructions;

            // --- Affichage détaillé du score ---

            printf("  > Argent liquide          : %6d M\n", player[i].argent);
            printf("  > Valeur Prop. non hypoth.: %6d M\n", valeurProprietesNonHypothequees);
            printf("  > Valeur Prop. hypothéquées: %6d M (Prix - Valeur Hypothèque)\n", valeurProprietesHypothequees);
            printf("    ------------------------------------\n");
            printf("  > SOUS-TOTAL PROPRIÉTÉS   : %6d M\n", valeurProprietes);
            printf("    ------------------------------------\n");
            printf("  > Valeur Maisons (%d)      : %6d M\n", totalMaisons(&player[i], plateau, NB_CASES), valeurMaisons);
            printf("  > Valeur Hôtels (%d)       : %6d M\n", totalHotels(&player[i], plateau, NB_CASES), valeurHotels);
            printf("    ------------------------------------\n");
            printf("  > SOUS-TOTAL CONSTRUCTIONS: %6d M\n", valeurConstructions);
            printf("  ====================================\n");
            printf("  > SCORE TOTAL pour %-8s : %6d M\n", player[i].Nom, score);
            printf("  ====================================\n");
        

            if (score > oldBest)
            {
                printf("Félicitations %s ! Vous avez battu le record précédent (ancien record : %d, votre score : %d) !\n", player[i].Nom, oldBest, score);
                oldBest = score; 
            }
            strcpy(bestScores[count].nom, player[i].Nom);
            bestScores[count].score = score;
            count++;
        }
    }

    // Tri du tableau par score décroissant 
    for (int i = 0; i < count - 1; i++)
    {
        for (int j = i + 1; j < count; j++)
        {
            if (bestScores[j].score > bestScores[i].score)
            {
                struct ScoreEntry temp = bestScores[i];
                bestScores[i] = bestScores[j];
                bestScores[j] = temp;
            }
        }
    }

    // Conserver uniquement les MAX_BEST_SCORE meilleurs scores
    int newCount = 10;

    // Réécriture du fichier avec les scores mis à jour 
    fp = fopen("meilleurs_scores.txt", "w"); 
    if (fp == NULL)
    {
        printf("Erreur lors de l'ouverture du fichier meilleurs_scores.txt pour écriture.\n");
    }
    else
    {
        for (int i = 0; i < newCount; i++)
        {
            fprintf(fp, "%s;%d\n", bestScores[i].nom, bestScores[i].score);
        }
        fclose(fp);

        printf("\n=== Meilleurs scores ===\n");
        for (int i = 0; i < newCount; i++)
        {
            printf("%d. %s : %d\n", i + 1, bestScores[i].nom, bestScores[i].score);
        }
    }
}

// --- Affichage des meilleurs scores ---
void afficherMeilleursScores()
{
    FILE *fp = fopen("meilleurs_scores.txt", "r");
    if (fp == NULL)
    {
        printf("Aucun meilleur score n'a été enregistré.\n");
    }
    else
    {

        struct ScoreEntry scores[MAX_BEST_SCORE];
        int count = 0;
        char ligne[256];

        // Lecture du fichier, chaque ligne au format "Nom;Score"
        while (fgets(ligne, sizeof(ligne), fp) != NULL && count < MAX_BEST_SCORE)
        {
            if (sscanf(ligne, "%[^;];%d", scores[count].nom, &scores[count].score) == 2)
            {
                count++;
            }
        }
        fclose(fp);

        // Tri du tableau par ordre décroissant de score
        for (int i = 0; i < count - 1; i++)
        {
            for (int j = i + 1; j < count; j++)
            {
                if (scores[j].score > scores[i].score)
                {
                    struct ScoreEntry temp = scores[i];
                    scores[i] = scores[j];
                    scores[j] = temp;
                }
            }
        }

        // Affichage du classement avec rang
        printf("\n=== Meilleurs scores ===\n");
        printf("+------+----------------------+---------+\n");
        printf("| Rang | %-20s | %-7s |\n", "Nom", "Score");
        printf("+------+----------------------+---------+\n");
        for (int i = 0; i < count; i++)
        {
            printf("| %-4d | %-20s | %-7d |\n", i + 1, scores[i].nom, scores[i].score);
        }
        printf("+------+----------------------+---------+\n\n");
    }
}

void gererFaillite(struct joueur *p, struct joueur players[], int num_players, struct CasePlateau board[], int creancierIndex)
{
    int joueurIndex = p - players;
    int choix = 0;
    int saisieOk = 0;
    char c;

    printf("\n⚠️ %s est en difficulté financière (Argent : %d M) ⚠️\n", (*p).Nom, (*p).argent);
    viderBuffer();

    // --- Menu interactif tant que le joueur est en négatif ---
    while ((*p).argent < 0 && choix != 3)
    {

        saisieOk = 0;
        choix = -1; 
        while (!saisieOk)
        {
            printf("\n--- MENU DE LIQUIDATION POUR %s ---\n", (*p).Nom);
            printf("Argent actuel : %d M (Négatif!)\n", (*p).argent); // Indiquer que c'est négatif
            printf("1. Vendre des constructions (maisons / hôtels)\n");
            printf("2. Hypothéquer une propriété\n");
            printf("3. Déclarer faillite\n");
            printf("Votre choix (1-3) : ");

            // Tenter de lire un entier
            if (scanf("%d", &choix) == 1)
            {
                viderBuffer();

                if (choix >= 1 && choix <= 3)
                {
                    saisieOk = 1; 
                }
                else
                {
                    printf("Choix invalide. Veuillez entrer 1, 2 ou 3.\n");
                    saisieOk = 0; 
                }
            }
            else 
            {
                printf("Entrée invalide. Veuillez entrer un nombre (1, 2 ou 3).\n");
                viderBuffer();
                saisieOk = 0; 
            }
        } 
        switch (choix)
        {
        case 1:
            vendreConstructions(p, board);
            break;
        case 2:
            hypothequerPropriete(p, board, NB_CASES);
            break;
        case 3:
            printf("Vous avez choisi de déclarer faillite.\n");
            break;
        default:
            printf("Choix invalide. Veuillez réessayer.\n");
        }
    }

    // Si le joueur a récupéré assez d'argent, on quitte la procédure
    if ((*p).argent >= 0)
    {
        printf("✅ %s a rassemblé les fonds nécessaires (%d M).\n", (*p).Nom, (*p).argent);
    }
    else
    {

        // --- Déclaration de faillite ---
        printf("\n💥 %s est en FAILLITE !\n", (*p).Nom);
        (*p).enJeu = 0;
        (*p).argent = 0;

        // Remise des cartes de sortie de prison
        if ((*p).cartesSortiePrison > 0) // Vérifier si le joueur avait au moins une carte
        {
            printf("Retour des %d carte(s) 'Sortie de prison' de %s...\n", (*p).cartesSortiePrison, (*p).Nom);
             
            if ((*p).libereChance > 0) 
            {
                struct Carte carteRetourChance;
                strcpy(carteRetourChance.type, "Chance"); 
                carteRetourChance.effet = 2;
                carteRetourChance.valeur = 0;
                strcpy(carteRetourChance.description, "VOUS ÊTES LIBÉRÉ DE PRISON. Cette carte peut être conservée jusqu'à ce qu'elle soit utilisée, échangée ou vendue.");
     
                reintegrerCarteSortieDePrison(carteRetourChance); // Remise dans la bonne pioche
                printf(" -> Une carte 'Sortie de Prison' (Chance) remise dans la pioche.\n");
     
                (*p).libereChance = 0; 
            }
     
            if ((*p).libereCommunaute > 0) 
            {
                struct Carte carteRetourCommunaute;
                strcpy(carteRetourCommunaute.type, "Communauté"); 
                carteRetourCommunaute.effet = 2;
                carteRetourCommunaute.valeur = 0;
                strcpy(carteRetourCommunaute.description, "Ouaf, Ouaf ! Vous adoptez un chiot dans un refuge ! VOUS ÊTES LIBÉRÉ DE PRISON. Conservez cette carte jusqu'à ce qu'elle soit utilisée, échangée ou vendue.");
     
                reintegrerCarteSortieDePrison(carteRetourCommunaute); // Remise dans la bonne pioche
                printf(" -> Une carte 'Sortie de Prison' (Communaute) remise dans la pioche.\n");
     
                (*p).libereCommunaute = 0; 
            }
     
            (*p).cartesSortiePrison = 0;
        }

        // Transfert des propriétés au créancier ou à la banque
        if (creancierIndex >= 0)
        {
            printf("-> Propriétés transférées à %s.\n", players[creancierIndex].Nom);
            transfertBiens(p, &players[creancierIndex], board, joueurIndex, creancierIndex);
        }
        else
        {
            printf("-> Propriétés retournées à la banque.\n");
            transfertBiens(p, NULL, board, joueurIndex, -1);
        }

        verifierFinDePartie(players, num_players, board, NB_CASES);
    }
}



// --- Retourne 1 si toute la couleur est sans constructions, 0 sinon ---
int groupe_est_nu(char couleur, struct CasePlateau plateau[], int nbCases)
{
    for (int i = 0; i < nbCases; i++)
    {
        if (plateau[i].couleur == couleur && (plateau[i].nbMaisons > 0 || plateau[i].Hotel > 0))
        {
            return 0;
        }
    }
    return 1;
}

// --- Retourne 1 si la propriété idx n’a ni maisons ni hôtel et que toutes les cases du même groupe de couleur sont sans constructions, 0 sinon ---

int propriete_est_transférable(int idx, struct CasePlateau plateau[], int nbCases)
{
    return plateau[idx].nbMaisons == 0 && plateau[idx].Hotel == 0 && groupe_est_nu(plateau[idx].couleur, plateau, nbCases);
}
