# Tema 1 ASC - Optimizarea Inmultirii Matricelor

## Descriere Generala
Acest proiect contine trei implementari diferite pentru rezolvarea unei ecuatii
cu matrice si vectori, avand scopul de a evidentia diferentele de performanta
dintre un cod scris de mana neoptimizat, unul scris folosind biblioteca BLAS si
unul optimizat manual prin tehnici de imbunatatire a accesului la memorie.

Ecuatia implementata este:
C = At * B
D = C * Ct
y = D * (suma pe linii a lui C) + x

## Explicatii Implementari

### 1. Varianta Neoptimizata (solver_neopt.c)
Aceasta este implementarea de referinta. 
Pentru prima inmultire, C = At * B, am calculat elementele direct folosind 3
bucle for. Pentru a evita transpunerea fizica a matricei A in memorie, am tinut
cont de transpusa modificand indicii de acces din A[i][k] in A[k][i] (adica
A[k * N + i] liniarizat).

Pentru calculul matricei D = C * Ct, m-am folosit de cerinta din enunt care
precizeaza ca D este o matrice simetrica. Astfel, am calculat doar elementele
de pe diagonala principala si cele de deasupra ei (j <= i), dupa care am
copiat rezultatul in pozitia simetrica D[j * N + i] = intermediate_sum. Asta a
injumatatit numarul de calcule necesare pentru matricea D.

Apoi am calculat sumele pe liniile matricei C intr-un vector separat, am
inmultit D cu acest vector si la final am adunat vectorul x.

### 2. Varianta BLAS (solver_blas.c)
Pentru aceasta varianta am folosit functiile din biblioteca cblas pentru o
eficienta maxima.
- Alocarile le-am facut cu calloc.
- Pentru C = At * B am folosit cblas_dgemm. Am setat parametrul TransA pe
  CblasTrans pentru a inmulti direct pe A transpusa, fara sa o transpun eu in
  prealabil.
- Pentru D = C * Ct am apelat cblas_dsyrk (Symmetric rank-k update). Este o
  functie super optimizata care tine cont automat de faptul ca matricea
  rezultata D este simetrica, calculand si scriind doar in partea superioara
  a ei (CblasUpper).
- Am insumat liniile lui C intr-un vector intermediar v folosind repetat
  cblas_daxpy.
- Am copiat vectorul x in y cu cblas_dcopy pentru a avea baza rezultatului.
- La final, am inmultit matricea simetrica D cu vectorul obtinut si l-am
  adunat in y folosind cblas_dsymv, profitand din nou de simetria lui D.

### 3. Varianta Optimizata manual (solver_opt.c)
Aceasta varianta imbunatateste varianta neopt exclusiv prin modificari la
nivel de cod. Pentru a creste performanta, am aplicat urmatoarele tehnici:
- Block Matrix Multiplication (BMM): Am impartit inmultirea matricelor in
  blocuri de dimensiune 40x40 (B_SIZE = 40). Asta creste enorm localitatea
  spatiala si temporala a datelor in memoria cache L1. In loc sa se aduca
  date noi la fiecare iteratie pentru toata matricea N, elementele sunt
  tinute in cache cat timp se lucreaza pe blocul respectiv.
- Loop Unrolling: In buclele interioare am grupat pasii cate 4 (j += 4).
  Asta reduce numarul de salturi (branches) evaluate de procesor si permite
  executarea mai multor instructiuni aritmetice in paralel.
- Keyword-ul register si pointeri: Am precalculat adresele de inceput de rand
  folosind pointeri locali (ex: c_row = &C[i * N]), evitand inmultirile
  costisitoare la fiecare pas din bucla. Variabilele foarte folosite au fost
  declarate cu register pentru a sugera compilatorului sa le tina in
  registrii procesorului, scazand numarul de accesari la memorie.

## Analiza Performantei (Valgrind Cachegrind si Memcheck)

Din rularea Valgrind Memcheck pe input_valgrind reiese ca toate cele 3
variante elibereaza complet memoria alocata (0 errors, 0 leaks), fara
probleme de acces (segfault-uri sau citiri neinitializate).

Analizand output-urile de la Cachegrind, diferentele sunt majore:

1. Numarul de instructiuni (I refs):
- Neopt ruleaza in jur de 3.55 miliarde de instructiuni.
- Opt_m reduce acest numar la 1.40 miliarde. Loop unrolling-ul si pointerii
  calculati in prealabil au scazut instructiunile necesare la mai putin
  de jumatate.
- BLAS foloseste doar 83 de milioane, demonstrand avantajul instructiunilor
  vectoriale din biblioteca precompilata.

2. Accese la date si miss rate (D1 misses):
- Neopt are un miss rate in cache-ul L1 de date (D1 miss rate) destul de
  mare, de 6.7% (aprox. 132 milioane misses absoulte). Accesarea pe coloane
  face ca datele din cache sa se invalideze rapid.
- Opt_m scade acest miss rate spectaculos, la doar 0.2% (doar 781 mii misses).
  Impartirea in blocuri de 40x40 garanteaza ca datele incap in cache-ul L1
  si sunt refolosite eficient inainte sa fie inlocuite.

3. Branches:
- Datorita unrolling-ului, numarul de branches in Opt_m a scazut masiv fata
  de Neopt (20 milioane fata de 97 milioane). Chiar daca mispredict rate-ul
  a crescut la 8% pe Opt_m, numarul total mult mai mic de branches si de
  instructiuni rezulta intr-un timp de executie mult mai bun.

## Bonus: Analiza comparativa Haswell vs. UCSX

Comparand rularea Opt_m pe Haswell fata de UCSX din fisierele cache generate:
- Pe Haswell, varianta Opt_m a avut 781,524 D1 misses.
- Pe UCSX, aceeasi varianta a avut 705,909 D1 misses (un miss rate usor
  imbunatatit, de la 0.18% la 0.14%).
Acest lucru sugereaza o arhitectura de cache superioara pe nodurile UCSX
(fie cache L1 mai mare, fie politici de prefetching mai agresive care
favorizeaza accesul blocat). Numarul de instructiuni (I refs) ramane practic
identic intre cele doua arhitecturi (~1.4 miliarde), diferenta de
performanta fiind data strict de ierarhia de memorie.


## Grafice si Timpi de Executie

Am rulat testele pe mai multe dimensiuni (N intre 400 si 1800) ca sa vad cum
se comporta variantele. Iata ce a reiesit din cele 13 grafice:

### 1. Performanta generala si scalabilitatea
- **timpi_neopt_blas_opt.png**: Aici se vede cat de lenta e varianta neopt 
  (ajunge la ~13.6s pt N=1200). Codul meu (opt_m) reduce timpul la vreo 3.5s,
  in timp ce BLAS termina totul instant in 0.17s.
- **scalare_opt_m.png**: M-am uitat doar la opt_m pana la N=1800. Se 
  observa clar forma de O(N^3) a algoritmului, timpul crescand progresiv de 
  la 0.15s la 11.6s pentru cea mai mare matrice.

### 2. Analiza memoriei si a instructiunilor (Cachegrind)
- **acces_memorie_drefs.png**: Neopt face enorm de multe accese la memorie 
  (aproape 2 miliarde pt N=400). Folosind variabile register si pointeri in 
  opt_m, am reusit sa scad numarul asta de vreo 4 ori (la 471M).
- **misses_l1_l3_variante.png**: Avand mult mai putine accese, automat si 
  rateurile in L1 au scazut imens: de la 132 de milioane in neopt, la sub 
  un milion in opt_m.
- **rate_miss_si_branch.png**: Graficul asta arata exact compromisul facut
  in opt_m. Rata de L1 miss scade dramatic de la 6.7% la 0.2%, dar din cauza
  unrolling-ului am stricat putin branch prediction-ul (a crescut de la 0.3%
  la 8%). Per total, a meritat clar sacrificiul.

### 3. Bonus: Comparatie arhitecturi (Haswell vs UCSX) - Timpi
Am rulat testele si pe coada UCSX ca sa vad diferenta de hardware.

- **arhitecturi_timpi_global.png**: Reprezentarea cu linii arata clar cum 
  toate variantele sunt mai rapide pe UCSX, indiferent de marimea matricei.
- **arhitecturi_grupate_n.png**: Forma cu bare evidentiaza diferenta uriasa
  la teste mari. La N=1800, neopt scade de la ~50s pe Haswell la ~37s pe UCSX.
- **optimizat_haswell_vs_ucsx.png**: Acelasi avantaj se pastreaza si la
  varianta mea optimizata (8.6s pe UCSX vs 11.9s pe Haswell pentru N=1800).
- **speedup_ucsx_opt_m.png**: Graficul arata de cate ori e mai rapid UCSX
  fata de Haswell pe codul optimizat. De la N=1200 in sus, treaba se
  stabilizeaza la un speedup de cam 1.35x - 1.39x.

### 4. Bonus: Comparatie arhitecturi - Cache si Branch-uri
- **drefs_haswell_ucsx.png**: Confirma ca numarul total de referinte la 
  memorie e absolut identic intre masini (logic, e fix acelasi cod rulat).
- **misses_arhitecturi_n400.png**: Chiar daca fac aceleasi accese, acest 
  grafic arata ca per total (L1 si L3), nodul UCSX inregistreaza un numar 
  mai mic de rateuri.
- **d1_miss_comparatie_noduri.png**: Facand zoom strict pe memoria L1, se 
  vede clar ca UCSX are un prefetcher mai bun. Pentru opt_m avem 705k 
  rateuri pe UCSX fata de 781k pe Haswell.
- **rate_comparatie_arhitecturi.png**: Super interesant aici: branch 
  predictorul greseste fix la fel pe ambele (8% la opt_m). In schimb, UCSX
  absoarbe mai bine accesele urate din neopt, scazand miss rate-ul de la 
  6.7% la 5.4%.

