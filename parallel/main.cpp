#include <iostream>
#include <string>
#include <cstring>
#include <fstream>
#include <algorithm>
#include <vector>
#include <iterator>
#include <list>
#include <map>
#include <cstdlib>
#include "fp_tree.hpp"
#include "transaction.hpp"

using namespace std;


template<typename T>
void print_tree(Tree_Node<T>* node);
table_transaction_t<int> toMem(string filename);

int main()
{
	int supp=2;
	//int supp=500;
	cout << "Iniciando programa" << endl;
	cout << "Passando para a memória" << endl;

	//table_transaction_t<int> tr_table = toMem("T40I10D100K.db");
	table_transaction_t<int> tr_table = toMem("items3.db");
	header_table<int> *table;

	cout << "Achando frequencias" << endl;
	map<int, int> freqs =  find_frequency<int>(tr_table);
	std::vector<int> elements;

	freq_order_class<int> freq_obj(freqs);

	for(auto& pair : freqs)
	{
		elements.push_back(pair.first);
	}

	cout << "Quantidade de elementos:" << elements.size() << endl;

	sort(elements.begin(),elements.end(),freq_obj);
	//elements.sort(freq_obj);

	table = new header_table<int>();

	int size = ceil(  (  (double) tr_table.size()  ) / 2 );

	cout << "Construindo FPTree" << endl;
	auto tr_begin = tr_table.begin();
	auto tr_split = tr_begin;

	std::advance(tr_split, size);


	Tree_Node<int> *root = build_fptree<int, table_transaction_t<int>::iterator>(tr_begin, tr_split, freqs, freq_obj, supp);
	Tree_Node<int> *root2 = build_fptree<int, table_transaction_t<int>::iterator>(tr_split, tr_table.end(), freqs, freq_obj, supp);

	print_tree(root);
	cout << endl << "----";
	merge_tree(root2, root);
	print_tree(root);

	for(auto it : *table) {
		delete it;
	}

	delete table;

	cout << endl << "----";


	cout << "Iniciando FPGrowth" << endl;
	loop_fp<int>(supp,root, elements);

	if (root != NULL) {
		delete root;
	}

	pthread_exit(NULL);
	return 0;
}
template<typename T>
void print_tree(Tree_Node<T>* node)
{
	node->print();
}

table_transaction_t<int> toMem(string filename)
{
	table_transaction_t<int> memoria;
	list<int> lista;
	int TID = 0;
	fstream sc;
	string s;
	char* pch;
	sc.open(filename.c_str());
	while(getline(sc,s))
	{
		//cout << ".";
		if(s!="")
		{
			int aux;
			pch = strtok((char*)s.c_str()," ,\t\r");
			while(pch!=NULL)
			{
				aux = atoi(pch);
				lista.push_back(aux);
				pch = strtok(NULL," ,\t\r");
			}
			memoria.push_back(transaction_t<int>(TID,lista));
			TID++;
			lista.clear();
		}
	}
	sc.close();
	return memoria;
}


