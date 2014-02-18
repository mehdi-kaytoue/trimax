
//using namespace std;
#include "InClose.h"


double mstimer(clock_t tstart, clock_t tstop){
return 1000*(double)(tstop-tstart)/(double)(CLOCKS_PER_SEC);
}


///// USAGE : ./trimax datafile integer-theta integer-min-extent-size integer-min-intent-size integer-max-extent-size integer-max-intent-size

// minExt and minInt should be set at "1" for having same results that the greedy algo.

int main(int argc, char** argv)
{
    
clock_t tstart=0, tstop=0;
//
// the strart time counter
tstart = clock();

    string a (argv[2]);
    double theta = boost::lexical_cast<double>(a);
    ifstream inFile ( argv[1], ifstream::in); 
    string line;
    int linenum = 0;
    string item;
    
    int nbObj  = 0;
    int nbAttr = 0;

    string me (argv[3]);
    string mi (argv[4]);
    string mae (argv[5]);
    string mai (argv[6]);

    int minExt =  boost::lexical_cast<int>(me);
    int minInt =  boost::lexical_cast<int>(mi);
    int maxExt =  boost::lexical_cast<int>(mae);
    int maxInt =  boost::lexical_cast<int>(mai);

        
    /// couting objects, attributes ///
    getline (inFile, line);
    nbAttr = std::count(line.begin(), line.end(), '\t');
    while (getline (inFile, line))
          nbObj++;
    cout << "Found " << nbObj << " objects and " << nbAttr << " attributes." << endl;
    
    /// init data ///
    std::set<double> sorted_domain;
   
    
    //string objNames[nbObj];
    //string attrNames[nbAttr];
    vector<string> objNames (nbObj);
    vector<string> attrNames (nbAttr);
    
    multi_double data(boost::extents[nbObj][nbAttr]);
     

    /// reading data ///
    inFile.clear(); 
    inFile.seekg (0, std::ios::beg);
    std::istringstream stm;

    int itemnum = 0;
    getline (inFile, line);
    istringstream linestream(line);
    getline (linestream, item, '\t');
    while (getline (linestream, item, '\t'))
    {
          attrNames[itemnum++] = item;
    }

    while (getline (inFile, line))
    {
        istringstream linestream(line);
        itemnum = 0;
        getline (linestream, item, '\t');
        objNames[linenum] = item;
	
        while (getline (linestream, item, '\t'))
        {
          data[linenum][itemnum] =  boost::lexical_cast<double>(item);
          sorted_domain.insert(data[linenum][itemnum]);
          itemnum++;
        }
        linenum++;
    }
    inFile.close();
      


    // Building domain  of value
     vector<double> W;
     set<double>::iterator myIterator;
     for(myIterator =  sorted_domain.begin();
         myIterator !=  sorted_domain.end();
             myIterator++)
     {
            W.push_back(*myIterator);
            //cout << *myIterator << endl;
     }

    vector<double> classesL;
    vector<double> classesR;
    
    /// Computing blocks of tolerance over W ///
    double curL,curR;
    unsigned int i, j;
    double k = -999999999;
    for (i = 0 ; i != W.size(); i++)
    {
       	curL= W[i];
        curR= W[i];
	for (j = i; j != W.size(); j++) 
	{
	if (W[j] - curL <= theta)
         		    curR = W[j];
			else break;
		}
		if (! (curR<= k) ) 
                {
                   classesL.push_back(curL);
                   classesR.push_back(curR);
                }
		k = curR;
    }


//// computing 'proximity' of a class, ie classes having a non empty intersection with the current
vector<int> beginL (classesR.size());
vector<int> beginR (classesR.size());

for (unsigned int l=0; l != classesL.size(); l++)
	{
		beginL[l] = -1;
		beginR[l] = -1;

		for (unsigned int prox=0; prox != classesL.size(); prox++)
		{
			double interL = (classesL[l] > classesL[prox] ? classesL[l] : classesL[prox]);
			double interR = (classesR[l] < classesR[prox] ? classesR[l] : classesR[prox]);
			if (interR >= interL) {
				if (beginL[l] == -1) beginL[l] = prox;
				beginR[l] = prox;
			}
		}
	}

   /// Building and mining each context. ///
   unsigned int totalBiclusters = 0;
   unsigned int totalCandidates = 0;
 //  unsigned long density = 0;

   for (unsigned int l = 0; l != classesL.size() ; l++) 
   {
       //cout << "[" << classesL[l] << ";" << classesR[l] << "], " << endl;
       InClose *algo;
       algo = new InClose(data,classesL, classesR, l, beginL, beginR, minExt, minInt, maxExt, maxInt);
       algo->initContextFromTol(classesL[l], classesR[l], data, nbObj, nbAttr);
       totalBiclusters += algo->ttmain();
      totalCandidates += algo->nbCandidates;
       //algo->outputConcepts();
      // density += algo->density;
       delete algo;
   } 
  tstop = clock();

// For experiments.
// cout << totalBiclusters << " total maximal biclusters." << endl;
// cout << totalCandidates << " candidates generated as either non maximal or redundant biclusters." << endl;

//double c = boost::lexical_cast<double>( classesL.size());
//double o = boost::lexical_cast<double>( nbObj);
//double at = boost::lexical_cast<double>( nbAttr);
//double d = boost::lexical_cast<double>( density);
//cout << d / c / (o * at);

cout << totalBiclusters << " max. biclusters\n"
     << totalCandidates << " candidates\n"
     << mstimer(tstart,tstop) << " ms\n" ;

/**cout << theta  << "\t"
     << nbObj  << "\t"
     << minExt << "\t"
     << maxExt << "\t"
     << minInt << "\t"
     << maxInt << "\t"
     << totalBiclusters << "\t"
     << totalCandidates << "\t" 
     << classesL.size() << "\t"
     << mstimer(tstart,tstop) << "\n";
**/
   return EXIT_SUCCESS;
}






