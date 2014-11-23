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


table_transaction_t toMem(string filename);

//template<typename T>
//tree<transaction_t<T>> ordered_to_tree(table_transaction_t ordered);


int main()
{
	table_transaction_t teste = toMem("items.db");
	map<int, int> freqs =  find_frequency<int>(teste);

	
	// for(auto transa = freqs.begin(); transa != freqs.end(); transa++) {
		// cout << transa->first << ": " << transa->second << "\n";
	// }
	// cout << "\n";
	// freq_order_class<int> freq_obj(freqs);
	
	// for(auto tr = teste.begin(); tr != teste.end(); tr++) {
		// sort(tr->items.begin(), tr->items.end(), freq_obj);
	// }
	
	// for(unsigned int i=0;i<teste.size();i++)
	// {
		// cout << teste[i] << endl;
	// }
	Tree_Node<int>* root;
	root  = new Tree_Node<int>(-1);
	root->children.push_back(new Tree_Node<int>(2));
	cout << root->data << endl;
	cout << root->children[0]->data << endl;
	Tree_Node<int>* acha = root->find_first_child(2);
	if(acha!=NULL)
	{
		cout << acha->data;
	}
	return 0;
}

table_transaction_t toMem(string filename)
{
	table_transaction_t memoria;
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


// template<typename T>
// tree<transaction_t<T>> ordered_to_tree(table_transaction_t ordered) {
	// tree<int> fptree;
	
	// tree<int>::iterator root = fptree.insert(-1);
	
	
	// for(auto tr = ordered.begin(); tr != ordered.end(); tr++) {
		
	// }

// }
