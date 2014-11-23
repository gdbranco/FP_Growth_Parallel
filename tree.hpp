#ifndef TREE_H
#define TREE_H

#include <vector>
#include <iostream>

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

#endif
