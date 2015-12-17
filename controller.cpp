#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <sstream>
#include <vector>
#include <fstream>
#include <unistd.h>
#include "utils.cpp"
#include <algorithm>
#define NODES 10
#define TOPOLOGY "topology.txt"

using namespace std;

vector<int>* parseTopologyFile(int i,int nodeNum)
{
	ifstream tplgy;
	string line;
	tplgy.open(TOPOLOGY);
	vector<int> *nbrs = new vector<int>();
	while(tplgy.good())
	{
		getline(tplgy,line);
		string buffer;
		stringstream tokens(line);
		vector<string> token;
		while(getline(tokens, buffer, ' '))
		{
			token.push_back(buffer);
		}
		if(!token.empty())
		{
			int dest = atoi(token.at(3).c_str());
			if(atoi(token.at(2).c_str())==nodeNum)
			{
				if(token.at(1)=="UP")
					nbrs->push_back(dest);
				else{
					if((token.at(1)=="DOWN")&&(i>=atoi(token.at(0).c_str())))
					{
						vector<int>::iterator it = remove(nbrs->begin(),nbrs->end(),dest);
						nbrs->erase(it);
					}
				}
			}

		}
	}
	return nbrs;
}

void writeToFile(int nbr, string message)
{
	vector<int>::iterator it;
	int index=0;
	ifstream to;
	to.open(toFile(nbr).c_str());
	if(message.find("TC")!=string::npos)
	{
			while(to.good())
		        {
                	string line;
                	getline(to, line);

			if(line.find(message)!=string::npos)
			{
				to.close();
				return;
			}
		}		
	}
	to.close();
	ofstream ito;
        ito.open(toFile(nbr).c_str(),ios::out|ios::app);

	ito<<message<<"\n";
	ito.close();
}

void readFromFile(int ts,int nodeNum)
{
	ifstream from;
	from.open(fromFile(nodeNum).c_str());
	string line;
	vector<string> lines;
	while(from.good())
	{
		getline(from, line);
		 if(line == "")
                {
                        break;
                }
		string buffer;
		stringstream tokens(line);
		size_t found = line.find(".");
		if(found == string::npos)
		{
			vector<string> token;
			while(getline(tokens, buffer, ' '))
			{
				token.push_back(buffer);
			}
			if(!token.empty())
			{
				vector<int> *nbrs;
				if(token.at(2)=="HELLO") 
				{	
					nbrs=parseTopologyFile(ts, atoi(token.at(1).c_str()));
					vector<int>::iterator it;
					for(it=nbrs->begin();it!=nbrs->end();++it)
					{
						writeToFile(*it, line);
					}
				}
				if (token.at(2)=="TC")
                                {
                                        nbrs=parseTopologyFile(ts, atoi(token.at(1).c_str()));
                                        vector<int>::iterator it;
                                        for(it=nbrs->begin();it!=nbrs->end();++it){
						if(*it != atoi(token.at(3).c_str()))	
                                                	writeToFile(*it, line);
					}
                                }

				if(token.at(2)=="DATA")
				{
					writeToFile(atoi(token.at(0).c_str()), line);	
				}
			}
			line.append(".");
		}
		lines.push_back(line);
	}
	ofstream writeFrom;
        writeFrom.open(fromFile(nodeNum).c_str());
	for (int v=0;v<lines.size();v++)
	{
		writeFrom << lines.at(v) << "\n";
	}

	
}
int main()
{
	int i;
	int j=0;
	while(j<120){
		for (i=0;i<NODES;i++)
		{
			readFromFile(j,i);
		
		}
		j=j+1;
		sleep(1);
	}
}
