#include "../Comportamientos_Jugador/jugador.hpp"
#include "motorlib/util.h"

#include <iostream>
#include <cmath>
#include <set>

#include <stack>
#include <queue>
#include <cstring>
		


using namespace std;

/*
	Función para calcular la distancia teórica hasta un destino
*/
double calcularDistancia(int fila, int columna, int d_fila, int d_col){
	return (abs(d_fila-fila)+abs(d_col-columna));
}

/*
	Función que compara dos estados y dice si son iguales, es decir, si tienen la misma columna y misma fila.
*/
struct ComparaEstados{
	bool operator()(const estado &a, const estado &n) const{
		if ((a.fila > n.fila) or (a.fila == n.fila and a.columna > n.columna) or
	      (a.fila == n.fila and a.columna == n.columna and a.orientacion > n.orientacion))
			return true;
		else
			return false;
	}
};




int num_aldeano = 0;
bool busqueda_completa = false;
int umbral_bateria = 200;
int umbral_tiempo = 300;
set<estado, ComparaEstados> set_baterias;
estado estado_zapatillas = {-1,0,0};
estado estado_bikini = {-1,0,0};
int umbral_pensando = 60;
int tiempo_simulacion = 3000;
set <estado, ComparaEstados> posiciones_conocidas;
int tiempo_gastado = 0;

/*
	Función para buscar una batería que hayamos visto previamente. Devuelve la fila y columna de la batería más cercana. Tenemos que asegurarnos 
	antes de que conocemos dónde está al menos una batería
*/
estado buscarBateria(int fila_actual, int col_actual);




/*
	Método principal que debe contener los 4 Comportamientos_Jugador
	que se piden en la práctica. Tiene como entrada la información de los
	sensores y devuelve la acción a realizar.
*/
Action ComportamientoJugador::think(Sensores sensores) {
	
	Action accion = actIDLE;
  // Estoy en el nivel 1
  if (sensores.nivel != 4){
      if( !hayplan) {
          actual.fila        = sensores.posF;
          actual.columna     = sensores.posC;
          actual.orientacion = sensores.sentido;

        cout << "Fila: " << actual.fila << endl;
        cout << "Col : " << actual.columna << endl;
        cout << "Ori : " << actual.orientacion << endl;

        destino.fila       = sensores.destinoF;
        destino.columna    = sensores.destinoC;

        hayplan = pathFinding (sensores.nivel, actual, destino, plan);
      }
	 
      if( hayplan and plan.size() > 0) {
		
        accion = plan.front();
		
        plan.erase(plan.begin());
      }
      else {
		
        // aquí entraría si no se ha encontrado un comportamiento o está mal implementado
        cout << "Plan mal implementado o no se ha encontrado " << endl;

      }
       
    }   
  else { // Estoy en el nivel 2
		actual.fila        = sensores.posF;
		actual.columna     = sensores.posC;
		actual.orientacion = sensores.sentido;
		destino.fila       = sensores.destinoF;
		destino.columna    = sensores.destinoC;

		// vemos si gasta demasiado tiempo pensando para llevar a cabo una estrategia u otra
	  	tiempo_gastado = sensores.tiempo - tiempo_gastado;
		if (tiempo_gastado > umbral_pensando)
			busqueda_completa = false;
		else
			busqueda_completa = true;
	
		//vemos la distancia hasta la bateria mas cercana (sup valor de casilla = 2) y redefinimos el valor umbral de la batería
		if (set_baterias.size()>0){
			estado prov = buscarBateria(actual.fila, actual.columna);
			umbral_bateria = calcularDistancia(prov.fila, prov.columna, actual.fila, actual.columna)*2 +200;
		}

		// si no tenemos las zapatillas o el bikini y sabemos donde estan vamos a cogerlas
		if (!zapatillas and estado_zapatillas.fila!= -1){
			destino.fila = estado_zapatillas.fila;
			destino.columna = estado_zapatillas.columna;
		}
		if (!bikini and estado_bikini.fila!= -1){
			destino.fila = estado_bikini.fila;
			destino.columna = estado_bikini.columna;
		}

		// si tenemos una batería cerca y la batería la tenemos peor que el tiempo de simulación recargamos
		if (bateriaCerca(sensores) && (bateria < tiempo_simulacion) ){
			destino = buscarBateria(actual.fila, actual.columna);
		}
		

		// si no tenemos plan:
    	if (plan.empty()){
			buscarPlan(sensores);
		}


		// si tenemos plan
		if( hayplan and plan.size() > 0) { 
			bool no_accion = true;
			// si estamos en una casilla de recarga
			if ((mapaResultado[sensores.posF][sensores.posC] == 'X')){
				no_accion = false;			
				if (bateria < tiempo_simulacion)  // si tenemos menos bateria que tiempo de simulación nos quedamos en recarga
					accion = actIDLE;
				else 
					no_accion = true;  // si ya hemos equiparado la bateria y el tiempo de simulación llevamos a cabo el plan
			}
			if(no_accion){
				accion = plan.front();
				plan.erase(plan.begin());
				
				switch(accion){
					case 0:  // si vamos a avanzar vemos si conocemos la casilla que tenemos delante
						if(ampliarHorizonte(sensores)){ // si no la conocemos actualizamos mapaResultado, borramos el plan y calculamos otro
							rellenarMatriz(sensores);
							plan.clear();
							actual.fila        = sensores.posF;
							actual.columna     = sensores.posC;
							actual.orientacion = sensores.sentido;
							buscarPlan(sensores);
							accion = plan.front();
							plan.erase(plan.begin());
						}
						
						// si nos encontramos con un aldeano esperamos 2 tiempos, si no podemos avanzar recalculamos el plan
						if (sensores.superficie[2] == 'a'){
							if (num_aldeano < 2){
								plan.push_front(accion);
								accion = actIDLE;
								num_aldeano++;
							}
							else{
								accion = actIDLE;
								plan.clear();
								num_aldeano = 0;
							}
						}
					
						
						
					break;

				
				}
			
			}	
		}
		
		
  }
  bateria = sensores.bateria;
  cambiarEstado(sensores.posF, sensores.posC);
  
  tiempo_simulacion--;
  return accion;
}


estado buscarBateria(int fila_actual, int col_actual){
	
	double distancia;
	posicion provisional;
	multiset<posicion, ComparaDistancia> baterias_actuales;
		
	for (std::set<estado, ComparaEstados>::iterator it=set_baterias.begin(); it!=set_baterias.end(); ++it){
		distancia = calcularDistancia(fila_actual, col_actual, (*it).fila, (*it).columna);
		provisional.distancia = distancia;
		provisional.fila = (*it).fila;
		provisional.columna = (*it).columna;
		baterias_actuales.insert(provisional);
	}

	int fila = (*baterias_actuales.begin()).fila;
	int columna = (*baterias_actuales.begin()).columna;
	return {fila, columna,-1};
}

/*
	Función que busca un plan para llevar a cabo. 

*/
void ComportamientoJugador::buscarPlan(Sensores sensores){
	// si estamos en una casilla de recarga y la bateria es mayor que el tiempo de simulación seguimos en recarga
	if ((mapaResultado[sensores.posF][sensores.posC] == 'X') && (bateria < tiempo_simulacion)){
		plan.push_front(actIDLE);
	}
	// en caso contrario:
	else{
		// si hemos encontrado una bateria  y se cumplen las condiciones de umbral tiempo y bateria el nuevo destino es ir hacia la batería
		if ((bateria < umbral_bateria) && (tiempo_simulacion > umbral_tiempo) && (set_baterias.size()>0)){
			destino = buscarBateria(sensores.posF, sensores.posC);
		}
		
		// si estamos tardando poco tiempo en calcular hacemos una búsqueda completa hasta destino
		if (busqueda_completa)
			hayplan = pathFinding (4, actual, destino, plan);
		else  // si estamos tardando mucho en calcular hacemos búsquedas entre las casillas que tenemos alrededor
			hayplan = avanceRapido(sensores, destino);
	}
}

/*
	Función para cambiar el Estado (batería, zapatillas y bikini) dependiendo de la acción que vayamos a realizar
*/
void ComportamientoJugador::cambiarEstado(int fila, int columna){
	if (mapaResultado[fila][columna] == 'K')
		bikini = true;
	if (mapaResultado[fila][columna] == 'D')
		zapatillas = true;
}

/*
	Función para ver si conocemos ya el mapa que tenemos delante o no. Si la casilla que tenemos delante es '?' 
	devuelve true, y en caso contrario devuele false
*/
bool ComportamientoJugador::ampliarHorizonte(Sensores sensores){
	int fil=sensores.posF, col=sensores.posC;

	switch (sensores.sentido) {
		case 0: fil--; break;
		case 1: col++; break;
		case 2: fil++; break;
		case 3: col--; break;
	}

	// Compruebo que no me salgo fuera del rango del mapa
	if (fil<0 or fil>=mapaResultado.size()) return true;
	if (col<0 or col>=mapaResultado[0].size()) return true;

	// Miro si esa casilla es desconocida
	if ((mapaResultado[fil][col]) == '?')
		return true;
	else
	  	return false;
	
}


/*
	Función que comprueba si tenemos una batería cerca
*/

bool ComportamientoJugador::bateriaCerca(Sensores sensores){
	int cerca = 5;
	for (int fila = -cerca; fila < cerca; fila++)
		for (int col = -cerca; col < cerca; col++){
			int fila_a = sensores.posF+fila; 
			int col_a = sensores.posC+col;
			if (mapaResultado.size() > fila_a && fila_a > 0 && mapaResultado.size() > col_a && col_a > 0)
			if (mapaResultado[fila_a][col_a] == 'X')
				return true;
		}
	
	return false;
}

/*
	Función que rellena la variable MapaResultado con la información que dan los Sensores respecto al triángulo que 
	tenemos delante
*/

void ComportamientoJugador::rellenarMatriz(Sensores sensores){
	
	mapaResultado[sensores.posF][sensores.posC] = sensores.terreno[0];
	posiciones_conocidas.insert({sensores.posF, sensores.posC, -1});
	
	int contador = 0;
	switch(sensores.sentido){

		case 0: //norte
			for(int i=0; i<=3; i++)
        		for(int j=-i; j<=i; j++){
           			if((0<=actual.fila-i && actual.fila-i<=99) && (0<=actual.columna+j && actual.columna+j<=99)){
            			mapaResultado[actual.fila-i][actual.columna+j]=sensores.terreno[contador];
						posiciones_conocidas.insert({actual.fila-i, actual.columna+j, -1});
						if (mapaResultado[actual.fila-i][actual.columna+j] == 'X')
							set_baterias.insert({actual.fila-i, actual.columna+j, -1});
						if (mapaResultado[actual.fila-i][actual.columna+j] == 'D')
							estado_zapatillas = {actual.fila-i, actual.columna+j, -1};
						if (mapaResultado[actual.fila-i][actual.columna+j] == 'K')
							estado_bikini = {actual.fila-i, actual.columna+j, -1};
					}
            		contador++;
						
         		}
		break;
		case 1: //este
			for(int i=0; i<=3; i++)
          		for(int j=-i; j<=i; j++){
            		if((0<=actual.fila+j && actual.fila+j<=99) && (0<=actual.columna+i && actual.columna+i<=99)){
            			mapaResultado[actual.fila+j][actual.columna+i]=sensores.terreno[contador];
            			posiciones_conocidas.insert({actual.fila+j, actual.columna+i, -1});
						if (mapaResultado[actual.fila+j][actual.columna+i] == 'X')
							set_baterias.insert({actual.fila+j, actual.columna+i, -1});
						if (mapaResultado[actual.fila+j][actual.columna+i] == 'D')
							estado_zapatillas = {actual.fila+j, actual.columna+i, -1};
						if (mapaResultado[actual.fila+j][actual.columna+i] == 'K')
							estado_bikini = {actual.fila+j, actual.columna+i, -1};
					}
					contador++;
       		    }
		break;
		case 2: //sur
			for(int i=0; i<=3; i++)
        		for(int j=i; j>=-i; j--){
          			if((0<=actual.fila+i && actual.fila+i<=99) && (0<=actual.columna+j && actual.columna+j<=99)){
						mapaResultado[actual.fila+i][actual.columna+j]=sensores.terreno[contador];
						posiciones_conocidas.insert({actual.fila+i, actual.columna+j, -1});
						if (mapaResultado[actual.fila+i][actual.columna+j] == 'X')
							set_baterias.insert({actual.fila+i, actual.columna+j, -1});
						if (mapaResultado[actual.fila+i][actual.columna+j] == 'D')
							estado_zapatillas = {actual.fila+i, actual.columna+j, -1};
						if (mapaResultado[actual.fila+i][actual.columna+j] == 'K')
							estado_bikini = {actual.fila+i, actual.columna+j, -1};
					  }
					contador++;
          		}
		break;
		case 3: //oeste
			for(int i=0; i<=3; i++)
          		for(int j=-i; j<=i; j++){
          			if((0<=actual.fila-j && actual.fila-j<=99) && (0<=actual.columna-i && actual.columna-i<=99)){
						mapaResultado[actual.fila-j][actual.columna-i]=sensores.terreno[contador];
						posiciones_conocidas.insert({actual.fila-j, actual.columna-i, -1});
						if (mapaResultado[actual.fila-j][actual.columna-i] == 'X')
							set_baterias.insert({actual.fila-j, actual.columna-i, -1});
						if (mapaResultado[actual.fila-j][actual.columna-i] == 'D')
							estado_zapatillas = {actual.fila-j, actual.columna-i, -1};
						if (mapaResultado[actual.fila-j][actual.columna-i] == 'K')
							estado_bikini = {actual.fila-j, actual.columna-i, -1};
					  }
					contador++;
         		}
		break;
	}
	

}




/*
	Función para ver cuál es la frontera más cercana en un radio de casillas. Devuelve un set de posiciones
	ordenadas de menor a mayor según estén a más o menos distancia de nuestro objetivo
*/
multiset<posicion, ComparaDistancia> ComportamientoJugador::fronteraCercana(Sensores sensores, const estado & destino){
	
	double distancia = 0;
	multiset<posicion, ComparaDistancia> setDistancias;
	int radio = 9;
	for (std::set<estado, ComparaEstados>::iterator it=posiciones_conocidas.begin(); it!=posiciones_conocidas.end(); ++it){
		distancia = 0;
		if ((mapaResultado[(*it).fila][(*it).columna] != 'P') and (mapaResultado[(*it).fila][(*it).columna] != 'M')){
			if (calcularDistancia(sensores.posF, sensores.posC, (*it).fila, (*it).columna) < radio){//mirar si borrar
				distancia = calcularDistancia((*it).fila, (*it).columna, destino.fila, destino.columna);
				setDistancias.insert({(*it).fila, (*it).columna, distancia});
			}
		}
	}
	return setDistancias;
}

/*
	Función que comprueba entre todas las fronteras generadas por fronteraCercana() ordenadas según la cercanía al 
	objetivo cuáles son alcanzables y para la primera que lo encuentre devuelve un plan según el método de busqueda
	por coste uniforme
*/

bool ComportamientoJugador::avanceRapido(Sensores sensores, const estado & destino){
	
	hayplan = false;
	estado estado_posible;
		
	actual.fila        = sensores.posF;
    actual.columna     = sensores.posC;
    actual.orientacion = sensores.sentido;
	
	multiset<posicion, ComparaDistancia> posibles_fronteras = fronteraCercana(sensores, destino);
	for (std::set<posicion, ComparaDistancia>::iterator it=posibles_fronteras.begin(); it!=posibles_fronteras.end(); ++it){
		estado_posible ={(*it).fila, (*it).columna, -1};
		hayplan = pathFinding(4, actual, estado_posible, plan);
		if (hayplan)	
			break;
	}
	return hayplan;
}







/*
	Llama al algoritmo de busqueda que se usará en cada comportamiento del agente
 	Level representa el comportamiento en el que fue iniciado el agente.
*/
bool ComportamientoJugador::pathFinding (int level, const estado &origen, const estado &destino, list<Action> &plan){
	switch (level){
		case 1: cout << "Busqueda en profundidad\n";
			      return pathFinding_Profundidad(origen,destino,plan);
						break;
		case 2: cout << "Busqueda en Anchura\n";
			      return pathFinding_Anchura(origen,destino,plan);
						break;
		case 3: cout << "Busqueda Costo Uniforme\n";
						return pathFinding_CostoUniforme(origen,destino,plan);
						break;
		case 4: cout << "Busqueda para el reto\n";
						return pathFinding_AlgoritmoA(origen,destino,plan);
						break;
	}
	cout << "Comportamiento sin implementar\n";
	return false;
}


//---------------------- Implementación de la busqueda en profundidad ---------------------------

/* 
	Dado el código en carácter de una casilla del mapa dice si se puede
 	pasar por ella sin riegos de morir o chocar.
*/
bool EsObstaculo(unsigned char casilla){
	if (casilla=='P' or casilla=='M')
		return true;
	else
	  return false;
}


/* 
	Comprueba si la casilla que hay delante es un obstaculo. Si es un
	obstaculo devuelve true. Si no es un obstaculo, devuelve false y
 	modifica st con la posición de la casilla del avance.
*/
bool ComportamientoJugador::HayObstaculoDelante(estado &st){
	int fil=st.fila, col=st.columna;

  // calculo cual es la casilla de delante del agente
	switch (st.orientacion) {
		case 0: fil--; break;
		case 1: col++; break;
		case 2: fil++; break;
		case 3: col--; break;
	}

	// Compruebo que no me salgo fuera del rango del mapa
	if (fil<0 or fil>=mapaResultado.size()) return true;
	if (col<0 or col>=mapaResultado[0].size()) return true;

	// Miro si en esa casilla hay un obstaculo infranqueable
	if (!EsObstaculo(mapaResultado[fil][col])){
		// No hay obstaculo, actualizo el parámetro st poniendo la casilla de delante.
    	st.fila = fil;
		st.columna = col;
		return false;
	}
	else{
	  return true;
	}
}

/*
	Struct que contiene información acerca del estado, la secuencia de acciones, la batería, las zapatillas, el bikini y la distancia de un nodo
*/
struct nodo{
	estado st;
	list<Action> secuencia;
	int bateria;
	bool zapatillas;
	bool bikini;
	double distancia;
};






/*
	Implementación de la búsqueda en profundidad.
	Entran los puntos origen y destino y devuelve la
 	secuencia de acciones en plan, una lista de acciones.
*/
bool ComportamientoJugador::pathFinding_Profundidad(const estado &origen, const estado &destino, list<Action> &plan) {
	//Borro la lista
	cout << "Calculando plan\n";
	plan.clear();
	set<estado,ComparaEstados> generados; // Lista de Cerrados
	stack<nodo> pila;											// Lista de Abiertos

 	nodo current;
	current.st = origen;
	current.secuencia.empty();

	pila.push(current);

  	while (!pila.empty() and (current.st.fila!=destino.fila or current.st.columna != destino.columna)){

		pila.pop();
		generados.insert(current.st);

		// Generar descendiente de girar a la derecha
		nodo hijoTurnR = current;
		hijoTurnR.st.orientacion = (hijoTurnR.st.orientacion+1)%4;
		if (generados.find(hijoTurnR.st) == generados.end()){
			hijoTurnR.secuencia.push_back(actTURN_R);
			pila.push(hijoTurnR);
		}

		// Generar descendiente de girar a la izquierda
		nodo hijoTurnL = current;
		hijoTurnL.st.orientacion = (hijoTurnL.st.orientacion+3)%4;
		if (generados.find(hijoTurnL.st) == generados.end()){
			hijoTurnL.secuencia.push_back(actTURN_L);
			pila.push(hijoTurnL);
		}

		// Generar descendiente de avanzar
		nodo hijoForward = current;
		if (!HayObstaculoDelante(hijoForward.st)){
			if (generados.find(hijoForward.st) == generados.end()){
				hijoForward.secuencia.push_back(actFORWARD);
				pila.push(hijoForward);
			}
		}

		// Tomo el siguiente valor de la pila
		if (!pila.empty()){
			current = pila.top();
		}
	}

  cout << "Terminada la busqueda\n";

	if (current.st.fila == destino.fila and current.st.columna == destino.columna){
		cout << "Cargando el plan\n";
		plan = current.secuencia;
		cout << "Longitud del plan: " << plan.size() << endl;
		PintaPlan(plan);
		// ver el plan en el mapa
		VisualizaPlan(origen, plan);
		return true;
	}
	else {
		cout << "No encontrado plan\n";
	}


	return false;
}




bool ComportamientoJugador::pathFinding_Anchura(const estado &origen, const estado &destino, list<Action> &plan) {
	//Borro la lista
	cout << "Calculando plan anchura\n";
	plan.clear();
	set<estado,ComparaEstados> generados; // Lista de Cerrados
	queue<nodo> cola;											// Lista de Abiertos

  	nodo current;
	current.st = origen;
	current.secuencia.empty();

	cola.push(current);

  	while (!cola.empty() and (current.st.fila!=destino.fila or current.st.columna != destino.columna)){

		cola.pop();
		generados.insert(current.st);

		// Generar descendiente de girar a la derecha
		nodo hijoTurnR = current;
		hijoTurnR.st.orientacion = (hijoTurnR.st.orientacion+1)%4;
		if (generados.find(hijoTurnR.st) == generados.end()){
			hijoTurnR.secuencia.push_back(actTURN_R);
			cola.push(hijoTurnR);
		}

		// Generar descendiente de girar a la izquierda
		nodo hijoTurnL = current;
		hijoTurnL.st.orientacion = (hijoTurnL.st.orientacion+3)%4;
		if (generados.find(hijoTurnL.st) == generados.end()){
			hijoTurnL.secuencia.push_back(actTURN_L);
			cola.push(hijoTurnL);
		}

		// Generar descendiente de avanzar
		nodo hijoForward = current;
		if (!HayObstaculoDelante(hijoForward.st)){
			if (generados.find(hijoForward.st) == generados.end()){
				hijoForward.secuencia.push_back(actFORWARD);
				cola.push(hijoForward);
			}
		}

		// Tomo el siguiente valor de la pila
		if (!cola.empty()){
			current = cola.front();
		}
	}

  cout << "Terminada la busqueda\n";

	if (current.st.fila == destino.fila and current.st.columna == destino.columna){
		cout << "Cargando el plan\n";
		plan = current.secuencia;
		cout << "Longitud del plan: " << plan.size() << endl;
		PintaPlan(plan);
		// ver el plan en el mapa
		VisualizaPlan(origen, plan);
		return true;
	}
	else {
		cout << "No encontrado plan\n";
	}


	return false;
}



/*
	Función para cambiar la batería del nodo según el terreno por el que pase. Batería en este caso es el valor de la batería gastada
*/
void cambiarBateria (nodo & hijo, const char terreno){
	if (terreno == 'A'){
		if(hijo.bikini){
			hijo.bateria = hijo.bateria-10;
		}
		else{
			hijo.bateria= hijo.bateria-100;
		}
	}
	else if (terreno == 'B'){
		if(hijo.zapatillas){
			hijo.bateria = hijo.bateria-5;
		}
		else{
			hijo.bateria = hijo.bateria-50;
		}
	}
	else if (terreno == 'T'){
		hijo.bateria = hijo.bateria-2;
	}
	else{
		hijo.bateria = hijo.bateria-1;
	}
	if ((terreno == 'X') and (hijo.bateria < 3000))
		hijo.bateria = min (hijo.bateria+10, 3000);
}



/*
	Función que cambia los atributos del nodo si se encuentra unas zapatillas o un bikini
*/
void cambiarAtributos(nodo & hijo, const char terreno){
	if (terreno == 'K'){
		hijo.bikini = true;
	}
	if (terreno == 'D'){
		hijo.zapatillas = true;	
	}
	
}


/*
	Struct para comparar la batería de los nodos que introducimos en el multiset
*/
struct ComparaBateria{
	bool operator()(const nodo &a, const nodo &b) const{
		if (a.bateria > b.bateria)
			return true;
		else
			return false;
	}
};

bool ComportamientoJugador::pathFinding_CostoUniforme(const estado &origen, const estado &destino, list<Action> &plan) {
	//Borro la lista
	cout << "Calculando plan\n";
	plan.clear();
	set<estado,ComparaEstados> generados; // Lista de Cerrados
	multiset<nodo, ComparaBateria> setNodos;											// Lista de Abiertos

 	nodo current;
	current.st = origen;
	current.bateria = bateria;
	current.bikini = bikini;
	current.zapatillas = zapatillas;
	current.secuencia.empty();

	setNodos.insert(current);

  while (!setNodos.empty() and (current.st.fila!=destino.fila or current.st.columna != destino.columna)){
	  	
		
		setNodos.erase(setNodos.begin());
		
		generados.insert(current.st);



		// Generar descendiente de avanzar
		nodo hijoForward = current;
		if (!HayObstaculoDelante(hijoForward.st)){
			
			if (generados.find(hijoForward.st) == generados.end()){
				const char terreno = mapaResultado[hijoForward.st.fila][hijoForward.st.columna];
				
				cambiarAtributos(hijoForward, terreno);
				cambiarBateria(hijoForward, terreno);
				hijoForward.secuencia.push_back(actFORWARD);
				setNodos.insert(hijoForward);
			}
		}

		nodo hijoTurnR = current;
		hijoTurnR.st.orientacion = (hijoTurnR.st.orientacion+1)%4;
		if (generados.find(hijoTurnR.st) == generados.end()){
			const char terreno = mapaResultado[hijoTurnR.st.fila][hijoTurnR.st.columna];
			cambiarBateria(hijoTurnR, terreno);
			hijoTurnR.secuencia.push_back(actTURN_R);
			setNodos.insert(hijoTurnR);

		}

		// Generar descendiente de girar a la izquierda
		nodo hijoTurnL = current;
		hijoTurnL.st.orientacion = (hijoTurnL.st.orientacion+3)%4;
		if (generados.find(hijoTurnL.st) == generados.end()){
			const char terreno = mapaResultado[hijoTurnL.st.fila][hijoTurnL.st.columna];
			cambiarBateria(hijoTurnL, terreno);
			hijoTurnL.secuencia.push_back(actTURN_L);
			setNodos.insert(hijoTurnL);
		}

		

		// Tomo el siguiente valor del set
		if (!setNodos.empty()){
			current = *(setNodos.begin());
		}
	}
	
  cout << "Terminada la busqueda\n";

	if (current.st.fila == destino.fila and current.st.columna == destino.columna){
		cout << "Cargando el plan\n";
		plan = current.secuencia;
		

		cout << "Longitud del plan: " << plan.size() << endl;
		PintaPlan(plan);
		// ver el plan en el mapa
		VisualizaPlan(origen, plan);
		return true;
	}
	else {
		cout << "No encontrado plan\n";
	}


	return false;
}

/*
	Función que compara los valores de dos nodos para introducirlos en el set. La heurística que utilizamos será F = b + d, donde b es la batería
	que tiene el nodo (la que le queda, no la que ha gastado), y d es la distancia hasta el nodo objetivo
*/

struct ComparaEstrella{
	bool operator()(const nodo &a, const nodo &b) const{
		double bateria_gastada1 = 3000 - a.bateria;
		double bateria_gastada2 = 3000 - b.bateria;
		double valor_a = bateria_gastada1 + a.distancia;
		double valor_b = bateria_gastada2 + b.distancia;

		if (valor_a < valor_b)
			return true;
		else
			return false;			
		
	}
};




/*
Algoritmo para el nivel 2
*/

bool ComportamientoJugador::pathFinding_AlgoritmoA(const estado &origen, const estado &destino, list<Action> &plan) {
	//Borro la lista
	cout << "Calculando plan \n";
	plan.clear();
	set<estado,ComparaEstados> generados; // Lista de Cerrados
	multiset<nodo, ComparaEstrella> setNodos;											// Lista de Abiertos
	char terreno;
 	nodo current;
	current.st = origen;
	current.bateria = bateria;
	current.bikini = bikini;
	current.zapatillas = zapatillas;
	current.secuencia.empty();
	 
	int nodo_distancia;
	current.distancia = calcularDistancia(current.st.fila, current.st.columna, destino.fila, destino.columna);
	
	setNodos.insert(current);

  while (!setNodos.empty() and (current.st.fila!=destino.fila or current.st.columna != destino.columna)){
		setNodos.erase(setNodos.begin());
		
		generados.insert(current.st);

		nodo hijoTurnR = current;
		hijoTurnR.st.orientacion = (hijoTurnR.st.orientacion+1)%4;
		if (generados.find(hijoTurnR.st) == generados.end()){
			terreno = mapaResultado[hijoTurnR.st.fila][hijoTurnR.st.columna];
			cambiarBateria(hijoTurnR, terreno);
			hijoTurnR.distancia = calcularDistancia(hijoTurnR.st.fila, hijoTurnR.st.columna, destino.fila, destino.columna);
			hijoTurnR.secuencia.push_back(actTURN_R);
			setNodos.insert(hijoTurnR);

		}

		// Generar descendiente de girar a la izquierda
		nodo hijoTurnL = current;
		hijoTurnL.st.orientacion = (hijoTurnL.st.orientacion+3)%4;
		if (generados.find(hijoTurnL.st) == generados.end()){
			terreno = mapaResultado[hijoTurnL.st.fila][hijoTurnL.st.columna];
			cambiarBateria(hijoTurnL, terreno);
			hijoTurnL.distancia = calcularDistancia(hijoTurnL.st.fila, hijoTurnL.st.columna, destino.fila, destino.columna);
			hijoTurnL.secuencia.push_back(actTURN_L);
			setNodos.insert(hijoTurnL);
		}

		// Generar descendiente de avanzar
		nodo hijoForward = current;
		if (!HayObstaculoDelante(hijoForward.st)){
			if (generados.find(hijoForward.st) == generados.end()){
				terreno = mapaResultado[hijoForward.st.fila][hijoForward.st.columna];
				cambiarAtributos(hijoForward, terreno);
				cambiarBateria(hijoForward, terreno);
				hijoForward.distancia = calcularDistancia(hijoForward.st.fila, hijoForward.st.columna, destino.fila, destino.columna);
				hijoForward.secuencia.push_back(actFORWARD);
				setNodos.insert(hijoForward);
			}
		}
		

		// Tomo el siguiente valor del set
		if (!setNodos.empty()){
			current = *(setNodos.begin());
		}
	}
	
  cout << "Terminada la busqueda\n";

	if (current.st.fila == destino.fila and current.st.columna == destino.columna){
		cout << "Cargando el plan\n";
		plan = current.secuencia;
		

		cout << "Longitud del plan: " << plan.size() << endl;
		PintaPlan(plan);
		// ver el plan en el mapa
		VisualizaPlan(origen, plan);
		return true;
	}
	else {
		cout << "No encontrado plan\n";
	}


	return false;
}







// Sacar por la términal la secuencia del plan obtenido
void ComportamientoJugador::PintaPlan(list<Action> plan) {
	auto it = plan.begin();
	while (it!=plan.end()){
		if (*it == actFORWARD){
			cout << "A ";
		}
		else if (*it == actTURN_R){
			cout << "D ";
		}
		else if (*it == actTURN_L){
			cout << "I ";
		}
		else {
			cout << "- ";
		}
		it++;
	}
	cout << endl;
}



void AnularMatriz(vector<vector<unsigned char> > &m){
	for (int i=0; i<m[0].size(); i++){
		for (int j=0; j<m.size(); j++){
			m[i][j]=0;
		}
	}
}


// Pinta sobre el mapa del juego el plan obtenido
void ComportamientoJugador::VisualizaPlan(const estado &st, const list<Action> &plan){
  AnularMatriz(mapaConPlan);
	estado cst = st;

	auto it = plan.begin();
	while (it!=plan.end()){
		if (*it == actFORWARD){
			switch (cst.orientacion) {
				case 0: cst.fila--; break;
				case 1: cst.columna++; break;
				case 2: cst.fila++; break;
				case 3: cst.columna--; break;
			}
			mapaConPlan[cst.fila][cst.columna]=1;
		}
		else if (*it == actTURN_R){
			cst.orientacion = (cst.orientacion+1)%4;
		}
		else if (*it == actTURN_L){
			cst.orientacion = (cst.orientacion+3)%4;
		}
		it++;
	}
}



int ComportamientoJugador::interact(Action accion, int valor){
  return false;
}
