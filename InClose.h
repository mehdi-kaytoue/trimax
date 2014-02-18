#ifndef INCLOSE_H
#define INCLOSE_H


#include <cassert>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>

#include <ctime>
#include <iomanip>

#include <boost/lexical_cast.hpp>
#include <boost/multi_array.hpp>
#include <boost/dynamic_bitset.hpp>

#include <vector>
#include <set>

#include <math.h>

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/time.h>

#define MAX_CONS 990000
#define MAX_COLS 5000
#define MAX_ROWS 5000
#define MAX_FOR_B 25000000
#define MAX_FOR_A 35000000

#ifndef BOOST_DISABLE_ASSERTS
#define BOOST_DISABLE_ASSERTS
#endif

using namespace std;

typedef boost::multi_array<double, 2> multi_double;
typedef multi_double::index indexT;

class  InClose
{

private: 

unsigned int** contextTemp;	//temporary context matrix for input of cxt file and physical sorting of columns
unsigned int** context;		//the context matrix used for concept mining. 

int mArray;					//size of bit-wise columns in temp context
int nArray;					//size of bit-wise rows in context							

int colOriginal[MAX_COLS];	//maps sorted columns to original order
int colSup[MAX_COLS];		//column support (for sorting and skipping empty columns)
int rowOriginal[MAX_ROWS];	//maps ham-sorted rows to original order

int n;						//no of attributes {0,1,...,n-1} 
int m;						//no of objects    {0,1,...,m-1} 

unsigned int pouet;



/************************ tree form of B[MAX_CONS][n] ******************************************/
short int B[MAX_FOR_B];					//intents (tree in linear array)
short int sizeBnode[MAX_CONS];			//the no. of attributes at a node (concept labels)
short int * startB[MAX_CONS];			//pointers to start of intents
int nodeParent[MAX_CONS];				//links to parent node in tree
bool Bparent[MAX_COLS];					//parent intent in Boolean form (attributes currently involved)
short int sizeB[MAX_CONS];				//intent sizes (calculated after con gen for analysis purposes)
short int * bptr;					//initialise B pointer to start of B

/************************ linear form of A[MAX_CONS][m] ***************************************/
int A[MAX_FOR_A];						//extents
int * startA[MAX_CONS];					//pointers to start of extents
int sizeA[MAX_CONS];					//extent sizes (calculated after con gen for analysis purposes)

int highc;			//highest concept number
int minIn;			//minimum size of intent (min support)
int minEx;			//minimum size of extent (min support)

int maxInt;
int maxExt;

int startCol;		//starting column for iteration (to skip empty cols)
int numcons;		//number of concepts
//char **anames;			//names of attributes	
//char **onames;			//names of objects
//char fname[100];

multi_double dataa;
vector<double> classesL;
vector<double> classesR;
int currentClass;
unsigned int theta;

//vector<boost::dynamic_bitset<> > proximities;

vector<int> beginL;
vector<int> beginR;

 public:   
unsigned int nbCandidates;
//unsigned long density;

    InClose(const multi_double &dataaaaa, vector<double> classesL, vector<double> classesR, int currentClass, vector<int> &_beginL, vector<int> &_beginR, int _minExt, int _minInt, int _maxExt,int _maxInt);
    ~InClose();
    int ttmain();
    void initContextFromTol(double left, double right, const multi_double &data, int obj, int attr);//, char** obj_names, char** attr_names);
	void InCloseS   (const int c, const int y, const bool *Bparent);	//incremental concept closure functions
	void InCloseMin(const int c, const int y, const bool *Bparent); //same as InClose() but with min support for A.
	void outputConcepts();			//output concepts using at and ob index numbers
	void outputConceptsNames();		//output concepts using at and ob names
	void outputNoConsBySize();		//"<size> - <number of concepts with intents/extents this size>"
	void sortColumns();				//sort columns in ascending order of support
	void calcAandBsizes();			//calculate sizes of extents and intents
	void outputContext();			//create sub-context and output it as cxt file
	void sortRows();     

    bool  IsCannonical(const int y, const int * endAhighc, const bool Bchild[]);
    unsigned int checkOutter();
    bool checkModus(const boost::dynamic_bitset<> &extent, const boost::dynamic_bitset<> &intent);
    void printBicluster(boost::dynamic_bitset<> extent, boost::dynamic_bitset<> intent);
    bool IsOrNot(const boost::dynamic_bitset<> &extent, const boost::dynamic_bitset<> &intent,  boost::dynamic_bitset<> &modus);
};
#endif
