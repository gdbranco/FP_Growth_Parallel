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
#include "parallel.hpp"

using namespace std;


template<typename T>
void print_tree(Tree_Node<T>* node);
table_transaction_t<int> toMem(string filename);

int THREAD_COUNT = 1;

int main(int argc, char** argv)
{
	if (argc < 3) {
		cout << "Uso: input_name supp [num_threads = 1]" << endl;
		return 1;
	} 
	else if(argv[3])
	{
		THREAD_COUNT = atoi(argv[3]);
		if(!THREAD_COUNT)
		{
			cout << "Uso: input_name supp [num_threads = 1]" << endl;
			return 1;
		}
	}

	int supp = atoi(argv[2]);
	string filename(argv[1]);
	cout << "Iniciando programa" << endl;
	cout << "Passando para a memória" << endl;


	table_transaction_t<int> tr_table = toMem(filename.c_str());

	cout << "Achando frequencias" << endl;
	map<int, int> freqs =  find_frequency<int>(tr_table);
	std::vector<int> elements;

	freq_order_class<int> freq_obj(freqs);

	for(auto& pair : freqs)
	{
		elements.push_back(pair.first);
	}

	sort(elements.begin(),elements.end(),freq_obj);

	if (elements.size() < (unsigned) THREAD_COUNT) {
		cout << endl << "Quantidade de elementos fornecidos é menor que a quantidade de threads. Quantidade de threads será limitada." << endl << endl;
		THREAD_COUNT = elements.size();
	}

	cout << "Construindo FPtree" << endl;

	auto sub_trees = build_tree_parallel<int, table_transaction_t<int>::iterator>(tr_table, freqs, supp );


	cout << "Iniciando FPGrowth" << endl;
	auto all_extracts = parallel_loop_fp<int>(supp,sub_trees[0], elements);

	fstream file;
	file.open("output.txt", std::fstream::trunc | std::fstream::out);

	for(auto it = all_extracts->begin(); it != all_extracts->end(); it++) {
		print_frequent_list(file, *it);
		file << endl;
	}

	cout << "Finalizando FPGrowth" << endl;

	auto it1 = all_extracts->begin();
	while(it1 != all_extracts->end()) {
		auto it2 = (*it1)->begin();
		while(it2 != (*it1)->end()) {
			auto it3 = (*it2)->begin();
			while(it3 != (*it2)->end()) {
				it3 = (*it2)->erase(it3);
			}
			it2 = (*it1)->erase(it2);
		}
		delete *it1;
		it1 = all_extracts->erase(it1);
	}

	delete all_extracts;

	delete sub_trees[0];
	
	delete [] sub_trees;

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


