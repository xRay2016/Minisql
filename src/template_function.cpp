#include "template_function.h"
#include "error.h"
#include "data_structure.h"

bool TuplesatisTree(Tuple & tuple, boolTree & tree, vector<int> indexes)
{
	tree.clear();
	if (tree.getAttributeList().empty()) {
		return 1;
	}
	for (int i = 0; i < indexes.size(); i++)
	{
		switch (tuple.getData()[indexes[i]].type)
		{
		case -1:
			tree.pushInt(tuple.getData()[indexes[i]].datai); break;
		case 0:
			tree.pushDouble(tuple.getData()[indexes[i]].dataf); break;
		default:
			tree.pushString(tuple.getData()[indexes[i]].datas); break;

		}
	}
	return tree.getResult();
}