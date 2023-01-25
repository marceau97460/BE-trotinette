
/*
	!!!! NB : ALIMENTER LA CARTE AVANT DE CONNECTER L'USB !!!

VERSION 16/12/2021 :
- ToolboxNRJ V4
- Driver version 2021b (synchronisation de la mise à jour Rcy -CCR- avec la rampe)
- Validé Décembre 2021

*/


/*
STRUCTURE DES FICHIERS

COUCHE APPLI = Main_User.c : 
programme principal à modifier. Par défaut hacheur sur entrée +/-10V, sortie 1 PWM
Attention, sur la trottinette réelle, l'entrée se fait sur 3V3.
Attention, l'entrée se fait avec la poignée d'accélération qui va de 0.6V à 2.7V !

COUCHE SERVICE = Toolbox_NRJ_V4.c
Middleware qui configure tous les périphériques nécessaires, avec API "friendly"

COUCHE DRIVER =
clock.c : contient la fonction Clock_Configure() qui prépare le STM32. Lancée automatiquement à l'init IO
lib : bibliothèque qui gère les périphériques du STM : Drivers_STM32F103_107_Jan_2015_b
*/



#include "ToolBox_NRJ_v4.h"
#include <math.h>




//=================================================================================================================
// 					USER DEFINE
//=================================================================================================================

//#define taui = 0.00268;
//#define Te_us  Te*1000000.0; // conversion en µs pour utilisation dans la fonction d'init d'interruption
#define fT 400
#define Rm 1
#define Lm 0.002
#define R5 5100
#define R8 10000
#define R12 10000
#define R18 12000
#define R21 220
#define C2 0.000000022
#define C7 0.000000022
#define Ubat 24
#define PI (3.14159265359)
#define phi_rad (0.3491) //20 degrés


// Choix de la fréquence PWM (en kHz)
#define FPWM_Khz 20.0
						


//==========END USER DEFINE========================================================================================

// ========= Variable globales indispensables et déclarations fct d'IT ============================================

void IT_Principale(void);
//=================================================================================================================


/*=================================================================================================================
 					FONCTION MAIN : 
					NB : On veillera à allumer les diodes au niveau des E/S utilisée par le progamme. 
					
					EXEMPLE: Ce progamme permet de générer une PWM (Voie 1) à 20kHz dont le rapport cyclique se règle
					par le potentiomètre de "l'entrée Analogique +/-10V"
					Placer le cavalier sur la position "Pot."
					La mise à jour du rapport cyclique se fait à la fréquence 1kHz.

//=================================================================================================================*/



float Te,Te_us;
float Km,Taum,Tau1,Tau2,Kf,Kg,Taui;
float T,K ;
float a0,b0,b1;
int main (void)
{
// !OBLIGATOIRE! //	
Conf_Generale_IO_Carte();	
	

// ------------- Discret, choix de Te -------------------	
Te=0.0002; // en seconde
Te_us= Te*1000000.0; // conversion en µs pour utilisation dans la fonction d'init d'interruption
	
// ------------- Variables -------------------	
	
Km=1/Rm;
Taum =Lm/Rm;
Tau1=(R8*R5)/(R8+R5)*C2;
Tau2=R21*C7; 
Kf=(R12+R8)/(R5+R8);
Kg =2*Km*Ubat;
//Taui=Kf*Kg/(2*PI*400*sqrt(1+(2*PI*Tau1*fT)*(2*PI*Tau1*fT))*sqrt(1+(Tau2*2*PI*fT)*(Tau2*2*PI*fT)));
//Taui=(Kg*Kf)/(2.0*PI*fT*sqrt(4.0*(PI*PI)*(fT*fT)*(Tau1*Tau2)+1.0));
Taui=0.0027;
// Coefficients correcteur


T = (1.0/((PI/phi_rad)*fT));
K = (Lm/Rm)/Taui;
b0 = (-K + (T/(2.0*Taui)));
b1 = (K + (T/(2.0*Taui)));
a0=1;
//______________ Ecrire ici toutes les CONFIGURATIONS des périphériques ________________________________	
// Paramétrage ADC pour entrée analogique
Conf_ADC();
// Configuration de la PWM avec une porteuse Triangle, voie 1 & 2 activée, inversion voie 2
Triangle (FPWM_Khz);
Active_Voie_PWM(1);	
Active_Voie_PWM(2);	
Inv_Voie(2);

Start_PWM;
R_Cyc_1(2048);  // positionnement à 50% par défaut de la PWM
R_Cyc_2(2048);


// Activation LED
LED_Courant_On;
LED_PWM_On;
LED_PWM_Aux_Off;
LED_Entree_10V_On;
LED_Entree_3V3_Off;
LED_Codeur_Off;


// Conf IT
Conf_IT_Principale_Systick(IT_Principale, Te_us);

	while(1)
	{}

}


//=================================================================================================================
// 					FONCTION D'INTERRUPTION PRINCIPALE SYSTICK
//=================================================================================================================
int Courant_1,Cons_In,INV3V3,IN1;
float sortie=0,sortie_preced=0,erreur=0.0,erreur_volt=0.0,erreur_volt_preced=0.0,rapport_cyc=0;

void IT_Principale(void)
{
 INV3V3=Entree_3V3();
 Courant_1=I1();
 erreur=INV3V3-Courant_1;
 erreur_volt=(erreur*3.3)/4096;
 sortie=b1*erreur_volt+b0*erreur_volt_preced+sortie_preced;
 if (sortie>0.5)
 {
	 sortie=0.5;
 }
 else if (sortie<-0.5)
 {
	 sortie=-0.5;
 }
sortie_preced=sortie;
erreur_volt_preced=erreur_volt;
rapport_cyc=(sortie+0.5)*4096;
R_Cyc_1((int)rapport_cyc);
R_Cyc_2((int)rapport_cyc);
}

