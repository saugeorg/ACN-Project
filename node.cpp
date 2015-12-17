 
#include <iostream>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <fstream>
#include <map>
#include <vector>
#include <unistd.h>
#include <algorithm>

#include "utils.cpp"

using namespace std;

typedef struct neigborTable{
	string status;
	vector<int> twoHop;	//Neigbors bidir nodes
	vector<int> nbrsOneHop; //Neigbors UNIDIR nodes
	vector<int> nbrMpr;	//Neigbors MPR list
	int timeStamp;
	
}nbrTble;

typedef struct topolgyCtrlTable{
	vector<int> destn;
	int destnMpr, seqNo, holdTime;
}tcTble;

typedef struct routingTable{
	int nxtHop, distance;
}rtTble;

map<int, nbrTble *> nbrList; /* Neigbor table */
vector<int> msList;
map<int, bool> twoHopnbrs;
map<int, tcTble *> tcList; /* Topology control table */
map<int, rtTble *> rtList; /* Routing table */

void writeTCIntoFrom( int nodeNum, string line)
{
	ofstream from;
        from.open(fromFile(nodeNum).c_str(),ios::out|ios::app);
	try{
		string fromTC = subStrings(line,"TC","\n");
		from << "* "<<nodeNum<<" TC "<<fromTC<<"\n";
		
	}
	catch(...){
	}
	return;
	
}

bool sendData(int nodeId, int src, int dst, string msg)
{
	ofstream from;
        from.open(fromFile(nodeId).c_str(),ios::out|ios::app);
	if(rtList.empty())
	{ 
		return false;
	}

	map<int, rtTble *>::iterator rit = rtList.find(dst);
	if(rit==rtList.end())
	{
		return false;
	}
	int nxtHop = rit->second->nxtHop;
	from << nxtHop <<" "<<nodeId<<" DATA "<<src<<" "<<dst<<" "<<msg<<endl;

	from.close();
	return true;
	
}
void calculateRT(int nodeId)
{
	rtList.clear();
	map<int, nbrTble *>::iterator mit;
	for(mit=nbrList.begin();mit!=nbrList.end();++mit)
	{
		rtTble *rt = new rtTble();
		rt->distance = 1;
		rt->nxtHop = mit->first; //one hop neighbors
		rtList[mit->first]=rt;
		vector<int>::iterator vit, kit, uit;
		if(mit->second->status == "MPR"){
			for(vit=mit->second->twoHop.begin();vit!=mit->second->twoHop.end();++vit)
			{
				/* I should not consider myself */
				if(*vit == nodeId)
					continue;
				rtTble *rt = new rtTble();
				rt->distance = 2;
				rt->nxtHop = mit->first;
				rtList[*vit]=rt;
			}
			for(uit=mit->second->nbrsOneHop.begin();uit!=mit->second->nbrsOneHop.end();++uit)
			{
				if(*uit == nodeId)
					continue;
				rtTble *rt = new rtTble;
				rt->distance = 2;
				rt->nxtHop = mit->first;
				rtList[*uit]=rt;
			}
			for(kit=mit->second->nbrMpr.begin();kit!=mit->second->nbrMpr.end();++kit)
			{

			rtTble *rt = new rtTble();
                        rt->distance = 2;
                        rt->nxtHop = mit->first;
			rtList[*kit]=rt;
			}
		}
		
	}	

	bool flag=true;
	map<int, rtTble *>::iterator rt;
	vector<int> visited;
	if(tcList.empty())
		return;
	while(flag)
	{
		map<int, tcTble *>::iterator tt;
		for(tt=tcList.begin();tt!=tcList.end();++tt)
		{
			if(tt->first == nodeId) 
			{	
				visited.push_back(tt->first);
				continue;
			}
			if(rtList.find(tt->first)!=rtList.end())
			{
				if(find(visited.begin(),visited.end(),tt->first)!=visited.end())
					continue;
				else
					visited.push_back(tt->first);
				//flag=false;
				if(rtList.find(tt->first)->second->distance!=1)
				{
					vector<int>::iterator nt;
					//flag = false;
					for(nt=tt->second->destn.begin();nt!=tt->second->destn.end();++nt)
					{
						if((rtList.find(*nt)!=rtList.end())||(*nt==nodeId)){
						/*do nothing*/
						}
						else
						{
							flag=true;
							rtTble *rt = new rtTble();
							rt->nxtHop=rtList.find(tt->first)->second->nxtHop;
							rt->distance=rtList.find(tt->first)->second->distance+1;
							rtList[*nt]=rt;
						}
					}
					if(visited.size()==tcList.size())
						flag=false;
				}
			}
		}
		if(visited.size()==tcList.size())
         		flag=false;
	}
	return;
}

bool updateTCTable(int curTime, int srcNode, int tcSeqNo, vector<int> dList)
{
	map<int, tcTble *>::iterator tit;
	tit = tcList.find(srcNode);
	if(tit != tcList.end())
	{
		/*If the information in the message has a smaller sequence number than
		 what you have seen before from <srcndoe> you just throw the message away */
		if(tit->second->seqNo > tcSeqNo)
			return false;
		if((tit->first==srcNode)&&(tit->second->seqNo > tcSeqNo))
			return false;
		if(tit->second->destn == dList)
			tit->second->holdTime = curTime;
		else{
			tit->second->destn.clear();
			tit->second->destn=dList;
			tit->second->seqNo=tcSeqNo;
			tit->second->holdTime = curTime;
		} 
	}
	return true;
}

/* Read toX.txt file and process messages according to whether they are HELLO, DATA or TC */
void parseToFile(int ts, int nodeNum)
{
	ifstream to;
	to.open(toFile(nodeNum).c_str());
	vector<string> lines;
	int lineCount = 0;

	while(to.good())
	{
		string line;
		getline(to, line);
		size_t processed = line.find(".");
		if(line == "")
		{
			break;	
		}
		if(processed ==  string::npos) 
		{
			vector<string> *token = tokenize(line);
			if(!token->empty())
			{
				/* HELLO message */
				if(token->at(2) == "HELLO")
				{
					int oneHop = atoi(token->at(1).c_str());
					map<int, nbrTble *>::iterator it = nbrList.find(oneHop);
					/* New one Hop Neighbor */
					if(it == nbrList.end())
					{
						nbrTble *nt = new nbrTble();
						nt->status = "UNIDIR";
						nt->timeStamp = ts;
						nbrList[oneHop]=nt;
					}
					else
					{
						it->second->timeStamp = ts;
						it->second->twoHop.clear();
						it->second->nbrsOneHop.clear();
						it->second->nbrMpr.clear();
						/* Determine if neigbori node is bidirectional and determine neigbours one hope neigbours*/
						vector<string> *unidirNodes = tokenize(subStrings(line, "UNIDIR","BIDIR"));
						if(!unidirNodes->empty())
						{
							it->second->nbrsOneHop = toInt(unidirNodes);
							for(vector<string>::iterator vit=unidirNodes->begin();vit!=unidirNodes->end();++vit)
							{
								if(atoi(vit->c_str())==nodeNum)
									it->second->status = "BIDIR";
							}
						}
						/* Determine 2 hop neigbors of the bidir neigbor */
						try{
							vector<string> *bidirNodes = tokenize(subStrings(line, "BIDIR","MPR"));
							if(!bidirNodes->empty())
							{
								it->second->twoHop = toInt(bidirNodes);	
							}
							vector<string> *mprList = tokenize(subStrings(line, "MPR","\0"));
							if(!mprList->empty())
							{
								it->second->nbrMpr = toInt(mprList);
								/* Determine if I am an MPR for this node */
								if(find(it->second->nbrMpr.begin(),it->second->nbrMpr.end(), nodeNum)!=it->second->nbrMpr.end())
								{
									if(find(msList.begin(),msList.end(), it->first)==msList.end())
										msList.push_back(it->first);	
								}
							}
						}
						catch(...)
						{
						}
						
					}
				}
				/* Process TC messages */
				if(token->at(2) == "TC")
				{
					bool newInfo;
					try{
						int tcSeqNo = atoi(token->at(4).c_str());
						int srcNode = atoi(token->at(3).c_str());
						vector<string> *destList = tokenize(subStrings(line, "MS","\0"));
						if(!destList->empty())
						{
							vector<int> dList = toInt(destList);
							map<int, tcTble *>::iterator it = tcList.find(srcNode);
							if(it == tcList.end())
							{
								tcTble *tt = new tcTble();
								tt->destn = toInt(destList);
								tt->seqNo = tcSeqNo;
                                                                tt->holdTime = ts;
								tcList[srcNode]=tt;
							}
							else{
								newInfo=updateTCTable(ts, srcNode, tcSeqNo, dList);
							}
						}
						/* Check if token(1) is present in my MS List; If yes, write into from file */
						if((srcNode != nodeNum)&&(newInfo)){
						if(find(msList.begin(),msList.end(), atoi(token->at(1).c_str()))!=msList.end())
						{
							writeTCIntoFrom( nodeNum, line);
						}
						}
					}
					catch(...){
					}	
				}
				if(token->at(2) == "DATA")
				{
					int d = atoi(token->at(4).c_str());	
					int s = atoi(token->at(3).c_str());
					string msg;
					try{
						int d=5;
						while(d){
							msg.append(token->at(d));
							msg.append(" ");	
							++d;
						}
					}
					catch(...){
					}
					if(d==nodeNum) //DATA message is for me
					{
						ofstream rcv;
						rcv.open(receivedFile(nodeNum).c_str(),ios::out|ios::app);
						rcv<<msg<<"\n";
						rcv.close();
	
					}
					else
					{
						calculateRT(nodeNum);
						sendData(nodeNum,s,d,msg);
					}
						
				}
			}
			line.append(".");
		}
		if(line == "")
		{}
		else
			lines.push_back(line);
	}
	to.close();
	ofstream writeTo;
	writeTo.open(toFile(nodeNum).c_str());	
	for(int v=0;v<lines.size();++v)
	{
		writeTo<<lines.at(v)<<"\n";
	}
	writeTo.close(); 
}

void selectMprs(int n)
{
	if(nbrList.empty())
		return ;
	map<int, nbrTble *>::iterator it;
	for(it=nbrList.begin();it!=nbrList.end();++it)
	{
		if(it->second->status != "UNIDIR")
		{
			vector<int>::iterator vit, kit;
			for(vit=it->second->twoHop.begin();vit!=it->second->twoHop.end();++vit)
			{
				map<int,bool>::iterator mit;
				mit = twoHopnbrs.find(*vit);
				if(mit == twoHopnbrs.end())
				{
					it->second->status="MPR";	
					twoHopnbrs[*vit]=true;
				}
			}
			for(kit=it->second->nbrMpr.begin();kit!=it->second->nbrMpr.end();++kit)
			{
				map<int,bool>::iterator mit;
				mit = twoHopnbrs.find(*kit);
				if(mit == twoHopnbrs.end())
				{
					it->second->status="MPR";
					twoHopnbrs[*kit]=true;
				}
			}
		}
	}
}

void sendHelloMsg(int nodeNum)
{
	ofstream from;
        from.open(fromFile(nodeNum).c_str(),ios::out|ios::app);
	string hello;
	ostringstream unidir, bidir, mpr;
	map<int,nbrTble *>::iterator it;
	for(it=nbrList.begin();it!=nbrList.end();++it)
	{
		if(it->second->status == "UNIDIR")
			unidir<<it->first<<" ";
		if(it->second->status == "BIDIR")
			bidir<<it->first<<" ";
		if(it->second->status == "MPR")
		{
			mpr<<it->first<<" ";
		}

	}	
	from << "* "<<nodeNum<<" HELLO UNIDIR "<<unidir.str()<<"BIDIR "<<bidir.str()<<"MPR "<<mpr.str()<<"\n";
	from.close();
}

void sendTCMessage( int seqNo, int nodeNum)
{
	ofstream from;
        from.open(fromFile(nodeNum).c_str(),ios::out|ios::app);
	vector<int>::iterator it;
	ostringstream ms;
	if(msList.empty())
		return;
	from << "* "<<nodeNum<<" TC "<<nodeNum<<" "<<seqNo<<" MS";
        for(int i=0;i<msList.size();++i)
        {
        	from << " "<< msList.at(i);
	}		
	from <<"\n";
}

bool removeStaleNbr(int curTime)
{
	bool updateStatus = false;
	map<int, nbrTble*>::iterator it;
        for(it=nbrList.begin();it!=nbrList.end();++it)
	{
		
		int diff = curTime - (it->second->timeStamp);	
		/*If a node does not receive a hello message from one of its 
		  neighbors in 15 seconds, it assume the neighbor is dead or has moved away, 
		  so it removes it from its list of neighbors.*/
		if(diff>15)
		{
			updateStatus = true;
			nbrList.erase(it);
			vector<int>::iterator mt = remove(msList.begin(),msList.end(),it->first);
			msList.erase(mt);
		}	
	}
	return updateStatus;
}
/*If you do not receive a TC message from a node during a period of 45 seconds,
 you remove all TC information that you received from that node.*/
bool removeStaleTC(int curTime)
{
	bool updateStatus = false;
	map<int, tcTble *>::iterator tit;
	for(tit=tcList.begin();tit!=tcList.end();++tit)
	{

		int diff = curTime - (tit->second->holdTime);
		if(diff>45)
		{
			 updateStatus = true;
			tcList.erase(tit);
		}
	}	
}

int main(int argc, char* argv[])
{
	int nodeId, destnId, time;
	bool nbrUpdated=false, tcUpdated=false;
	string message;
	if(argv[1])
		nodeId = atoi(argv[1]);
	if(argv[2])
		destnId = atoi(argv[2]);
	if(argv[3])
		message = argv[3];
	if(argv[4])
		time = atoi(argv[4]);
	int seqNo = 1;
	bool sent;
	int i =0;
	while(i<120){
		parseToFile(i, nodeId);
		/* Time to send data message */
		if ((i == time)&&(!message.empty())) 
		{
			calculateRT(nodeId);
			sent = sendData(nodeId, nodeId, destnId, message);
			if(!sent)
			{
				time = time+30;
			}
		}
		selectMprs(nodeId);
		if(i%5==0)
			sendHelloMsg(nodeId);
		if(i%10==0)
		{	
			sendTCMessage( seqNo, nodeId);
			++seqNo;
		}
		nbrUpdated = removeStaleNbr(i);
		tcUpdated = removeStaleTC(i);
		if(nbrUpdated || tcUpdated)
		{
			rtList.clear();
			calculateRT(nodeId);
		}
		i=i+1;
		sleep(1);
	}
	return 0;
}
