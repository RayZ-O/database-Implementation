#include "HeapDBFile.h"

HeapDBFile::HeapDBFile () {

}

//The following function is used to actually create the file. The first
//parameter is a text string that tells where the binary data is 
//physically to be located. If there is meta-data(type of file, etc),
//store in an associated text file(.header). The second parameter is an
//enumeration with three possible values: heap, sorted and tree. The 
//return value is a one on success and a zero on failure
int HeapDBFile::Create (const char *f_path, fType f_type, void *startup) {
	string header = f_path;
	//create the associated text file
	header += ".header";
	
	ofstream metafile(header.c_str());
	if ( false == metafile.is_open()){
		cerr << "Can't create associated file for " << f_path << "\n";
		return 0;
	}
	metafile << (int)f_type << endl;
	metafile.close();	

	//create the binary DBfile
	curFile.Open (0, f_path);
	return 1;
}



//This function assumes that the DBFile already exists and has previous 
//been created and then closed. The one parameter to this function is 
//simply the physical location fo the file.The return value is a one on 
//success and a zero on failure(open auxiliary text file at startup)
int HeapDBFile::Open (const char *f_path) {
	//open the exist DBfile
	curFile.Open (1, f_path);
	return 1;
}

//Simply close the file. The return value is a one on success and a zero 
//on failure
int HeapDBFile::Close () {
	if(Write == curMode){
		ChModToRead();
	}
	off_t f_flag = curFile.Close ();
	if(f_flag < 0){
		return 0;
	}else{
		return 1;
	}
	
}

//This function bulk the DBFile instance from a text file, appending new
//data to it using the SuckNextRecord function from Record.h. 
//The character string passed to Load is the name of the data file to 
//bulk load
void HeapDBFile::Load (Schema &f_schema, const char *loadpath) {
	FILE *tableFile = fopen (loadpath, "r");
	Record tempRec;

	if(Read == curMode){
		ChModToWrite();
	}

	if( nullptr == tableFile ){
		cerr << "Can't open table file for " << loadpath << "\n";
		exit(1);
	}
    while (tempRec.SuckNextRecord (&f_schema, tableFile)) {
		if( 0 == curPage.Append(&tempRec) ){
			//if the page is full, create a new page
			curFile.AddPage(&curPage, curPageIndex);  
			curPage.EmptyItOut();
			if( 0 == curPage.Append(&tempRec) ){ //if fail again, record larger than page
				cerr << "Can't load " << loadpath << ", a record larger than page" << "\n";
				exit(1);
			}
			curPageIndex++;
		}		   
    }
    //the records don't fill a full page, add the page directly
    curFile.AddPage(&curPage, curPageIndex);
}

//Each DBfile instance has a pointer to the current record in the file. 
//By default, this pointer is at the first record in the file, but it 
//can move in response to record retrievals.The following function 
//forces the pointer to correspond to the first record in the file
void HeapDBFile::MoveFirst () {
	if (Write == curMode) {
		curMode = Read;
		curFile.AddPage(&curPage, curPageIndex);  
		// cerr << "Switch from write to read\n";
	}
	
	curPageIndex = 0;
	if(curFile.GetLength() > 0){//if the file is not empty
		curFile.GetPage(&curPage, curPageIndex);
	}else{ //if the file is empty, cause "read past the end of the file"
		curPage.EmptyItOut();
	}
}

//In order to add records to the file, the following function is uesd. 
//In the case of the unordered heap file this function simply adds the 
//new record to the end of the file. This function should actually 
//consume addMe, so that after addMe has been put into the file, it 
//cannot be used again
void HeapDBFile::Add (Record &addMe) {
	if(Read == curMode){
		ChModToWrite();
	}	 
	//Page is full or record larger than page
	//clean the page and add the record that failed before
	if( 0 == curPage.Append(&addMe) ){ 
		curFile.AddPage(&curPage, curPageIndex);
		curPage.EmptyItOut();		
		if( !curPage.Append(&addMe) ){ 
			cerr << "This record is larger than a DBFile page\n";
			exit (1);
		}		
		curPageIndex++;		
	}	
}

//There are two functions that allow for record retrieval from a DBFile 
//instance. The first version simply gets the next record from the file 
//and returns it to user, where "next" is defined to be relative to the 
//current location of the pointer. After the function call returns, the 
//porinter into the file is incremented, the return value is zero if and 
//only if there is not a valid record returned from the function call
int HeapDBFile::GetNext (Record &fetchme) {	
	if(Write == curMode){
		ChModToRead();
	}
//first time call this function, MoveFirst() is needed 
	while(1){//if there is no "hole" in the file, while is not necessary
		 //GetFirst consume fetchme in curPage
		 if( curPage.GetFirst(&fetchme)){//fetch record successfully
			return 1;
		 }
		 if(++curPageIndex <= curFile.GetLength() - 2){
			curPage.EmptyItOut();
			curFile.GetPage(&curPage, curPageIndex); 
			continue;
		 }
		break;
     }	
     return 0;
}

//The next version also accepts a selection predicate(CNF). It returns 
//the next record in the file that is accepted by the selection predicte
//The literal record is used to check the selection predicate, and is 
//created when the parse tree for the CNF is processed
int HeapDBFile::GetNext (Record &fetchme, CNF &cnf, Record &literal) {
	if(Write == curMode){
		ChModToRead();
	}
//first time call this function, MoveFirst() is needed 
	ComparisonEngine comp;
	while( GetNext(fetchme) ){//fetch record successfully
		if(comp.Compare(&fetchme, &literal, &cnf)){
			return 1;
		}				
	}	
	return 0;
}

void HeapDBFile::ChModToWrite() {
	curMode = Write;

	off_t length = curFile.GetLength();	
	//length is 0 for empty file, GetPage(&curPage, 0) will lead to 
	//read over bound of file	
	if( 0 == length ){
		curPageIndex = 0;      
	}
	//if the file is not empty, file length-2 is the last page
	else if(length > 0){
		curPageIndex = length - 2;
		curFile.GetPage(&curPage, curPageIndex);		
	}
	else{ //length less than 0, something wrong with current file
		cerr << "File length less than 0 in DBFile add\n";
		exit (1);
	}
}

void HeapDBFile::ChModToRead() {
	curMode = Read;
	curFile.AddPage(&curPage, curPageIndex);  
	// cerr << "Switch from write to read\n";
	MoveFirst();
}