#ifndef PLAN_TREE_H
#define PLAN_TREE_H

#include "Statistics.h"
#include "Schema.h"
#include "DBFile.h"
#include "RelOp.h"
#include "ParseInfo.h"
#include <cstring>
#include <string>

typedef struct AndList* Predicate;

typedef struct {
	int left;
	int right;
	Predicate p;
}JoinRelInfo;

typedef struct {
	std::vector<int> relNo;
	Predicate p;
}CrossSelectInfo;

typedef struct {
	DBFile *dbf;
	Schema *sch;
	Predicate initPred;
}TableInfo;

class PlanNode;
class JoinNode;

class PlanTree {
private:
	PlanNode *root;
	Statistics &s;
	int numToJoin;
	// store all the alias relation name
	std::vector<char*> relNames;
	// hashtable for alias and corresponding origin realtion name
	std::unordered_map<char *, char *> tableList;
	std::unordered_map<std::string, TableInfo> tableInfo;
	std::unordered_map<char*, Predicate> selectList;
	std::vector<JoinRelInfo> joinList;
	std::vector<CrossSelectInfo> crossSelectList;
	//store builded node expect first 2 single relation, which stores in nodeList
	std::vector<PlanNode*> bulidedNodes;

	void SeparatePredicate(struct AndList *parseTree);
	int CheckCrossSelect(std::vector<CrossSelectInfo> &csl, std::vector<int> &joinedTable);	
	void InitPredicate(char *attName, Predicate &initPred);
	void CreateTable(char* tableName);
	Type GetSumType(Function *func);

	void GrowSelectFileNode(std::vector<PlanNode*> &nodeList);
	void GrowSelectPipeNode(std::vector<int> &joinedTable, std::vector<char*> &minList, int numOfRels);
	void GrowRowJoinNode(std::vector<int> &joinedTable, std::vector<char*> &minList, std::vector<PlanNode*> &nodeList);
	void GrowCookedJoinNode(std::vector<int> &joinedTable, std::vector<char*> &minList, std::vector<PlanNode*> &nodeList, int numOfRels);
	void GrowProjectNode (struct NameList *attsToSelect);
	void GrowDuplicateRemovalNode ();
	void GrowSumNode (struct FuncOperator *finalFunction);
	void GrowGroupByNode (struct FuncOperator *finalFunction, struct NameList *groupingAtts);
	void GrowWriteOutNode(const char* filename);

	void BuildUnaryNode(PlanNode *child, PlanNode *parent);
	void BuildBinaryNode (PlanNode *lchild, PlanNode *rchild, JoinNode *parent, int outID);
	void PrintNode(PlanNode *root);

public:
	PlanTree(Statistics &stat);
	void BuildTableList(struct TableList *tables);
	void GetJoinOrder(struct AndList *pAnd);
	void PrintTree();	
};

enum NodeType { Unary, Binary };

class PlanNode {
public:
	PlanNode *parent;
	NodeType type;	
	PlanNode *child;
	int inPipeID;
	int outPipeID;
	Schema *outSch;
	double numTuples;
	PlanNode() : parent(nullptr), type(Unary), child(nullptr), inPipeID(-1), outPipeID(-1) { }
	virtual ~PlanNode() {
		delete outSch;
		outSch = nullptr;
		if(child) {
			delete child;
			child = nullptr;
		}
	}
	virtual void Print() = 0;
};

class SelectFileNode : public PlanNode { 
private:
	DBFile *inFile;
	CNF *selOp;
	Record *literal;
public:
	SelectFileNode(DBFile *dbf, CNF *cnf, Record *rec) : inFile(dbf), selOp(cnf), literal(rec) { }
	SelectFileNode(const SelectFileNode &sf) = delete;
	SelectFileNode & operator =(const SelectFileNode &sf) = delete;
	void Print();
	~SelectFileNode() {
		delete selOp;
		delete literal;
	}
};

class SelectPipeNode : public PlanNode  {
private:
	CNF *selOp;
	Record *literal;
public:
	SelectPipeNode(CNF *cnf, Record *rec) : selOp(cnf), literal(rec) { }
	SelectPipeNode(const SelectPipeNode &sf) = delete;
	SelectPipeNode & operator =(const SelectPipeNode &sf) = delete;
	void Print();
	~SelectPipeNode() {
		delete selOp;
		delete literal;
	}
};

class ProjectNode : public PlanNode { 
private:
	int *keepMe;
	int numAttsInput;
	int numAttsOutput;
public:
	ProjectNode(int *atts, int numIn, int numOut) : keepMe(atts), numAttsInput(numIn), numAttsOutput(numOut) { }
	ProjectNode(const ProjectNode &pn) = delete;
	ProjectNode & operator =(const ProjectNode &pn) = delete;
	void Print();
	~ProjectNode() {
		delete keepMe;
	}
};

class JoinNode : public PlanNode { 
private:
	CNF *selOp;
	Record *literal;
public:
	PlanNode *rchild;
	int rightInPipeID;
	JoinNode(CNF *cnf, Record *rec) : selOp(cnf), literal(rec), rchild(nullptr), rightInPipeID(-1) { }
	JoinNode(const JoinNode &jn) = delete;
	JoinNode & operator = (const JoinNode &jn) = delete;
	void Print();
	~JoinNode() {
		delete selOp;
		delete literal;
		if(rchild) {
			delete rchild;
			child = nullptr;
		}
	}
};

class DuplicateRemovalNode : public PlanNode {
private:
	Schema *mySchema;
public:
	DuplicateRemovalNode(Schema * sch) : mySchema(sch) { }
	void Print();
	~DuplicateRemovalNode() {
		// nothing
	}
};

class SumNode : public PlanNode {
private:
	Function *computeMe;
public:	
	SumNode(Function *func) : computeMe(func) { }
	SumNode(const SumNode &sn) = delete;
	SumNode & operator = (const SumNode &sn) = delete;
	void Print();
	~SumNode() {
		delete computeMe;
	}
};

class GroupByNode : public PlanNode {
private:
	OrderMaker *groupAtts;
	Function *computeMe;
public:
	GroupByNode(OrderMaker *om, Function *func) : groupAtts(om), computeMe(func) { }
	GroupByNode(const GroupByNode *gn) = delete;
	GroupByNode & operator = (const GroupByNode *gn) = delete;
	void Print();
	~GroupByNode() {
		delete groupAtts;
		delete computeMe;
	}
};

class WriteOutNode : public PlanNode {
private:
	FILE *outFile;
	Schema *mySchema;
	std::string filename;
public:
	WriteOutNode(FILE *f, Schema *sch, std::string name) : outFile(f), mySchema(sch), filename(name) { }
	WriteOutNode(const WriteOutNode &wn) = delete;
	WriteOutNode & operator =(const WriteOutNode &wn) = delete;
	void Print();
	~WriteOutNode() {
		// nothing
	}
};


#endif