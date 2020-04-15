#include<set>
#include<iostream>

using namespace  std;

int main(){
    set<int> prueba;
    prueba.insert(2);
    prueba.insert(1);
    prueba.insert(7);
    int numero = *(prueba.begin());
    cout << numero << endl;
    cout << prueba.size();
    prueba.erase(prueba.begin());
    cout << *(prueba.begin()) << endl;
    cout << prueba.size();
}
