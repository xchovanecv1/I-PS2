# PS2 - Počítačové siete 2, ZADANIE

Smerovaci protokol:

OLSR:
- Nas pripad pouzitia vyzaduje vysoku mobilitu uzlov v sieti (kde je vhodnejsie pouzit OLSR protokol) a proaktivny pristup k vyhladavaniu trasy v sieti.
- Vyhladanie cesty medzi uzlami pri pouziti AODV trva dlhsie, ako pri OLSR -> nas pripad pouzitia vyzaduje rychlu komunikaciu, s vzdy aktualnou informaciou o topologii siete 
- OLSR -> okamzite najdenie cesty pomocou smerovacej tabulky, AODV-> inicializacia procesu objavenia trasy zahlcuje siet
- Nove trasy dokaze OLSR protokol zistit ihned po nahlaseni prerusenia povodnej trasy (narozdiel od AODV protokolu)
- Pri OLSR je oneskorenie prenosu paketu v sieti relativne male, v porovnani s rout request protokolom (ako napr AODV) 

Transformny protokol:

***UDP:
- pointou nasho systemu je vymienanie informacii medzi vsetkymi uzlami v sieti  (broadcast), resp. medzi najblizsimi susedmi
- z tohto dovodu je v nasom systeme vhodne (miestami az nutne) pouzit UDP, taktiez pozadujeme rychlost prenosu, bez nutnosti potvrdenia o prijati odoslanych paketov (nakolko odosielame viacerym uzlom)
- Neaktualnu spravu je v nasom pripade mozne zahodit

grafy
- Troughput od vzdialenosti
- Qos od vzdialenosti 
- Troughput od data
- Nejake callbacks v case

qos
- pomer prijatých paketov k odoslaným, 
- časový interval medzi prijatím a odoslaním dátového paketu 
- priepustnosť (troughput): množstvo dát úspešne poslaných od odosielateľa k príjemcovi (bits per second).

L2-L5 zmeny
OLSR spravit
- callback na routing table, zistovanie, ci poznam vsetky uzly 
- Ked sa uzlik strati, nech ide do stredu mapy

Wifimanager preco?
- dopisat
