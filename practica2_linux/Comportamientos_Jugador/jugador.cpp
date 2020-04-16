#include "../Comportamientos_Jugador/jugador.hpp"
#include "motorlib/util.h"

#include <iostream>
#include <cmath>
#include <set>

#include <stack>
#include <queue>


using namespace std;

// Este es el método principal que debe contener los 4 Comportamientos_Jugador
// que se piden en la práctica. Tiene como entrada la información de los
// sensores y devuelve la acción a realizar.
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
  else {
    // Estoy en el nivel 2
    nivel2(sensores);
  }

  return accion;
}

void ComportamientoJugador::nivel2(Sensores sensores){
	while ((current.st.fila!=destino.fila) or (current.st.columna != destino.columna)){
		ampliarHorizonte(sensores);
		set = fronteraCercana();
		for (int i = 0; i < set.size(); i++){
			hayplan = pathFinding(2, actual, (*set.begin()), plan);
			if (hayplan) break;
		}
		if( hayplan and plan.size() > 0) {
			accion = plan.front();
			plan.erase(plan.begin());
		}
	}
}


void fronteraCercana(){
	int distancia = 0;
	set<posicion, ComparaDistancia> setDistancias;
	for (int fila = -3; fila < 4; fila++)
		for (int col = -3; col < 4; col++){
			int fila_a = sensores.posF+fila; 
			int col_a = sensores.posC+col;
			if ((mapaResultado[fila_a][col_a] != P)  and (mapaResultado[fila_a][col_a] != M)){
				distancia = calcularDistancia(sensores.posF+fila, sensores.posC+col);
				posicion = {fila, columna, distancia};
				setDistancias.insert(posicion);
			}
			
		}
}

void ComportamientoJugador::ampliarHorizonte(Sensores sensores){
	for (int i = 0; i < 4; i++){
		rellenarMatriz(sensores);
		actTURN_R;
	}
}
// el caso norte con respecto al este cambia fila = -columna, columna= fila, cambiar!
void ComportamientoJugador::rellenarMatriz(Sensores sensores){
	mapaResultado[sensores.posF][sensores.posC] = sensores.terreno[0];
	int fila = sensores.posF;
	int columna = sensores.posC;
	switch(sensores.sentido){
		case norte:
			fila = -columna;
			columna = fila;
			/*mapaResultado[sensores.posF-1][sensores.posC-1]=sensores.terreno][1];
			mapaResultado[sensores.posF-1][sensores.posC] = sensores.terreno[2];
			mapaResultado[sensores.posF-1][sensores.posC+1] = sensores.terreno[3];
			mapaResultado[sensores.posF-2][sensores.posC-2]=sensores.terreno][4];
			mapaResultado[sensores.posF-2][sensores.posC-1] = sensores.terreno[5];
			mapaResultado[sensores.posF-2][sensores.posC]=sensores.terreno][6];
			mapaResultado[sensores.posF-2][sensores.posC+1] = sensores.terreno[7];
			mapaResultado[sensores.posF-2][sensores.posC+2]=sensores.terreno][8];
			mapaResultado[sensores.posF-3][sensores.posC-3] = sensores.terreno[9];
			mapaResultado[sensores.posF-3][sensores.posC-2]=sensores.terreno][10];
			mapaResultado[sensores.posF-3][sensores.posC-1]=sensores.terreno][11];
			mapaResultado[sensores.posF-3][sensores.posC]=sensores.terreno][12];
			mapaResultado[sensores.posF-3][sensores.posC+1]=sensores.terreno][13];
			mapaResultado[sensores.posF-3][sensores.posC+2]=sensores.terreno][14];
			mapaResultado[sensores.posF-3][sensores.posC+3]=sensores.terreno][15];*/
		break;
		case este:
			/*mapaResultado[sensores.posF-1][sensores.posC+1]=sensores.terreno][1];
			mapaResultado[sensores.posF][sensores.posC+1] = sensores.terreno[2];
			mapaResultado[sensores.posF+1][sensores.posC+1] = sensores.terreno[3];
			mapaResultado[sensores.posF-2][sensores.posC+2]=sensores.terreno][4];
			mapaResultado[sensores.posF-1][sensores.posC+2] = sensores.terreno[5];
			mapaResultado[sensores.posF][sensores.posC+2]=sensores.terreno][6];
			mapaResultado[sensores.posF+1][sensores.posC+2] = sensores.terreno[7];
			mapaResultado[sensores.posF+2][sensores.posC+2]=sensores.terreno][8];
			mapaResultado[sensores.posF-3][sensores.posC+3] = sensores.terreno[9];
			mapaResultado[sensores.posF-2][sensores.posC+3]=sensores.terreno][10];
			mapaResultado[sensores.posF-1][sensores.posC+3]=sensores.terreno][11];
			mapaResultado[sensores.posF][sensores.posC+3]=sensores.terreno][12];
			mapaResultado[sensores.posF+1][sensores.posC+3]=sensores.terreno][13];
			mapaResultado[sensores.posF+2][sensores.posC+3]=sensores.terreno][14];
			mapaResultado[sensores.posF+3][sensores.posC+3]=sensores.terreno][15];*/
		break;
		case sur:
			fila = columna;
			columna = - fila;
		break;
		case oeste:
			fila = -fila;
			columna = -columna;
		break;
	}
	mapaResultado[fila-1][columna+1]=sensores.terreno[1];
	mapaResultado[fila][columna+1] = sensores.terreno[2];
	mapaResultado[fila+1][columna+1] = sensores.terreno[3];
	mapaResultado[fila-2][columna+2]=sensores.terreno[4];
	mapaResultado[fila-1][columna+2] = sensores.terreno[5];
	mapaResultado[fila][columna+2]=sensores.terreno[6];
	mapaResultado[fila+1][columna+2] = sensores.terreno[7];
	mapaResultado[fila+2][columna+2]=sensores.terreno[8];
	mapaResultado[fila-3][columna+3] = sensores.terreno[9];
	mapaResultado[fila-2][columna+3]=sensores.terreno[10];
	mapaResultado[fila-1][columna+3]=sensores.terreno[11];
	mapaResultado[fila][columna+3]=sensores.terreno[12];
	mapaResultado[fila+1][columna+3]=sensores.terreno[13];
	mapaResultado[fila+2][columna+3]=sensores.terreno[14];
	mapaResultado[fila+3][columna+3]=sensores.terreno[15];
}

// Llama al algoritmo de busqueda que se usará en cada comportamiento del agente
// Level representa el comportamiento en el que fue iniciado el agente.
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
						// Incluir aqui la llamada al algoritmo de búsqueda usado en el nivel 2
						break;
	}
	cout << "Comportamiento sin implementar\n";
	return false;
}


//---------------------- Implementación de la busqueda en profundidad ---------------------------

// Dado el código en carácter de una casilla del mapa dice si se puede
// pasar por ella sin riegos de morir o chocar.
bool EsObstaculo(unsigned char casilla){
	if (casilla=='P' or casilla=='M')
		return true;
	else
	  return false;
}


// Comprueba si la casilla que hay delante es un obstaculo. Si es un
// obstaculo devuelve true. Si no es un obstaculo, devuelve false y
// modifica st con la posición de la casilla del avance.
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




struct nodo{
	estado st;
	list<Action> secuencia;
	int bateria;
	bool zapatillas = false;
	bool bikini = false;
};

struct ComparaEstados{
	bool operator()(const estado &a, const estado &n) const{
		if ((a.fila > n.fila) or (a.fila == n.fila and a.columna > n.columna) or
	      (a.fila == n.fila and a.columna == n.columna and a.orientacion > n.orientacion))
			return true;
		else
			return false;
	}
};

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


void cambiarAtributos(nodo & hijo, const char terreno){
	if (terreno == 'K') 
		hijo.bikini = true;
	if (terreno == 'D')
		hijo.zapatillas = true;	
}

// Implementación de la búsqueda en profundidad.
// Entran los puntos origen y destino y devuelve la
// secuencia de acciones en plan, una lista de acciones.
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
	cout << "Calculando plan\n";
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
	current.secuencia.empty();

	setNodos.insert(current);

  while (!setNodos.empty() and (current.st.fila!=destino.fila or current.st.columna != destino.columna)){
	  	//cout << current.st.fila << " " << current.st.columna <<  current.st.orientacion << endl;
		//cout << "el numero de set es " << setNodos.size() << endl;
		
		setNodos.erase(setNodos.begin());
		//cout << "el numero de set es " << setNodos.size() << endl;
		generados.insert(current.st);

		// Generar descendiente de girar a la derecha
		nodo hijoTurnR = current;
		hijoTurnR.st.orientacion = (hijoTurnR.st.orientacion+1)%4;
		if (generados.find(hijoTurnR.st) == generados.end()){
			const char terreno = mapaResultado[hijoTurnR.st.fila][hijoTurnR.st.columna];
			cambiarBateria(hijoTurnR, terreno);
			hijoTurnR.secuencia.push_back(actTURN_R);
			//cout << "añadimos nodo con bateria " << hijoTurnR.bateria << endl;
			setNodos.insert(hijoTurnR);

		}

		// Generar descendiente de girar a la izquierda
		nodo hijoTurnL = current;
		hijoTurnL.st.orientacion = (hijoTurnL.st.orientacion+3)%4;
		if (generados.find(hijoTurnL.st) == generados.end()){
			const char terreno = mapaResultado[hijoTurnL.st.fila][hijoTurnL.st.columna];
			cambiarBateria(hijoTurnL, terreno);
			hijoTurnL.secuencia.push_back(actTURN_L);
			//cout << "añadimos nodo con bateria " << hijoTurnR.bateria << endl;
			setNodos.insert(hijoTurnL);
		}

		// Generar descendiente de avanzar
		nodo hijoForward = current;
		if (!HayObstaculoDelante(hijoForward.st)){
			if (generados.find(hijoForward.st) == generados.end()){
				const char terreno = mapaResultado[hijoForward.st.fila][hijoForward.st.columna];
				cambiarAtributos(hijoForward, terreno);
				cambiarBateria(hijoForward, terreno);
				hijoForward.secuencia.push_back(actFORWARD);
				//cout << "añadimos nodo con bateria " << hijoTurnR.bateria << endl;
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
		else {
			cst.orientacion = (cst.orientacion+3)%4;
		}
		it++;
	}
}



int ComportamientoJugador::interact(Action accion, int valor){
  return false;
}
