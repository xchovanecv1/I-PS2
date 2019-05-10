# PS2 - ZADANIE

## Pokyny k odovzdaniu

Odovzdajte projekt t.j. c++ kod so vsetkymi kniznicami ak boli navyse (napr. *.h alebo *.cc) popripade aj so spustacimi subormi bash, alebo gnuplot konfiguratorom ak ho projekt nevytvara.

nutne parametre prikazovom riadku s jednotnymi nazvami: 
- --simulTime cas trvania simulacie (double)
- --anim vytvorenie animacie (boolean)
- a dalsie vhodne podla projektu

ak sa zmestia obrazky a animacia vlozte aj tu


## Dokumentacia zadania

_Smerovaci protokol_:

OLSR:
- Nas pripad pouzitia vyzaduje vysoku mobilitu uzlov v sieti (kde je vhodnejsie pouzit OLSR protokol) a proaktivny pristup k vyhladavaniu trasy v sieti.
- Vyhladanie cesty medzi uzlami pri pouziti AODV trva dlhsie, ako pri OLSR -> nas pripad pouzitia vyzaduje rychlu komunikaciu, s vzdy aktualnou informaciou o topologii siete 
- OLSR -> okamzite najdenie cesty pomocou smerovacej tabulky, AODV-> inicializacia procesu objavenia trasy zahlcuje siet
- Nove trasy dokaze OLSR protokol zistit ihned po nahlaseni prerusenia povodnej trasy (narozdiel od AODV protokolu)
- Pri OLSR je oneskorenie prenosu paketu v sieti relativne male, v porovnani s rout request protokolom (ako napr AODV) 

_Transformny protokol_:

***UDP:
- pointou nasho systemu je vymienanie informacii medzi vsetkymi uzlami v sieti  (broadcast), resp. medzi najblizsimi susedmi
- z tohto dovodu je v nasom systeme vhodne (miestami az nutne) pouzit UDP, taktiez pozadujeme rychlost prenosu, bez nutnosti potvrdenia o prijati odoslanych paketov (nakolko odosielame viacerym uzlom)
- Neaktualnu spravu je v nasom pripade mozne zahodit

## TODO

_Grafy:_
- Troughput od vzdialenosti
- Qos od vzdialenosti 
- Troughput od data
- Nejake callbacks v case

- Pocet stratenych nodov od intervalu hello paketov
_QOS:_
- pomer prijatých paketov k odoslaným, 
- časový interval medzi prijatím a odoslaním dátového paketu 
- priepustnosť (troughput): množstvo dát úspešne poslaných od odosielateľa k príjemcovi (bits per second).

_L2-L5 zmeny:_
OLSR spravit
- callback na routing table, zistovanie, ci poznam vsetky uzly 
- Ked sa uzlik strati, nech ide do stredu mapy

_Wifimanager preco?:_
- dopisat
