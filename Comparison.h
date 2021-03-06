#ifndef COMPARISON_H
#define COMPARISON_H

#include <fstream>
#include "Record.h"
#include "Schema.h"
#include "File.h"
#include "Comparison.h"
#include "ComparisonEngine.h"

using std::ofstream;
using std::ifstream;
// This stores an individual comparison that is part of a CNF
class Comparison {

	friend class ComparisonEngine;
	friend class CNF;

	Target operand1;		//Left, Right or Literal
	int whichAtt1;			//the index of attribute1 in the schema
	Target operand2;
	int whichAtt2;			//the index of attribute2 in the schema

	Type attType;			//Int, Double, String

	CompOperator op;		//LessThan, GreaterThan, Equals

public:
	//empty constructor
	Comparison();

	//copy constructor
	Comparison(const Comparison &copyMe);

	// print to the screen
	void Print ();
};


class Schema;

// This structure encapsulates a sort order for records
class OrderMaker {

	friend class ComparisonEngine;
	friend class CNF;
	friend class Set_member;

	int numAtts;		//the number of attributes in the ordermaker(schema)

	int whichAtts[MAX_ANDS];   //the array of index of attributes in the ordermaker(schema)
	Type whichTypes[MAX_ANDS]; //the array of type of attributes in the ordermaker(schema)

public:
	
	// creates an empty OrdermMaker
	OrderMaker();

	// create an OrderMaker that can be used to sort records
	// based upon ALL of their attributes
	explicit OrderMaker(Schema *schema);

	// construct order maker using input name list
	OrderMaker(struct NameList *sortAtts, Schema &schema);

	//overload ofstream << operator to write OrderMaker to file
	friend ofstream & operator << (ofstream &out, const OrderMaker &om);

	//overload ifstream >> operator to read OrderMaker from file
	friend ifstream & operator >> (ifstream &in, OrderMaker &om);

	//Get number of attributes of the ordermaker
	int GetNumAtts();

	//Get attribute array of the ordermaker
	int * GetAtts();

	//Get attribute Type array of the ordermaker
	Type * GetType();

	//Add a attribute to the ordermaker
	int Add(int attIndex, Type attType);

	//clear the ordermaker
	void Clear();

	// print to the screen
	void Print ();
};

class Record;

// This structure stores a CNF expression that is to be evaluated
// during query execution

class CNF {

	friend class ComparisonEngine;

	Comparison orList[MAX_ANDS][MAX_ORS];
	
	int orLens[MAX_ANDS];
	int numAnds;

public:

	CNF() = default;

	// initial default select file cnf
	explicit CNF(const Schema &schema);
	// this returns an instance of the OrderMaker class that
	// allows the CNF to be implemented using a sort-based
	// algorithm such as a sort-merge join.  Returns a 0 if and
	// only if it is impossible to determine an acceptable ordering
	// for the given comparison
	int GetSortOrders (OrderMaker &left, OrderMaker &right);

	// print the comparison structure to the screen
	void Print ();

    // this takes a parse tree for a CNF and converts it into a 2-D
    // matrix storing the same CNF expression.  This function is applicable
    // specifically to the case where there are two relations involved
    void GrowFromParseTree (struct AndList *parseTree, Schema *leftSchema, 
	Schema *rightSchema, Record &literal);

    // version of the same function, except that it is used in the case of
    // a relational selection over a single relation so only one schema is used
    void GrowFromParseTree (struct AndList *parseTree, Schema *mySchema, 
	Record &literal);

};

#endif
