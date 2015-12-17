#include <iostream>
#include <string>
#include <sstream>
#include <stdlib.h>
#include <stdio.h>

using namespace std;

string fromFile(int nodeNum)
{	
	/*string fromFile = "from";
        fromFile += nodeNum;
        fromFile += ".txt";
	return fromFile;*/
	ostringstream fromFile;
	fromFile<<"from"<<nodeNum<<".txt";
	return fromFile.str();
}

string toFile(int nodeNum)
{
	ostringstream toFile;
	toFile<<"to"<<nodeNum<<".txt";
	return toFile.str();
}

string receivedFile(int nodeNum)
{
	ostringstream rcvFile;	
	rcvFile<<nodeNum<<"received.txt";
	return rcvFile.str();	
}
string createHelloMessage(int nodeNum)
{
        /*if nt is empty*/
	        ostringstream hloMsg;
		hloMsg<<"* "<<nodeNum<<" HELLO UNIDIR BIDIR MPR";
		return hloMsg.str();
}

vector<string>* tokenize(string line)
{
	stringstream tokens(line);
	vector<string> *token = new vector<string>();
	string buffer;
	while(getline(tokens, buffer,' '))
	{
		token->push_back(buffer);
	}
	return token;

}

string subStrings(string line, string fdelim, string sdelim)
{
	unsigned first = line.find(fdelim);
	unsigned second = line.find(sdelim);
	string sub;

	if(fdelim == "UNIDIR")
		sub = line.substr(first+7, second-first-7);
	if(fdelim == "BIDIR")
		sub = line.substr(first+6, second-first-6);	
	if(fdelim == "MPR")
		sub=line.substr(first+4);
	if(fdelim == "MS"||fdelim == "TC")
		sub=line.substr(first+3);
	return sub;
}

vector<int> toInt(vector<string> *bidirNodes)
{
	vector<int> nodes; //= new vector<int>();
	vector<string>::iterator it;
	for(it=bidirNodes->begin();it!=bidirNodes->end();++it)
	{
		nodes.push_back(atoi(it->c_str()));
	}
	return nodes;
}
