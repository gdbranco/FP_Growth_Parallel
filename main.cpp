#include <iostream>
#include <string>
#include <cstring>
#include <fstream>
#include <vector>
#include <map>
#include <algorithm>
#include <cstdlib>
#include "tree.hpp"
#include "transaction.hpp"

using namespace std;


template<typename T>
void print_tree(Tree_Node<T>* node);
table_transaction_t<int> toMem(string filename);

int main()
{
	table_transaction_t<int> teste = toMem("items3.db");
	header_table<int> *table;
	map<int, int> freqs =  find_frequency<int>(teste);
	std::vector<int> elements;

	freq_order_class<int> freq_obj(freqs);

	for(auto tr = teste.begin(); tr != teste.end(); tr++) {
		sort(tr->items.begin(), tr->items.end(), freq_obj);
	}

	for(auto pair = freqs.begin(); pair != freqs.end(); pair++) {
		elements.push_back(pair->first);
	}

	sort(elements.begin(), elements.end(), freq_obj);

	table = new header_table<int>();
	Tree_Node<int> *root = Tree_Node<int>::build_fptree(teste, table);
	print_tree(root);

	cout << endl << "----" << endl;

	Tree_Node<int>* nroot =	clone_tree<int>(root, table);

	//nroot = build_conditional(9, 2, nroot);
	
	list< list<int>* >* extract_list = new list<list<int>* >();
	list<int>* loko = NULL;

	build_full(5, 2, nroot, elements.rbegin(), elements.rend(), extract_list, loko);

	for(auto ele = extract_list->rbegin(); ele != extract_list->rend(); ele++) {
		cout << "{";
		for(auto it = (*ele)->rbegin(); it != (*ele)->rend(); it++) {
			cout << *it;
			if(*it != (*ele)->front()) 
			 cout << ",";
		}
		cout << "}";

	}

	if (nroot != NULL) {
		//print_tree(nroot);
		delete nroot;
		
	}

	delete root;
	
	return 0;
}
template<typename T>
void print_tree(Tree_Node<T>* node)
{
	for(auto it = node->children.begin();it!=node->children.end();it++)
	{
		cout << (*it)->data << "(" << (*it)->count << ")" << endl;
		cout << "   ";
		print_tree(*it);
		cout << "\b\b";
	}
}

table_transaction_t<int> toMem(string filename)
{
	table_transaction_t<int> memoria;
	vector<int> lista;
	int TID = 0;
	fstream sc;
	string s;
	char* pch;
	sc.open(filename.c_str());
	while(getline(sc,s))
	{
		if(s!="")
		{
			int aux;
			pch = strtok((char*)s.c_str(),"\t ,");
			while(pch!=NULL)
			{
				aux = atoi(pch);
				lista.push_back(aux);
				pch = strtok(NULL,"\t ,");
			}
			memoria.push_back(transaction_t<int>(TID,lista));
			TID++;
			lista.clear();
		}
	}
	sc.close();
	return memoria;
}


