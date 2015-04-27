#include <iostream>
#include <stdlib.h>
#include "PlanTree.h"
#include "Statistics.h"
#include "Tables.h"
#include "time.h"

extern "C" {
	struct YY_BUFFER_STATE *yy_scan_string(const char*);
	int yyparse(void);   // defined in y.tab.c
}

char *cnf[] = { // q0
				"SELECT SUM (ps.ps_supplycost), s.s_suppkey " 
				"FROM part AS p, supplier AS s, partsupp AS ps " 
				"WHERE (p.p_partkey = ps.ps_partkey) AND (s.s_suppkey = ps.ps_suppkey) AND (s.s_acctbal > 2500.0) "
				"GROUP BY s.s_suppkey",
				// q1
				 "SELECT SUM (c.c_acctbal), c.c_name "
				"FROM customer AS c, orders AS o "
				"WHERE (c.c_custkey = o.o_custkey) AND (o.o_totalprice < 10000.0) "
				"GROUP BY c.c_name",
				// q2
				"SELECT l.l_orderkey, l.l_partkey, l.l_suppkey "
				"FROM lineitem AS l "
				"WHERE (l.l_returnflag = 'R') AND (l.l_discount < 0.04 OR l.l_shipmode = 'MAIL')",
				// q3
	 			"SELECT DISTINCT c1.c_name, c1.c_address, c1.c_acctbal "
				"FROM customer AS c1, customer AS c2 "
				"WHERE (c1.c_nationkey = c2.c_nationkey) AND (c1.c_name ='Customer#000070919')",
				// q4
				"SELECT SUM(l.l_discount) "
				"FROM customer AS c, orders AS o, lineitem AS l "
				"WHERE (c.c_custkey = o.o_custkey) AND (o.o_orderkey = l.l_orderkey) "
				"AND (c.c_name = 'Customer#000070919') AND (l.l_quantity > 30.0) AND (l.l_discount < 0.03)",
				// q5
				"SELECT l.l_orderkey "
				"FROM lineitem AS l "
				"WHERE (l.l_quantity > 30.0)",
				// q6
				"SELECT DISTINCT c.c_name "
				"FROM lineitem AS l, orders AS o, customer AS c, nation AS n, region AS r "
				"WHERE (l.l_orderkey = o.o_orderkey) AND (o.o_custkey = c.c_custkey) AND "
				"(c.c_nationkey = n.n_nationkey) AND (n.n_regionkey = r.r_regionkey)",
				// q7
				"SELECT l.l_discount "
				"FROM lineitem AS l, orders AS o, customer AS c, nation AS n, region AS r "
				"WHERE (l.l_orderkey = o.o_orderkey) AND (o.o_custkey = c.c_custkey) AND (c.c_nationkey = n.n_nationkey) AND "
	  			"(n.n_regionkey = r.r_regionkey) AND (r.r_regionkey = 1) AND (o.o_orderkey < 10000)",
	  			// q8
				"SELECT SUM (l.l_discount) "
				"FROM customer AS c, orders AS o, lineitem AS l "
				"WHERE (c.c_custkey = o.o_custkey) AND (o.o_orderkey = l.l_orderkey) AND "
	  			"(c.c_name = 'Customer#000070919') AND (l.l_quantity > 30.0) AND (l.l_discount < 0.03)",
	  			// q9
				"SELECT SUM (l.l_extendedprice * l.l_discount) "
				"FROM lineitem AS l "
				"WHERE (l.l_discount<0.07) AND (l.l_quantity < 24.0)" };



int main(int argc, char *argv[]) {
	if (argc < 2) {
		cerr << "You need to supply me the query number to run as a command-line arg.." << endl;
		cerr << "Usage: ./test.out [0-11] >" << endl;
		exit (1);
	}
	int qindx = atoi (argv[1]);
	if (qindx >=0 && qindx < 10) {
		Statistics s;
		Tables table;
		int outMode = STDOUT_;
		s.Read("STATS.txt");	
		table.CreateAll();
		// table.LoadAll();			
		yy_scan_string(cnf[qindx]);
		yyparse();
		PlanTree planTree(s, table, outMode);
		if (DISCARD == planTree.GetPlanTree()) {
			cerr << "ERROR: Can not grow plan tree!\n";
			return 1;
		}		
		planTree.Print();	
	}
	else {
		cout << " ERROR!!!!  Usage: ./test.out [0-11] >\n";
	}

}