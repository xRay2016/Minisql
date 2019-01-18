#include "lexical_analyzer.h"

bool lexical_analyzer::match_var(string str)
{
regex reg("^[a-zA-Z][a-zA-Z0-9]{0,22}$");
smatch s1;
return regex_match(str, s1, reg);
}

bool lexical_analyzer::match_const_string(string str)
{
	//regex reg("^\"[\u4e00-\u9fa5_a-zA-Z0-9]+\"$");
	regex reg("^\"(.*?)\"$");
	smatch s1;
	return regex_match(str, s1, reg);
}

bool lexical_analyzer::match_const_int(string str)
{
	regex reg2("^\\d+$");
	smatch s2;
	return regex_match(str, s2, reg2);
}

bool lexical_analyzer::match_const_double(string str)
{
	regex reg("^(-?\\d+)(\\.\\d+)?$");
	smatch s1;
	return regex_match(str, s1, reg);
}

vector<str_pair> lexical_analyzer::get_result(string str)
{
	vector<string> temp = string_dealer::divide_string(str);
	vector<str_pair> ans;
	set<string>::iterator iter;
	for (int i = 0; i < temp.size(); i++)
	{
		bool in_keyword = (word_divider::keyword.find(temp[i]) != word_divider::keyword.end());
		bool in_seperater = (word_divider::seperater.find(temp[i]) != word_divider::seperater.end());
		bool in_Operator = (word_divider::Operator.find(temp[i]) != word_divider::Operator.end());
		bool in_constraints = (word_divider::constraints_word.find(temp[i]) != word_divider::constraints_word.end());

		if (in_keyword)
		{
			str_pair stemp(temp[i], KEYWORD);
			ans.push_back(stemp);
		}
		else if (in_seperater)
		{
			str_pair stemp(temp[i], SEPERATER);
			ans.push_back(stemp);
		}
		else if (temp[i] == "not")
		{
			if (i + 1 < temp.size() && temp[i + 1] == "null")
			{
				str_pair stemp(temp[i] + " " + temp[i + 1], CONSTRAINT_WORD);
				ans.push_back(stemp);
				i++;
			}
			else
			{
				str_pair stemp(temp[i], OPERATOR);
				ans.push_back(stemp);
			}
		}
		else if (in_Operator)
		{
			str_pair stemp(temp[i], OPERATOR);
			ans.push_back(stemp);
		}
		else if (in_constraints)
		{
			str_pair stemp(temp[i], CONSTRAINT_WORD);
			ans.push_back(stemp);
		}
		else if (temp[i] == "primary" || temp[i] == "foreign")
		{
			if (i + 1 < temp.size() && temp[i + 1] == "key")
			{
				str_pair stemp(temp[i] +" "+ temp[i + 1], CONSTRAINT_WORD);
				ans.push_back(stemp);
				i++;
			}
			else
			{
				return ans;
			}
		}
		else if (match_var(temp[i]))
		{
			str_pair stemp(temp[i], VAR);
			ans.push_back(stemp);
		}
		else if (match_const_string(temp[i]))
		{
			str_pair stemp(temp[i], _STRING);
			ans.push_back(stemp);
		}
		else if (match_const_int(temp[i]))
		{
			str_pair stemp(temp[i], _INT);
			ans.push_back(stemp);
		}
		else if (match_const_double(temp[i]))
		{
			str_pair stemp(temp[i], _DOUBLE);
			ans.push_back(stemp);
		}
		else
		{
			return ans;
		}
	}
	return ans;
}