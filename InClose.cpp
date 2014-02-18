#include "InClose.h"


/// Here we take the algorithm InClose (source code from the web
/// and we optimized it a bit
/// plus add modus computation for <<triconcepts>> and check if we can generate correspoding biclusters.



InClose::InClose(const multi_double &dataaaaa, vector<double> _classesL, vector<double> _classesR, int _currentClass, vector<int> &_beginL, vector<int> &_beginR, int _minExt, int _minInt, int _maxExt,int _maxInt)
: dataa(dataaaaa), beginL(_beginL), beginR(_beginR), n(0), m(0), highc(1), startCol(0), numcons(0), pouet(0), nbCandidates(0), maxInt(_maxInt), maxExt(_maxExt), minIn(_minInt), minEx(_minExt)
{
	bptr = B;
	currentClass = _currentClass;
	classesL = _classesL;
	classesR = _classesR;	
	//density=0;

///// FOR SAFE CODE, BUT USELESS AND TIME CONSUMING.
	//for (int i=0; i < MAX_COLS; i++)
	//  colSup[i]=0;
	
	//for (int i=0; i < MAX_CONS; i++)
	//{
	 //  nodeParent [i]=0;
	 //  sizeBnode [i]=0;
	 //  sizeB[i] = 0;
//	}
///// END SAFE CODE
}


InClose::~InClose(){}


void InClose::initContextFromTol(double left, double right, const multi_double &data, int obj, int attr)
{
     int i,j;
     m = obj ;
     n = attr ;
   

    mArray = (m-1)/32 + 1;							//calculate size of second dimension (objects) - 1bit per object
	contextTemp = new unsigned int*[n];				//create one dimension of the temporary context
	for (j = 0;j < n;j++){							//for each attribute
		contextTemp[j] = new unsigned int[mArray];	//create a row of objects
		for(i=0;i<mArray;i++) contextTemp[j][i]=0;
	}
 
 

/* create context */
	nArray = (n-1)/32 + 1;						//calculate size of second dimension (attributes) - 1bit per object
	context = new unsigned int*[m];	 			//create one dimension of the context
	for (i = 0;i < m;i++){						//for each object
		context[i] = new unsigned int[nArray];	//create a row of attributes
		for(j=0;j<nArray;j++) context[i][j]=0;
	}


	 for(i = 0; i <  m; i++) {
		for(j = 0;j < n; j++) {	
         if (data[i][j] >= left && data[i][j] <= right) {
				contextTemp[j][(i>>5)] |= (1<<(i%32));
				colSup[j]++;
				//density++;
          }
		}
	}

	//	
//	onames = new char*[m];
//	for (i = 0;i != m;i++) {
//       std::string bar = boost::lexical_cast<string>(i+1);
//       bar = "o"+bar;
//       onames[i] = (char*) bar.c_str();
//    }
//    
//	anames = new char*[n];
//	for (i = 0;i != n;i++) {
//       std::string bar = boost::lexical_cast<string>(i+1);
//       bar = "p" + bar;
//       anames[i] = (char*) bar.c_str();
//      
//    }
//    

}


int InClose::ttmain() 					
{
    
    
    
	//set initial intent parent to 'no attributes involved'
	for(int i = 0; i < MAX_COLS; i++) Bparent[i] = false;

	/* Initialse concept 0: the supremum */
	/* A[0] = {0,1,...,m-1} - supremum involves all objects */
	for(int i = 0; i < m; i++) A[i] = i;
	startA[0] = &A[0];
	startA[1] = &A[m];
	/* B[0] = {} - supremum initially has no attributes */
	sizeB[0] = 0;
	startB[0] = &B[0];
	nodeParent[0] = -1; //supremun does not have a parent

	for (int i = 0; i < n; i++) colOriginal[i] = i; //init column index array for sorting
    	sortColumns();

	/* skip empty columns: note iteration is from right to left (i.e. towards zero) */
	startCol = n - 1;
	while(colSup[startCol] == 0) startCol--;
	sortRows();

	/* mine concepts */
	InCloseMin(0, startCol, Bparent);
	calcAandBsizes();	
	//outputNoConsBySize();//and count concepts
	unsigned int  res = checkOutter();

    
     for (int i = 0; i != m; i++)
         delete[] context[i];
     delete[] context;
     return res;
} 

//********** END MAIN *********************************************************


bool InClose::IsCannonical(const int y, const int * endAhighc, const bool Bchild[])
/* y: attribute number, endAhighc: pointer to end of the next extent to be created */
/* Bchild: the current intent in Boolean form (to skip columns when checking cannonocity of any 'new' concept) */
{
	for(int q = y; q <= startCol; q++)//iterate back through conext matrix
		if((!Bchild[q]) && (context[*(startA[highc])][q>>5] & (1 << (q % 32)))){//if att not currently involved and there is a X in the 
			int i;	//first cell checked in the column (1st ob in the intersection)
			int * Ahighc = startA[highc]+1;		//point to the next cell
			for(i = endAhighc - Ahighc; i > 0; i--){//and iterate down the column to check for a match
				if(!(context[*Ahighc][q>>5] & (1 << (q % 32))))break;//if an empty cell is found, move on to check next column
				Ahighc++;//otherwise check next cell down
			}
			if(i==0) return(false);	//if intersection is found, it is not cannonical
		}
//	 numcons++;
	return(true);//if intersection is not found, it is cannonical
}

/* InCloseMin() is identical to InClose() apart from test for min support instead of test for not-empty intersection */
/* which gives a small performance benefit */
void InClose::InCloseMin(const int c, const int y, const bool *Bparent)
/* c: concept number, y: attribute number, Bparent: parent intent in Boolean form */
{	
	/* y: attribute number, endAhighc: pointer to end of the next extent to be created */
	/* Bchild: the current intent in Boolean form (to skip columns when checking cannonocity of any 'new' extent) */
	int Bchildren[MAX_COLS];							//the attributes that will spawn new concepts
	int numchildren = 0;								//the number of new concepts spawned from current one
	int Cnums[MAX_COLS];								//the concept no.s of the spawned concepts
	bool Bchild[MAX_COLS];								//the current intent in Boolean form

	for(int q = startCol; q >= 0; q--) {
		Bchild[q] = Bparent[q];	//set the child attributes to the parent ones (inheritance)
        }
	if(c){	//if not concept 0, add the last attribute to intent
		Bchild[y+1] = 1;
		*bptr = y+1;
		bptr++;
		sizeBnode[c]++;
	}
	/*********************** MAIN LOOP *********************************************************
		interate across attribute columns forming column intersetcions with current extent 
	********************************************************************************************/
	for(int j = y; j >= 0; --j)	{
		if(!Bchild[j]){									//if attribute is not an inherited one
			int * Ac = startA[c];						//pointer to start of current extent
			int * aptr = startA[highc];					//pointer to start of next extent to be created
			int sizeAc = startA[c+1]-startA[c];			//calculate the size of current extent
			/* iterate across objects in current extent to find them in current attribute column */
			for(int i = sizeAc; i > 0; i--){
				if(context[*Ac][j>>5] & (1 << (j % 32))){//context[*Ac][J] where J is byte J div 8, bit J mod 8
					*aptr = *Ac;						//add object to new extent (intersection)
					aptr++;
				}
				Ac++;									//next object
			}
			int size = aptr - startA[highc];			//calculate size of intersection
			if(size>=minEx){	//if intersection is >= min support for extents
				if(size == sizeAc){						//if extent is unchanged
					*bptr = j;							//add current attribute to intent
					bptr++;
					Bchild[j] = 1;						//record that the attribute will be inherited by any child concepts
					sizeBnode[c]++;						//increment the number of attributes at this node in the B tree
				}
				/* if the intersection is smaller test for cannonicity */
				else if(IsCannonical(j+1,aptr,Bchild)){	//if the intersection is a new extent, note the child for later spawning:
            		Bchildren[numchildren] = j;			//note where (attribute column) it was found,
					Cnums[numchildren++] = highc;		//note the concept number,
					nodeParent[highc] = c;				//note the parent concept number and
					startA[++highc] = aptr;				//note the start of the new extent in A.
				}
			} 
		}
	}
	/* spawn child concepts from this parent */
	for(int i = numchildren-1; i >= 0 ; i--){
		startB[Cnums[i]] = bptr;						//set the start of the intent in B tree
		InCloseMin(Cnums[i], Bchildren[i]-1, Bchild);	//close the child concept (next closure starts at j-1 to
    }	
      												//avoid having to create this intersection again)
}




void InClose::calcAandBsizes()	//calculate sizes of extents and intents of each concept
{	
	int highsizeB = 0;						//largest intent generated
	int c = 0;	
	int i = 0;


	for(c = 0; c < highc; c++){				//for each concept
		sizeA[c] = startA[c+1] - startA[c];		//calculate size of extent
		/* calculate size of intent by traversing B tree */
		i = c;
		while(i >= 0){
			sizeB[c] = sizeB[c] + sizeBnode[i];	//add number of attributes at node i to size
			i = nodeParent[i];					//get the next node (parent)
		}
		if(sizeB[c] > highsizeB)
			 highsizeB = sizeB[c];
	}
	
	///* add empty infimum if necessary */
	//if(highsizeB < n) {
	//	sizeBnode[highc] = n;
	//	nodeParent[highc] = -1;
	//	startB[highc] = bptr;
	//	for(int j = 0; j < n; j++){
	//		*bptr = j;							
	//		bptr++;
	//	}
	//	sizeA[highc] = 0;
	//	sizeB[highc] = n;
	//	highc++;
	//}
}




void InClose::sortColumns()
{
 	std::sort( colSup, colSup + n );
	std::sort( colOriginal, colOriginal + n );

	int i,j;   //temp is now unsued.

	/* rewrite sorted context (physical sort) */
	int tempColNums[MAX_COLS];
	int rank[MAX_COLS];
	
	for(j = 0; j < n; j++){
		tempColNums[j]=colOriginal[j]; //use original col nos to index the sort
		rank[colOriginal[j]]=j;			//record the ranking of the column
	}
	for(j = 0; j < n-1; j++){
		for(i = 0; i < mArray; i++){
			unsigned int temp = contextTemp[j][i];
			contextTemp[j][i] = contextTemp[tempColNums[j]][i];
			contextTemp[tempColNums[j]][i] = temp;
		}
		tempColNums[rank[j]]=tempColNums[j];		//make note of where swapped-out col has moved to using its rank
		rank[tempColNums[j]]=rank[j];
	}

	/* write translated context */
	for(i=0;i<m;i++){
		for(j=0;j<n;j++){
			if(contextTemp[j][(i>>5)]&(1<<(i%32)))
				context[i][(j>>5)] |= (1<<(j%32));
		}
	}
	for (i=0; i< n ;i++)
	    delete[] contextTemp[i];
	delete[]  contextTemp; 
}

void InClose::sortRows()
{
	//initialise row pointers and original row indexes
	for(int i=0;i<m;i++)
		rowOriginal[i]=i;

	//quickHamsort(0, m-1, rowOriginal); <==== this is not efficient at all, use std::sort instead.

  	std::sort( rowOriginal, rowOriginal + m-1 );

	
	/* rewrite sorted context (physical sort) */
	int tempRowNums [MAX_ROWS];
    	int rank [MAX_ROWS]; 

	for(int i = 0; i < m; i++){
		tempRowNums[i]=rowOriginal[i];	//use original row nos to index the sort
		rank[rowOriginal[i]]=i;			//record the ranking of the row
	}
	
	for(int i = 0; i < m-1; i++){
		for(int j = 0; j < nArray; j++){
			unsigned int temp = context[i][j];
			context[i][j] = context[tempRowNums[i]][j];
			context[tempRowNums[i]][j] = temp;
		}
		tempRowNums[rank[i]]=tempRowNums[i];		//make note of where swapped-out row has moved to using its rank	
		rank[tempRowNums[i]]=rank[i];
	}
}



void InClose::outputNoConsBySize()
{
	/* create arrays to store the number of concepts of each size */
	int noOfBSize[n+1];
	int noOfASize[m+1];

	int v;
	for (v = 0 ; v < n+1 ; v++)
		noOfBSize[v]=0;

	for (v = 0 ; v < m+1 ; v++)
		noOfASize[v]=0;

	/* calculate number of concepts of each size satisfying min supports */
	for(int c = 0; c < highc; c++){
		if(sizeB[c] >= minIn && sizeA[c] >= minEx){
			noOfBSize[sizeB[c]]++;
			noOfASize[sizeA[c]]++;
			numcons++;
		}
	}
	if(noOfBSize[n]==0 && minIn==0){
		noOfBSize[n] = 1;
		numcons++;
	}
}



void InClose::outputConcepts() //output concepts to file
{
	string strData;
	ofstream outlist;	
	outlist.open ("result.txt", ios_base::app);

	/* for each concept */ 
	for(int c = 0; c < highc; c++)
	{
		if(sizeB[c] >= minIn && sizeA[c] >= minEx)
		{
			/* traverse B tree to obtain intent */ 
			int i = c;									//set i to concept
			while(i >= 0){								//when i==-1, head node is reached
				short int *bptr = startB[i];			//point to start of attributes in first node of this intent
				for(int j = 0; j < sizeBnode[i]; j++){	//iterate attributes at this node
	 
					strData+="p";
					strData+=  boost::lexical_cast<string, int>(colOriginal[*bptr]+1);
					strData+=",";
					bptr++;
				}
				i = nodeParent[i];						//point to next node
			}	
			strData+= "\t";				 				//<tab> between intent and extent
			/* obtain extent from A */
			int * aptr = startA[c];						//point to start of extent
			for(int j = 0; j < sizeA[c]; j++){			//iterate objects
			
				strData += "g";
				strData += boost::lexical_cast<string, int>(rowOriginal[*aptr]+1);
				strData+=",";
				aptr++;
			}
			strData+= "\n";
		}
		 outlist << strData;
		strData="";
	}
	    outlist.close();
}


unsigned int InClose::checkOutter()
{

	//ofstream outlist;	
	//outlist.open ("result.txt", ios_base::app);

	unsigned int nbBiclusters = 0;
	/* for each concept */ 
	for(int c = 0; c < highc; c++)
	{
		boost::dynamic_bitset<> extent (m);
		boost::dynamic_bitset<> intent (n);
		if(sizeB[c] >= minIn && sizeA[c] >= minEx &&  sizeB[c] <= maxInt  && sizeA[c] <= maxExt)
		{
			/* traverse B tree to obtain intent */ 
			int i = c;									//set i to concept
			while(i >= 0){								//when i==-1, head node is reached
				short int *bptr = startB[i];			//point to start of attributes in first node of this intent
				for(int j = 0; j < sizeBnode[i]; j++){	//iterate attributes at this node
					intent[colOriginal[*bptr]] = 1;
					bptr++;
				}
				i = nodeParent[i];						//point to next node
			}	
			/* obtain extent from A */
			int * aptr = startA[c];						//point to start of extent
			for(int j = 0; j < sizeA[c]; j++){			//iterate objects
				extent[rowOriginal[*aptr]]=1;
				aptr++;
				}
		

		if (checkModus(extent,intent))
		{
			 nbBiclusters++;

            
            //// WRITE OUTPUT
            
			string strData = "Objects {";
			for (boost::dynamic_bitset<>::size_type i = 0; i < extent.size(); i++)
				if (extent[i]) strData += boost::lexical_cast<string, int>(i) + " ";
			strData += "}, Attributes {";
			for (boost::dynamic_bitset<>::size_type i = 0; i < intent.size(); i++)
				if (intent[i]) strData += boost::lexical_cast<string, int>(i) + " ";
			strData += "}\n";
			cout << strData;
        
		
		}
		else nbCandidates++;
		}
	}
	//outlist.close();
	return nbBiclusters;
}


bool InClose::checkModus(const boost::dynamic_bitset<> &extent, const boost::dynamic_bitset<> &intent)
{
	boost::dynamic_bitset<> modus  (classesL.size());
	unsigned int modSize=0;

	int lastCondition = -1;
	
	bool ok = true;
	size_t i, j;


	double min = classesR[classesR.size()-1];
	double max = classesL[0];

	for (  i = extent.find_first(); i  != boost::dynamic_bitset<>::npos; i = extent.find_next(i))
	{
			for (  j = intent.find_first(); j !=  boost::dynamic_bitset<>::npos; j= intent.find_next(j))
			{
				if (dataa[i][j] < min) min = dataa[i][j];
				if (dataa[i][j] > max) max = dataa[i][j];
			}
	}
	
	for(int l = beginL[currentClass]; l!= classesL.size();l++)
	{
		if (classesL[l] <= min && classesR[l] >= max)
		{
			modus[l] = 1; modSize++; lastCondition=l;
		}
	}
	



	/*for(int l = beginL[currentClass]; l!= classesL.size();l++)
	///////for( int l = beginL[currentClass]; l <= beginR[currentClass] ;l++)	// this optimization seems useless ...
	{

		ok = true;
		for (  i = extent.find_first(); i  != boost::dynamic_bitset<>::npos && ok; i = extent.find_next(i))
		{
			for (  j = intent.find_first(); j !=  boost::dynamic_bitset<>::npos && ok; j= intent.find_next(j))
			{
				if ( dataa[i][j] > classesR[l] || dataa[i][j] < classesL[l] )
					ok = false;
			}
		}
		
		if (ok) 
		{
			modus[l] = 1; modSize++; lastCondition=l;
		} 
		else {
			if (lastCondition != -1) break;
		}
			
	}*/


	if (modSize == 1)
	{
		return true;
	}
	else if (lastCondition == currentClass) // to avoid redundancy // directly on numerial data to spare memory
	{
		return IsOrNot(extent, intent, modus);
	}
	else return false; // redundant pattern	
	return true;
}


bool InClose::IsOrNot(const boost::dynamic_bitset<> &extent, const boost::dynamic_bitset<> &intent,  boost::dynamic_bitset<> &modus)
{
	for (size_t modIndex = modus.find_first(); modIndex !=  boost::dynamic_bitset<>::npos && modIndex < currentClass; modIndex= modus.find_next(modIndex))

		//for (int modIndex = 0; modIndex < currentClass; modIndex++)
		{
		//if (modus[modIndex])			
		//{
			boost::dynamic_bitset<> C2 (m);// = extent;
			
			for (int i = 0 ; i != m ; i ++) // for each obj
			{
				bool ok = true;// has the obj all attr from intent?
				//for (boost::dynamic_bitset<>::size_type j = 0; j < intent.size() && ok; j++)
				for (size_t j = intent.find_first(); j !=  boost::dynamic_bitset<>::npos && ok; j= intent.find_next(j))
					if (!(dataa[i][j] <= classesR[modIndex] && dataa[i][j] >= classesL[modIndex]))
						ok = false;
				if (ok) C2[i]=1;
				if (ok != extent[i]) return false;
			}

			//if (extent != C2) return false;
			//boost::dynamic_bitset<> D2 = intent;	

			for (int i = 0 ; i != n ; i ++) 
			{
				bool ok = true;
				for (size_t j = C2.find_first(); j !=  boost::dynamic_bitset<>::npos && ok; j= C2.find_next(j))
				//for (boost::dynamic_bitset<>::size_type j = 0; j < C2.size() && ok; j++)
					if (   !(dataa[j][i] <= classesR[modIndex] && dataa[j][i] >= classesL[modIndex]))
						ok = false;

				//if (ok) D2[i]=1;
				if (ok != intent[i]) return false;
			}
			//if (intent != D2) return false;
		//}
		}

return true;
}

void InClose::printBicluster(boost::dynamic_bitset<> extent, boost::dynamic_bitset<> intent) // todo: use bitset find first, next.
{
	cout << "{";
	for (boost::dynamic_bitset<>::size_type i = 0; i < extent.size(); i++)
		if (extent[i]) cout << "g" << i << ",";
	cout << "},{";
	for (boost::dynamic_bitset<>::size_type i = 0; i < intent.size(); i++)
		if (intent[i]) cout << "m" << i << ",";
	cout << "}";
	
}

//void InClose::outputConceptsNames() //output concepts to file using object and attribute names
//{
//	cout << "\n\nOutputting concepts to file...";
//	string strData;
//	
//	
//		ofstream outlist;	
//	    outlist.open ("result.txt", ios_base::app);
//
//	/* for each concept */
//	for(int c = 0; c < highc; c++){
//		if(sizeB[c] >= minIn && sizeA[c] >= minEx){
//			/* traverse B tree to obtain intent */
//			int i = c;									//set i to concept
//			while(i >= 0){								//when i==-1, head node is reached
//				short int * bptr = startB[i];			//point to start of attributes in first node of this intent
//				for(int j = 0; j < sizeBnode[i]; j++){	//iterate attributes at this node
//					strData+=anames[colOriginal[* bptr]];
//					strData+=",";
//					bptr++;
//				}
//				i = nodeParent[i];						//point to next node
//			}	
//			strData+="\t";								//<tab> between intent and extent
//			
//			int * aptr = startA[c];						//point to start of extent
//			for(int j = 0; j < sizeA[c]; j++){			//iterate objects
//				strData+=onames[rowOriginal[*aptr]];
//				strData+=",";
//				aptr++;
//			}
//			strData+= "\n";	
//		}
//	     outlist << strData;
//		strData="";
//	}
//    outlist.close();
//}
