clear all;
close all;

Rm   = 1;
Km   = 1/Rm;
Lm   = 2*10^(-3);
Taum = Lm/Rm;

R5  = 5100;
R8  = 10000;
R12 = 10000;
R18 = 12000;
R21 = 220;

C2 = 22*10^(-9);
C7 = 22*10^(-9);

Ubat = 24;
Tau1 = ((R8*R5)/(R8+R5))*C2;
Tau2 = R21*C7;

Kf = (R12+R8)/(R5+R8);
Kg = 2*Km*Ubat;

G_p=tf(Kg,[Taum 1]);
%bode(G_p)
f_p1 = tf(Kf,[Tau1 1]);
%bode(f_p1)
f_p2 = tf(1 ,[Tau2 1]);
%bode(f_p2)
f_p_intermediaire = f_p2*f_p1;
f_p  = zpk(f_p_intermediaire);
tauc1= Taum;
tauc2= 0.00268;
Cp   = tf([tauc1 1],[tauc2 0]);
FTBO=0.108*G_p*f_p*Cp;
bode(FTBO)%bode boucle ouverte
FTBF=FTBO/(1+FTBO);
%bode(FTBF)%bode boucle ferme

Te = 200*10^(-6);
c_pz = c2d(Cp,Te,'tustin');
%step(Cp,'--',c_pz,'-')