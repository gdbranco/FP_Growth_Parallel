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


table_transaction_t<int> toMem(string filename);

template<typename T>
Tree_Node<T>* ordered_to_tree(table_transaction_t<T> ordered);


int main()
{
	table_transaction_t<int> teste = toMem("items.db");
	map<int, int> freqs =  find_frequency<int>(teste);

	freq_order_class<int> freq_obj(freqs);

	for(auto tr = teste.begin(); tr != teste.end(); tr++) {
		sort(tr->items.begin(), tr->items.end(), freq_obj);
	}

	Tree_Node<int> *root = ordered_to_tree<int>(teste);



	 //for(auto transa = freqs.begin(); transa != freqs.end(); transa++) {
		 //cout << transa->first << ": " << transa->second << "\n";
	 //}
	 //cout << "\n";
	 //
	cout << endl << "----" << endl;

	for(unsigned int i=0;i<teste.size();i++)
	{
		cout << teste[i] << endl;
	}
	
	return 0;
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


template<typename T>
Tree_Node<T>* ordered_to_tree(table_transaction_t<T> ordered) {

	Tree_Node<T> *root, *cur, *aux;
	root  = new Tree_Node<T>(-1);
	for(auto tr = ordered.begin(); tr != ordered.end(); tr++) {
		cur = root;
		for(auto it = tr->items.begin(); it != tr->items.end(); it++) {
			aux = cur->find_first_child(*it);
			if (aux != NULL) {
				cout << "." << *it << endl;
				cur = aux;
			} else {
				cout << "/" << *it << endl;
				aux = new Tree_Node<T>((*it));
				cur->children.push_back( aux );
				cur = aux;
			}
		}
		cout << endl;
	}

	return root;

}
