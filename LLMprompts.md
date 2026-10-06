## Utilizare LLM (Prompturi)

Unealta folosita: Gemini (versiunea Advanced)

### Interactiunea 1: Alegerea functiei BLAS
**Intrebare:**
bunaa, ce functie din blas pot sa folosesc ca sa inmultesc o matrice cu 
transpusa ei? stiu ca rezultatul o sa fie o matrice simetrica si cica 
pot sa fac asta mai repede decat cu inmultirea normala.

**Raspuns:**
Buna! Cea mai buna varianta pentru tine este sa folosesti `cblas_dsyrk`. 
Functia asta face exact ce ai nevoie: o operatie care returneaza direct o 
matrice simetrica. E mult mai eficienta decat varianta standard de inmultire 
pentru ca scrie si calculeaza doar jumatate din elemente (de exemplu, doar 
triunghiul superior al matricei, daca ii dai parametrul aferent).

### Interactiunea 2: Rularea pe cluster
**Intrebare:**
cum rulez codul pe partitia haswell ca ma cam bate enuntul. imi zice de 
un apptainer si de srun, cum dau comanda exact ca sa intru sa rulez?

**Raspuns:**
Pentru a rula interactiv pe un nod Haswell intr-un container, ai de facut
doi pasi simpli. Mai intai ceri un shell pe nod cu srun:
`srun -p haswell --pty bash`
Dupa ce ai ajuns pe nodul haswell-wnXX, deschizi containerul:
`apptainer shell optimizari.sif`
Acum poti rula executabilele tale cu `./tema1_neopt input` fara probleme.

### Interactiunea 3: Optimizari de mana
**Intrebare:**
trebuie sa scriu o varianta optimizata de mana la inmultirea matricelor. 
am tot auzit de loop unrolling si block matrix pe la laboratoare. poti sa 
imi explici scurt cum vin astea puse in cod?

**Raspuns:**
Sigur! 
1. Block Matrix Multiplication: In loc sa faci for-urile pe toata linia de N 
elemente, imparti operatia in patratele (blocuri, ex. 40x40). Faci inmultirile
doar in interiorul blocului. Asta tine datele calde in memoria Cache L1.
2. Loop Unrolling: In ultimul `for` (cel mai din interior), in loc sa cresti
cu 1 (`j++`), cresti cu 4 (`j+=4`) si scrii operatia de 4 ori una sub alta.
Asta face procesorul sa evalueze de mai putine ori conditia buclei.

### Interactiunea 4: Eroare Valgrind
**Intrebare:**
imi da definitely lost in valgrind desi m am asigurat ca am dat free la 
matricele C si D la finalul functiei my_solver. de ce tot imi zice ca
pierd memorie?

**Raspuns:**
Daca matricile principale sunt eliberate corect, cel mai probabil pierzi 
memorie de la un vector intermediar creat pe parcurs. Verifica daca nu cumva 
ai alocat dinamic un vector auxiliar (de exemplu, pentru a face suma de pe 
linii) caruia ai uitat sa ii dai `free` la final.

### Interactiunea 5: Grafice Python
**Intrebare:**
da mi si mie un exemplu super simplu de cum fac cu matplotlib un grafic 
cu niste bare paralele. am timpii pentru cele 3 variante si vreau 
sa le pun asa una langa alta sa se vada clar diferenta.

**Raspuns:**
Poti folosi pachetul `matplotlib.pyplot`. Ca sa pui barele fix una langa
alta, folosesti functia de plotare cu bare si setezi un offset pe axa X 
(un "width"). Practic desenezi prima bara la o pozitie `x - width`, pe 
a doua la `x` si pe a treia la `x + width`, iar asa vor aparea grupate.


### Interactiunea 6: Explicatii text pentru README
**Intrebare:**
zi mi te rog in 2-3 propozitii scurte de ce loop unrolling ul mi a stricat 
branch prediction ul in opt_m fata de neopt, ca sa stiu cum formulez faza
asta in readme la partea de cachegrind.

**Raspuns:**
Loop unrolling-ul scade numarul total de iteratii ale buclei. Din acest motiv,
branch predictor-ul din procesor are mai putine oportunitati sa "invete" 
tiparul de salt al for-ului. Chiar daca rata procentuala de mispredict creste, 
numarul absolut de greseli si de salturi este mult mai mic, deci per total 
timpul de executie scade.