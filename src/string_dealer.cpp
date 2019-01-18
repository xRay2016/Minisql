#include "string_dealer.h"

string string_dealer::get_string(string str)//ȥ�����з����ҰѴ�д����ĸ��ΪСд
{
	int length = str.length();
	bool is_word = false;
	for (int i = 0; i < str.length(); i++)
	{
		if (str[i] == '\n')
		{
			str[i] = ' ';
		}
		else if (str[i] <= 'Z'&&str[i] >= 'A')
		{
			str[i] = str[i] - 'A' + 'a';
		}
		else if (str[i] == '"')
		{
			while (i + 1 < str.length() && str[i + 1] != '"')
			{
				i++;
			}
			i++;
		}
		else if (str[i] == ')' || str[i] == '(' || str[i] == ',' || str[i] == '=' || str[i] == '+'||str[i]==';')
		{
			str.insert(i + 1, " ");
			str.insert(i, " ");
			i = i + 2;
		}
		else if (str[i] == '-' || str[i] == '*' || str[i] == '\\' || str[i] == ':')
		{
			str.insert(i + 1, " ");
			str.insert(i, " ");
			i = i + 2;
		}
		else if (str[i] == '.')
		{
			if (i + 1 < str.length() && !is_number(str[i + 1]))
			{
				str.insert(i + 1, " ");
				str.insert(i, " ");
				i = i + 2;
			}
		}
		else if (str[i] == '<' || str[i] == '>')
		{
			if (i + 1 < str.length() && str[i + 1] == '=')
			{
				str.insert(i + 2, " ");
				str.insert(i, " ");
				i += 3;
			}
			else if (i + 1 < str.length() && str[i + 1] != '=')
			{
				str.insert(i + 1, " ");
				str.insert(i, " ");
				i = i + 2;
			}
		}
		else if (str[i] == '!')
		{
			if (i + 1 < str.length() && str[i + 1] == '=')
			{
				str.insert(i + 2, " ");
				str.insert(i, " ");
				i += 3;
			}
		}
	}
	return str;
}

vector<string> string_dealer::divide_string(string str)
{
	string temp = get_string(str);
	temp = trim(temp);
	vector<string> ans = splitEx(temp, " ");
	return ans;
}

vector<string> string_dealer::splitEx(const string& src, string sep)
{
	vector<string> strs;
	int seplen = sep.size();//�ָ��ַ����ĳ���,�����Ϳ���֧���硰,,�����ַ����ķָ���
	int lastpos = 0, index = -1;
	while (-1 != (index = src.find(sep, lastpos)))
	{
		if (src.substr(lastpos, index - lastpos) != "")
			strs.push_back(src.substr(lastpos, index - lastpos));
		lastpos = index + seplen;
	}
	string laststr = src.substr(lastpos);//��ȡ���һ���ָ����������
	if (!laststr.empty())
		strs.push_back(laststr);//������һ���ָ����������ݾ����
	return strs;
}

string& string_dealer::trim(string &str)
{
	if (str.empty())
	{
		return str;
	}
	str.erase(0, str.find_first_not_of(" ")); //ȥ����߿ո�
	str.erase(str.find_last_not_of(" ") + 1);//ȥ���ұ߿ո�
	return str;
}

bool string_dealer::is_number(char c)
{
	if (c == '1' || c == '2' || c == '3' || c == '4' || c == '5'||c=='0')
	{
		return true;
	}
	else if (c == '6' || c == '7' || c == '8' || c == '9')
	{
		return true;
	}
	return false;
}
