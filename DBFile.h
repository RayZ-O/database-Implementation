#ifndef DBFILE_H
#define DBFILE_H

#include "TwoWayList.h"
#include "Record.h"
#include "Schema.h"
#include "File.h"
#include "Comparison.h"
#include "ComparisonEngine.h"
#include "Defs.h"
#include "GenericDBFile.h"
#include "HeapDBFile.h"
#include "SortedDBFile.h"

// stub DBFile header..replace it with your own DBFile.h 

class DBFile {

friend class HeapFileTest;
FRIEND_TEST(HeapFileTest, LoadFile);
FRIEND_TEST(HeapFileTest, MoveFirst);
FRIEND_TEST(HeapFileTest, AddRecord);

private:
	GenericDBFile *myInernalPoniter;

public:
	DBFile (); 
	~DBFile (); 

	int Create (char *fpath, fType file_type, void *startup);
	int Open (char *fpath);
	int Close ();
	void Load (Schema &myschema, char *loadpath);
	void MoveFirst ();
	void Add (Record &addme);
	int GetNext (Record &fetchme);
	int GetNext (Record &fetchme, CNF &cnf, Record &literal);

	fType StringToEnum(char *type);

};
#endif
