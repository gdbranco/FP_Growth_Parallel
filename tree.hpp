#ifndef TREE_H
#define TREE_H

#include <vector>
#include <iostream>
#include <list>
#include <iterator>
#include "transaction.hpp"
using namespace std;
template<typename T>
class Tree_Node;

template<typename T>
using header_table = std::list< Tree_Node<T>* >;


template<typename T>
class Tree_Node
{
public:
	T data;
	Tree_Node<T>* parent;
	std::list< Tree_Node<T>* > children;
	int count;
	Tree_Node<T>* next;
	Tree_Node<T>* prev;
	Tree_Node(){}
	~Tree_Node();
	Tree_Node(T _data):
		data(_data), parent(NULL), count(0), next(NULL), prev(NULL){}
	Tree_Node(T _data, Tree_Node<T>* _parent):
		data(_data), parent(_parent), count(0), next(NULL), prev(NULL){}
	Tree_Node<T>* find_first_child(T data);
	static Tree_Node<T>* build_fptree(table_transaction_t<T> ordered, header_table<T>* table);
};

template<typename T>
Tree_Node<T>::~Tree_Node() {
	for(auto ch = children.begin(); ch != children.end(); ch++) {
		delete *ch;
	}
	children.clear();
	parent = NULL;

	if (prev != NULL) {
		prev->next = next;
		
	}

	if (next != NULL) {
		next->prev = prev;
	}
}

template<typename T>
Tree_Node<T>* clone_tree(Tree_Node<T> *fptree, header_table<T>* header);

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
Tree_Node<T>* Tree_Node<T>::build_fptree(table_transaction_t<T> ordered, header_table<T>* table) {

	Tree_Node<T> *root, *cur, *aux;
	//std::list<Tree_Node<T>* > *lk_elmnt;
	//bool flag = false;
	root  = new Tree_Node<T>(-1);
	for(auto tr = ordered.begin(); tr != ordered.end(); tr++) {
		cur = root;
		for(auto it = tr->items.begin(); it != tr->items.end(); it++) {
			aux = cur->find_first_child(*it);
			if (aux != NULL) {
				cur = aux;
				cur->count++;
			} else {
				aux = new Tree_Node<T>((*it));
				aux->parent = cur;
				aux->count++;
				cur->children.push_back( aux );

				/* Procura se o elemento já possui uma lista dele e adiciona/cria conforme necessário */
				//for(auto table_it = table->begin(); table_it != table->end(); table_it++) {
					//if (  ((*table_it)->front())->data == *it  ) {
						//(*table_it)->push_back(aux);
						//flag = true;
						//break;
					//}
				//}
				//if (!flag) {
					//lk_elmnt = new std::list<Tree_Node<T>* >();
					//lk_elmnt->push_back(aux);
					//table->push_back(lk_elmnt);
				//}
				cur = aux;
			}
		}
		std::cout << std::endl;
	}

	return root;

}

template<typename T>
void loop_fp(Tree_Node<T> *root, std::vector<T> sorted) {
	int supp = 2;

	Tree_Node<T> *temp;
	T cur_element;

	for(auto it = sorted.rbegin(); it != sorted.rend(); it++) {
		cur_element = *it;
		temp = build_conditional(cur_element, supp, root);
		//calma
		//delete temp;
	}

}


template<typename T, typename Iterator>
void build_full(T element, int supp, Tree_Node<T>* root, Iterator sort_beg, Iterator sort_end, std::list<std::list<T>* >* freq_list, std::list<T>* caller_list) {
	Tree_Node<T> *nroot;
	std::list<T>* m_list;

	nroot = build_conditional(element, supp, root);

	if (caller_list != NULL) {
		m_list = new std::list<T>(*caller_list);
		m_list->push_back(element);
	} else {
		m_list = new std::list<T>();
		m_list->push_back(element);
	}

	if (nroot != NULL) {
		for(auto el = sort_beg; el != sort_end; el++) {
			build_full(*el, supp, nroot, el, sort_end, freq_list, m_list);

		}

		if (freq_list != NULL) {
			freq_list->push_back(m_list);
		}
		
	}

}

template<typename T>
Tree_Node<T>* build_conditional(T element, int supp, Tree_Node<T> *root) {
	Tree_Node<T> *n_root, *aux, *nxt, *par;
	header_table<int> *table;
	int count = -1;
	table = new header_table<int>();

	n_root = clone_tree(root, table);

	for(auto ele = table->begin(); ele != table->end(); ele++) {
		if ((*ele)->data == element) {
			aux = *ele;
			while(aux != NULL) {
				count++;
				aux = aux->next;
			}
			break;
		}
	}

	if(count > 0) {
		podar(element, n_root);

		int c;
		for(auto el = table->begin(); el != table->end(); el++) {
			aux = *el;
			c = 0;
			while(aux != NULL) {
				c += aux->count;
				aux = aux->next;
			}

			aux = *el;

			if (c < supp) {
				while(aux != NULL) {
					if (aux->count > 0) {
						nxt = aux->next;
						par = aux->parent;

						for(auto son = aux->children.begin(); son != aux->children.end(); son++) {
							(*son)->parent = par;
						}

						//Transfere os filhos do nó para o pai
						par->children.splice(par->children.end(), aux->children);

						par->children.remove(aux);
						aux = nxt;
					} else {
						aux = aux->next;
					}
				}
			}
		}

		delete table;

		return n_root;

	} else {
		delete table;

		delete n_root;

		return NULL;
	}


}



template<typename T>
bool podar(T val, Tree_Node<T> *node) {
	bool found = false, aux = false;

	node->count = 0;

	auto it = node->children.begin();
	while(it != node->children.end()) {
		if ((*it)->data == val) {
			node->count += (*it)->count;
			it = node->children.erase(it);
			found = true;
		} else {
			aux = podar(val, *it);
			if(aux) {
				found = true;
				node->count += (*it)->count;
				it++;
			} else {
				it = node->children.erase(it);
			}
		}
	}

	return found;
}

template<typename T>
Tree_Node<T>* clone_tree(Tree_Node<T> *fptree, header_table<T>* header) {
	Tree_Node<T> *node, *aux;	
	bool found = false;
	node = new Tree_Node<T>(fptree->data);
	node->count = fptree->count;

	for(auto it = header->begin(); it != header->end(); it++) {
		if ((*it)->data == node->data) {
			found = true;
			aux = *it;
			while(aux->next != NULL) {
				aux = aux->next;
			}
			aux->next = node;
			node->prev = aux;
			break;
		}
	}	
	if (!found) {
		if (node->data != -1) {
			aux = new Tree_Node<T>(node->data);
			aux->next = node;
			node->prev = aux;
			header->push_back(aux);
		}
	}

	for(auto ch = fptree->children.begin(); ch != fptree->children.end(); ch++) {
		node->children.push_back(clone_tree(*ch, header));
		node->children.back()->parent = node;
	}

	return node;
}


#endif
