#ifndef COMPORTAMIENTOJUGADOR_H
#define COMPORTAMIENTOJUGADOR_H

#include "comportamientos/comportamiento.hpp"

#include <list>
#include <set>


struct posicion{
	int fila;
	int columna;
	double distancia;
};

struct ComparaDistancia{
	bool operator()(const posicion &a, const posicion &n) const{
		if (a.distancia < n.distancia)
			return true;
		else
			return false;
	}
};

struct estado {
  int fila;
  int columna;
  int orientacion;
};

class ComportamientoJugador : public Comportamiento {
  public:
    ComportamientoJugador(unsigned int size) : Comportamiento(size) {
      // Inicializar Variables de Estado
      fil = col = 99;
      brujula = 0; // 0: Norte, 1:Este, 2:Sur, 3:Oeste
      destino.fila = -1;
      destino.columna = -1;
      destino.orientacion = -1;
      hayplan=false;
    }
    ComportamientoJugador(std::vector< std::vector< unsigned char> > mapaR) : Comportamiento(mapaR) {
      // Inicializar Variables de Estado
      fil = col = 99;
      brujula = 0; // 0: Norte, 1:Este, 2:Sur, 3:Oeste
      destino.fila = -1;
      destino.columna = -1;
      destino.orientacion = -1;
      hayplan=false;
      
    }
    ComportamientoJugador(const ComportamientoJugador & comport) : Comportamiento(comport){}
    ~ComportamientoJugador(){}

    Action think(Sensores sensores);
    int interact(Action accion, int valor);
    void VisualizaPlan(const estado &st, const list<Action> &plan);
    ComportamientoJugador * clone(){return new ComportamientoJugador(*this);}

  private:
    // Declarar Variables de Estado
    int fil, col, brujula;
    estado actual, destino;
    list<Action> plan;
    bool hayplan;
    int bateria = 3000;   
    bool zapatillas = false; 
    bool bikini = false; 
    

    // Métodos privados de la clase
    bool pathFinding(int level, const estado &origen, const estado &destino, list<Action> &plan);
    bool pathFinding_Profundidad(const estado &origen, const estado &destino, list<Action> &plan);
    bool pathFinding_Anchura(const estado &origen, const estado &destino, list<Action> &plan);
    bool pathFinding_CostoUniforme(const estado &origen, const estado &destino, list<Action> &plan);
    bool pathFinding_AlgoritmoA(const estado &origen, const estado &destino, list<Action> &plan);
    void PintaPlan(list<Action> plan);
    bool HayObstaculoDelante(estado &st);
    void rellenarMatriz(Sensores sensores);
    bool ampliarHorizonte(Sensores sensores);
    bool avanceRapido(Sensores sensores, const estado & destino);
    multiset<posicion, ComparaDistancia> fronteraCercana(Sensores sensores, const estado & destino);
    bool bateriaCerca(Sensores sensores);
    void cambiarEstado(int fila, int columna);
    void buscarPlan(Sensores sensores);
   
};

#endif
