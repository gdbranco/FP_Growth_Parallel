#ifndef TREE_H
#define TREE_H

#include <vector>
#include <iostream>
#include "transaction.hpp"

template<typename T>
class Tree_Node
{
public:
	T data;
	std::vector< Tree_Node<T>* > children;
	Tree_Node(){}
	Tree_Node(T _data):
		data(_data){}
	Tree_Node<T>* find_first_child(T data);
	static Tree_Node<T>* build_fptree(table_transaction_t<T> ordered);
};

template<typename T>
Tree_Node<T>* Tree_Node<T>::find_first_child(T data)
{
	for(auto iter = children.begin(); iter != children.end(); iter++) {
		if((*iter)->data == data) {
			return *iter;
		}
	}
	return NULL;
}

template<typename T>
Tree_Node<T>* Tree_Node<T>::build_fptree(table_transaction_t<T> ordered) {

	Tree_Node<T> *root, *cur, *aux;
	root  = new Tree_Node<T>(-1);
	for(auto tr = ordered.begin(); tr != ordered.end(); tr++) {
		cur = root;
		for(auto it = tr->items.begin(); it != tr->items.end(); it++) {
			aux = cur->find_first_child(*it);
			if (aux != NULL) {
				std::cout << "." << *it << std::endl;
				cur = aux;
			} else {
				std::cout << "/" << *it << std::endl;
				aux = new Tree_Node<T>((*it));
				cur->children.push_back( aux );
				cur = aux;
			}
		}
		std::cout << std::endl;
	}

	return root;

}
#endif
