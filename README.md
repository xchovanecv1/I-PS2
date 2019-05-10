# PS2 - ZADANIE

## Pokyny k odovzdaniu

Odovzdajte projekt t.j. c++ kod so vsetkymi kniznicami ak boli navyse (napr. *.h alebo *.cc) popripade aj so spustacimi subormi bash, alebo gnuplot konfiguratorom ak ho projekt nevytvara.

nutne parametre prikazovom riadku s jednotnymi nazvami: 
- --simulTime cas trvania simulacie (double)
- --anim vytvorenie animacie (boolean)
- a dalsie vhodne podla projektu

ak sa zmestia obrazky a animacia vlozte aj tu


## Dokumentacia zadania
Zadanie simuluje drony hliadkujuce loziska poziaru v Australskych lesoch. Jednotlive drony su rozmiestene do grid usporiadania. Vsetky drony maju spociatku nastaveny nahodny pohyb v okoli ich pociatku. V pripade, ako dron najde podozrivu oblast, privolava svojich najblizsich susedov z okolia k sebe. Taktiez si vymienaju telemetricke udaje vo forme zostavajucej baterie, ktorej hodnotu posielaju svojim susedom. V zadani je taktiez implementovany vlastny broadcast protokol, pre odosielanie udajov vsetkym nodom.

Implementacia vlastneho broadcast protokolu
- Pomocou OLSR routovacej tabulky si vyhladame susedov, ktory su klasifikovany vo vzdalenosti 1 hopu.
- Kazdy datovy paket obsahuje informacie:
  * cislo odosielajuceho nodu:cislo spravy:data
  * cislo odosielatela: IDcko daneho nodku ktory vyslal danu spravu.
  * cislo spravy: poradove cislo danej spravy pre dany node
- Protokol:
  * kazdy node si drzi posledne ID spravy, ktoru prijal od jedntlivych nodov
  * ak prijmem spravu, ktorej ID je vacsie ako posledne zname ID, spracujem data a preposieam vsetkym mojim susedom
  * ak je ID danej spravy mensie alebo rovnake ako aktualne, dana sprava je zahodena, pretoze prisla od ineho nodu, ktoremu som danu spravu poslal ja

## Zadanie

- Vizualizácia: NetAnim 2b (pozn. využite cmd-line-arguments)
  - --anim=true
- Reprezentácia merania: Tri grafy (pozn. minimálne 10 bodov merania s vyhodnotením, t.j. odchýlky merania) 3b + zhodnotenie grafu (prečo je to taká závislosť) 2b
  - prvy
  - druhy
  - treti
Popis merania:
### vhodný vyber ISO OSI 4b
_Smerovaci protokol_:

***OLSR***:
- Nas pripad pouzitia vyzaduje vysoku mobilitu uzlov v sieti (kde je vhodnejsie pouzit OLSR protokol) a proaktivny pristup k vyhladavaniu trasy v sieti.
- Vyhladanie cesty medzi uzlami pri pouziti AODV trva dlhsie, ako pri OLSR -> nas pripad pouzitia vyzaduje rychlu komunikaciu, s vzdy aktualnou informaciou o topologii siete 
- OLSR -> okamzite najdenie cesty pomocou smerovacej tabulky, AODV-> inicializacia procesu objavenia trasy zahlcuje siet
- Nove trasy dokaze OLSR protokol zistit ihned po nahlaseni prerusenia povodnej trasy (narozdiel od AODV protokolu)
- Pri OLSR je oneskorenie prenosu paketu v sieti relativne male, v porovnani s rout request protokolom (ako napr AODV) 

_Transformny protokol_:

***UDP***:
- pointou nasho systemu je vymienanie informacii medzi vsetkymi uzlami v sieti  (broadcast), resp. medzi najblizsimi susedmi
- z tohto dovodu je v nasom systeme vhodne (miestami az nutne) pouzit UDP, taktiez pozadujeme rychlost prenosu, bez nutnosti potvrdenia o prijati odoslanych paketov (nakolko odosielame viacerym uzlom)
- Neaktualnu spravu je v nasom pripade mozne zahodit

### volanie časových udalostí  2b
### volanie udalostí zmenu stavu (atributu modelu) 2b
### zmena v modelu L1  fyzické médium, pohyb, útlm … 3b
### zmena v modelu L2-L5 2b
