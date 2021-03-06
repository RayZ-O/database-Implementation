#ifndef A22TEST_H
#define A22TEST_H
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <math.h>

#include "Pipe.h"
#include "DBFile.h"
#include "Record.h"
using namespace std;

// test settings file should have the 
// catalog_path, dbfile_dir and tpch_dir information in separate lines
const char *settings = "test.cat";

// donot change this information here
string catalog_path, dbfile_dir, tpch_dir;

extern "C" {
	int yyparse(void);   // defined in y.tab.c
}

extern struct AndList *final;

typedef struct {
	Pipe *pipe;
	OrderMaker *order;
	bool print;
	bool write;
}testutil;

class relation {

private:
	char *rname;
	const char *prefix;
	char rpath[100]; 
	Schema *rschema;
public:
	relation (char *_name, Schema *_schema, const char *_prefix) :
		rname (_name), rschema (_schema), prefix (_prefix) {
		sprintf (rpath, "%s%s.bin", prefix, rname);
	}	
	char* name () { return rname; }
	char* path () { return rpath; }
	Schema* schema () { return rschema;}
	void info () {
		cout << " relation info\n";
		cout << "\t name: " << name () << endl;
		cout << "\t path: " << path () << endl;
	}

	void get_cnf (CNF &cnf_pred, Record &literal) {
		cout << "\n enter CNF predicate (when done press ctrl-D):\n\t";
  		if (yyparse() != 0) {
			cout << " Error: can't parse your CNF.\n";
			exit (1);
		}
		cnf_pred.GrowFromParseTree (final, schema (), literal); // constructs CNF predicate
	}
	void get_sort_order (OrderMaker &sortorder) {
		cout << "\n specify sort ordering (when done press ctrl-D):\n\t ";
  		if (yyparse() != 0) {
			cout << " Error: can't parse your CNF.\n";
			exit (1);
		}
		Record literal;
		CNF sort_pred;
		sort_pred.GrowFromParseTree (final, schema (), literal); // constructs CNF predicate
		OrderMaker dummy;
		sort_pred.GetSortOrders (sortorder, dummy);
	}
};


relation *rel;

char *supplier = "supplier"; 
char *partsupp = "partsupp"; 
char *part = "part"; 
char *nation = "nation"; 
char *customer = "customer"; 
char *orders = "orders"; 
char *region = "region"; 
char *lineitem = "lineitem"; 

relation *s, *p, *ps, *n, *li, *r, *o, *c;
//added by rui, prevent memory leak
Schema *ssc, *psc, *pssc, *nsc, *lisc, *rsc, *osc, *csc;


void setup (string &tpch_dir) {
	ifstream test;
	test.open(settings);
	if(test.is_open()){
		test >> catalog_path;
		test >> dbfile_dir;
		test >> tpch_dir;
		if (! (catalog_path.size() && dbfile_dir.size() && tpch_dir.size())) {
			cerr << " Test settings file 'test.cat' not in correct format.\n";
			exit (1);
		}
	}else {
		cerr << " Test settings files 'test.cat' missing \n";
		exit (1);
	}

	cout << " \n** IMPORTANT: MAKE SURE THE INFORMATION BELOW IS CORRECT **\n";
	cout << " catalog location: \t" << catalog_path << endl;
	cout << " tpch files dir: \t" << tpch_dir << endl;
	cout << " heap files dir: \t" << dbfile_dir << endl;
	cout << " \n\n";

	//added by rui, prevent memory leak
	ssc = new Schema (catalog_path.c_str(), supplier);
	pssc = new Schema (catalog_path.c_str(), partsupp);
	psc = new Schema (catalog_path.c_str(), part); 
	nsc = new Schema (catalog_path.c_str(), nation);
	lisc = new Schema (catalog_path.c_str(), lineitem);
	rsc= new Schema (catalog_path.c_str(), region);
	osc = new Schema (catalog_path.c_str(), orders);
	csc = new Schema (catalog_path.c_str(), customer);

	s = new relation (supplier, ssc, dbfile_dir.c_str());
	ps = new relation (partsupp, pssc, dbfile_dir.c_str());
	p = new relation (part, psc, dbfile_dir.c_str());
	n = new relation (nation, nsc, dbfile_dir.c_str());
	li = new relation (lineitem, lisc, dbfile_dir.c_str());
	r = new relation (region, rsc, dbfile_dir.c_str());
	o = new relation (orders, osc, dbfile_dir.c_str());
	c = new relation (customer, csc, dbfile_dir.c_str());
}

void cleanup () {
	
	//added by rui, prevent memory leak
	delete ssc;
	delete psc;
	delete pssc; 
	delete nsc;
	delete lisc;
	delete rsc;
	delete osc;
	delete csc;

	delete s;
	delete p;
	delete ps; 
	delete n;
	delete li;
	delete r;
	delete o;
	delete c;
}

#endif
