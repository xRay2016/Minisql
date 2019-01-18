#include "parameter.h"
set<string> word_divider::keyword = {
	"select", "update", "create", "drop", "alter", "insert", "delete", "grant", "revoke", "add", "alter"
	"in", "table", "view", "all", "distinct", "from", "as", "natural", "join", "where", "group", "by",
	"having", "order", "asc", "desc", "any", "some", "union", "intersect", "except", "into", "values", "set",
	"on", "to", "cascade", "restrict", "dba", "all", "max", "min", "avg", "sum", "count", "check",
	"string","double","int","identified","by"
};

set<string> word_divider::Operator = {
	"+","*","-","/","<",">","=","!=","<=",">=","like","not","between","and","or","is","null","not"
};

set<string> word_divider::seperater = {
	"(",")",",",";","."
};

set<string> word_divider::constraints_word = {
	"not null","primary key","unique","foreign key","refernces"
};