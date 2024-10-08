## Farcasanu Bianca Ioana 313CA 2023-2024

# CACHE
Pentru aceasta prima parte a programului am adaugat functiile (operatiile)
specifice pentru prelucrarea listelor dublu inlantuite si hashmap-ului intrucat
le voi folosi mai tarziu.

## Functii
    	Primele functii sunt cele clasice pentru prelucrarea listelor dublu
	inlantuite: crearea, adaugarea pe pozitia n a unui nod, stergerea de pe
	pozitia n, returnarea unui nod de pe o anumita pozitie, cat si eliberarea
	unei astfel de liste. Toate lucreaza cu nodurile si modifica conexiunile
	corespunzator, lista avand atat head, cat si tail.
    	Urmatoarele functii sunt cele clasice pentru hashmap : functii de
	comparare a cheilor, crearea unui hashtable, verificarea prezentei unei
	chei, obtinerea valorii de la o anumita cheie, introducerea si eliminarea
	din ht, dar si eliberarea memoriei pentru intreaga structura.
    	Structurile au fost introduse in lru_cache.h .

## INIT_LRU_CACHE
		Aceasta functie creeaza si returneaza un cache, cu toate structurile
	incluse in el. Creeaza si aloca memorie pentru fiecare in parte:

	~ lista dublu inlantuita (order_list) ce va contine atat numele
	documentului, cat si continutul acestuia intr-un nod in ordinea impusa :
	documentele prelucrate cel mai recent la final, iar cele mai vechi spre
	inceput

	~ hashtable in care se vor afla : cheia (numele documentului), iar valoarea
	va fi pointer catre nodul corespunzator din order_list pentru a putea
	modifica si accesa mai rapid.

## LRU_CACE_IS_FULL
		Verific prin aceasta functie daca cache-ul , mai exact hashtable-ul,
	a atins limita maxima (daca este plin sau nu), campul size din ht fiind
	cel care retine numarul total de noduri din ht, iar hmax cel maxim.

## FREE_ORDER_LIST
		Am creat aceasta functie pentru a elibera memoria ocupata de order_list
	intrucat functia generica dll_free nu elibera si continutul fiecarui camp
	din list->data.

## FREE_LRU_CACHE
		Aceasta functie elibereaza memoria totala a cache-ului si a fiecarei
	structuri din acesta.

## LRU_CACHE_PUT
		Aceasta functie adauga un nod in cache, urmand anumite reguli: incep
	prin a verifica daca cache-ul este full, caz in care va trebui sa elimin
	din order_list cea mai veche accesare (nodul de pe pozitia 0 fiind cel mai
	vechi), sa eliberez memoria pentru nodul eliminat si mai apoi sa elimin
	intrarea din ht.
		Pentru a elimina din hashtable am modificat functia
	'key_val_free_function' astfel incat sa eliberez doar memoria ocupata de
	cheie intrucat valoarea este un pointer catre nodul pe care l-am eliberat
	deja anterior. Dupa aceea, chiar daca cache-ul era full sau nu, mi-am facut
	loc pentru a adauga o noua intrare. Adaug mai intai in order_list, creand
	nodul in prealabil, dupa care adaug in ht, valoarea fiind nodul creat
	anterior, adica tail-ul din order_list.

## LRU_CACHE_GET
		Aceasta functie returneaza valoarea din ht de la o anumita cheie,
	adica pointer catre nodul din order_list prin prisma functiei ajutatoare
	ht_get. Valoarea e NULL daca nu se gaseste in ht.

## LRU_CACHE_REMOVE
		Aceasta functie elimina intrarea din cache de la o anumita cheie.
	Sterg prima data din order_list si elimin memoria pentru nodul eliminat,
	iar apoi sterg din ht, eliberand memoria doar pentru cheie intrucat
	valoarea (pointer la nodul din order_list) a fost deja eliberata.

# SERVER
Pentru aceasta parte a programului am introdus functiile specifice pentru
prelucrarea cozii (queue) intrucat voi avea nevoie de ele pentru retinerea
task-urilor pe care va trebui sa le aplic pe un document : crearea unei cozi
si alocarea de memorie pentru aceasta si fiecarui camp din aceasta (care va
fi de tip request),  verificarea daca coada este plina sau nu, returnarea
primului element din coada (front), eliminiarea si adaugarea unui element in
coada, cat si eliberarea cozii si a tuturor campurilor sale.

## UPDATE_CONTENT_IN_EDIT
		Aceasta este o functie ajutatoare folosita in server_edit_document
	pentru cazul in care documentul se afla in cache. Functia elimina din
	order_list vechiul nod pentru a actualiza continutul documentului si
	pentru a-l muta pe ultima pozitie intrucat este editat recent, elibereaza
	memoria pentru acesta si creeaza un nod nou pe care-l adauga la finalul
	order_list. Modifica, de asemenea, si in hashtable valoarea de la cheia
	respectiva (numele documentului) facand sa pointeze catre nodul nou
	adaugat in order_list (tail) si, deopotriva, modifica si in baza de date
	continutul cheii.

## SERVER-EDIT_DOCUMENT
		Aceasta functie realizeaza editarea unui document, avand mai multe
	cazuri dupa care trebuie sa ne ghidam (am urmat organigrama din enuntul
	temei) :
		~ daca documentul se afla in cache printez raspunsul corespunzator
	si actualizez continutul din cache cu noul continut si toate modificarile
	necesare prin functia 'update_content_in_edit' .
		~ daca documentul nu se afla in cache, incep sa caut in baza de date
		~ daca il gasesc in baza de date primul lucru pe care il fac este sa
	adaug documentul cu noul continut in cache (folosinf lru_cache_get care ia
	in calcul si cazul in care cache-ul este full, adica exista evicted_key),
	dupa care ma ocup sa actualizez continutul in baza de date, printand
	raspunsul corespunzator in functie de existenta lui evicted_key.
		~ daca documentul nu se gaseste nici in baza de date, il creez si il
	adaug atat in baza de date, cat si in cache.

## UPDATE_ORDER_CACHE_IN_GET
		Utilizez aceasta functie pentru a actualiza ordinea din cache : sterg
	nodul de la indexul trecut din order_list, il adaug la final, dupa care
	actualizez pointerul din ht fiindca in dll_add_nth_node a fost creat un
	nou nod in care doar s-a copiat data din vechiul nod, deci si pointerul
	s-a modificat.

## SERVER_GET_DOCUMENT
		Aceasta functie seamana cu cea de editare, singura diferenta fiind
	ca nu mai trebuie modificat continutul documentului, ci doar actualizate
	pozitiile in toata cazurile :
		~ daca documentul se afla in cache (verific daca se afla cu functia
	'lru_cache_get' care intoarce valoarea de la o anumita cheie, iar daca e
	diferit de NULL, inseamna ca acel nod exista) , printez raspunsul
	corespuznator si actualizez ordinea cu functia explicata mai sus
	'update_order_cache_in_get'.
		~ daca documentul nu e in cache, caut in database
		~ daca este in baza de date, printez raspunsul si il pun cache cu
	functia 'lru_cache_put' (care ia in considerare cazul in care cahe-ul
	este full, adica exista evicted_key) intrucat este accesat recent.
		~ daca documentul nu e nici in baza de date, afisez raspunsul
	corespunzator.

## INIT_SERVER
		Aceasta functie creeaza si aloca memorie pentru toate structurile
	de date folosite pentru realizarea unui server folosind functiile
	auxiliare specifice.

## SERVER_HANDLE_REQUEST
		Aceasta functie realizeaza aplicarea functiei de edit sau get,
	in functie de tipul requestului primit ca parametru. Daca request-ul
	este de tipul 'EDIT', printez raspunsul potrivit si adaug in coada de
	task-uri din server edit-urile pe care le voi aplica in momentul in
	care primesc get pentru acel document.
		Daca request-ul este de tipul 'GET' aplic toate edit-urile memorate
	in coada de task-uri pe documentul cerut printr-un while pana cand coada
	devinde goala, dupa care folosesc 'server_get_document' pentru a obtine
	raspunsul corespunzator documentului modificat complet.

## FREE_REQUEST_QUEUE & FREE_DATABASE
		Aceste functii le-am creat intrucat continutul fiecarui nod din coada,
	respectiv lista era unul diferit care necesita eliberarea memoriei
	separat, deci nu ma puteam folosi de cele generice deja implementate
	(sau daca alegeam sa ma folosesc de cele generice ar fi trebuit sa parcurg
	inca o data inante toata lista, respectiva coada, pentru e elibera fiecare
	continut din nod si dupa sa apelez functia generica, dar as fi parcurs de
	2 ori inutil).

## FREE_SERVER
		Eliberez prin aceasta functie toata memoria unui server si a tuturor
	structurilor din interiorul server-ului. Totul se termina cu bine :)
