#include <stdio.h>
#include <string>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <vector>
#include<iostream>
#define N 11
using namespace std;

struct Commodity{
    string name;
    vector<double> price;
    double currPrice = 0.00;
    double avgPrice = 0.00;
    Commodity(string name): name(name)
    {
    };
};


int main(int argc,char*argv[])
{
    vector<string> commodities_names {"GOLD", "SILVER","CRUDEOIL","NATURALGAS","ALUMINUM","COPPER","NICKEL","LEAD","ZINC","METHANOIL","COTTON"};
    vector<Commodity> commodities;
    for(int i=0;i<N;i++)
    {
     Commodity c = Commodity(commodities_names[i]);
     commodities.insert(commodities.begin(),c);
    }
    cout << "+-------------------------------------+" << endl;
    cout << "| Currency     | Price     | AvgPrice |" << endl;
    for(int i =0;i<N;i++)
    {
        cout << "| " << commodities[i].name << "|" << commodities[i].currPrice << "|" << commodities[i].avgPrice << "|" << endl;
    }
    cout << "+-------------------------------------+" << endl;
    return 0;
}