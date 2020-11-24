#ifndef Q_LEARNING_Q_H
#define Q_LEARNING_Q_H

#include <iostream>
#include <strstream>
#include <rcsc/player/player_agent.h>
#include <vector>
#include <map>

struct MyAgent{
    double Alpha;
    double Gamma;
    double Eposilon;
    int ActionSpace;
    std::map<std::vector<double>,std::vector<double> > Qtable;
    double mykey;
    std::vector<std::vector<int> > test;
};

struct MyPlayer{
    std::vector<double> State;
    std::vector<std::vector<double> > Actions;
};

class Q {
public:
    
    Q();

    std::map<std::string, std::string > myMap;
    MyAgent myAgent;
    void setKey(double);
    double getKey();
    void NewAgent(int actionSpace);
    int ChooseAction(std::vector<double> state);
    std::vector<double> getActionByState(std::vector<double>);
    void UpdataQ(std::vector<double>&,int,double ,std::vector<double>&);
   
    MyPlayer myPlayer;
    void NewPlayer();
    std::vector<double> getState();

    std::vector<std::string> split(const std::string& str, const std::string& delim);
    std::string convertDoubleToString(const double val);
    std::string convertIntToString(const int val);
    int convertStringToInt(const std::string &s);
    double convertStringToDouble(const std::string &s);
    std::map<std::string, std::string > getQtable();
    void setQtable(std::map<std::vector<double>,std::vector<double> >);
    void save();
    std::map<std::vector<double>,std::vector<double> > read();
    void setQ();

};

#endif
