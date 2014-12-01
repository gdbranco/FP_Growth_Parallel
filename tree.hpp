#ifndef TREE_H
#define TREE_H

#include <iostream>
#include <list>
#include <iterator>
#include "transaction.hpp"
#include <omp.h>
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
	static Tree_Node<T>* build_fptree(table_transaction_t<T> ordered, header_table<T>* table, map<T, int> freqs, freq_order_class<T> &freq_obj, int supp);
	//bool operator == (const int b) const
	//{
		//return (b == data);
	//}
	void add_child(Tree_Node<T>* _child)
	{
		this->children.push_back(_child);
	}
	bool has_children()
	{
		return !children.empty();
	}
	bool is_singlepathed()
	{
		return children.size() < 2;
	}
	bool is_root()
	{
		return data == -1;
	}
	std::list< Tree_Node<T>* >* get_children()
	{
		return &(this->children);
	}
	T get_data()
	{
		return data;
	}
	void set_parent(Tree_Node<T>* _parent)
	{
		this->parent = _parent;
	}
	void increment(int _value = 1)
	{
		this->count+=_value;
	}
	void print()
	{
		if(!is_root())
		{
			cout << data << " ";
		}
		for(auto child : children)
		{
			child->print();
		}
		cout << endl;
	}
};

template<typename T>
Tree_Node<T>::~Tree_Node() {
	for(auto ch : children){
	//for(auto ch = children.begin(); ch != children.end(); ch++) {
		delete ch;
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
	for(auto iter : children){
		if(iter->data == data) {
			return iter;
		}
	}
	return NULL;
}

template<typename T>
Tree_Node<T>* Tree_Node<T>::build_fptree(table_transaction_t<T> ordered, header_table<T>* table, map<T, int> freqs, freq_order_class<T> &freq_obj, int supp) {


	Tree_Node<T> *root, *cur, *aux;
	root  = new Tree_Node<T>(-1);
	int s = ordered.size();
	int i = 0;
	for(auto tr : ordered){
		cur = root;
		cout << i << "/" << s << endl;
		i++;
		tr.items.sort(freq_obj);
		for(auto it = tr.items.begin(); it != tr.items.end(); it++) {
			aux = cur->find_first_child(*it);
			if (aux != NULL) {

				if(freqs[*it] < supp) {
					it = tr.items.erase(it);
					it--;
				} 

				cur = aux;
				cur->increment();
			} else {
				aux = new Tree_Node<T>((*it));
				aux->set_parent(cur);
				aux->increment();
				cur->add_child(aux);

				cur = aux;
			}
		}
	}

	return root;

}

template<typename T>
void print_frequent_list(const list< list<T>* >* extract_list)
{
	for(auto ele = extract_list->rbegin(); ele != extract_list->rend(); ele++) {
		cout << "{";
		for(auto it = (*ele)->rbegin(); it != (*ele)->rend(); it++) {
			cout << *it;
			if(*it != (*ele)->front()) 
			 cout << ",";
		}
		cout << "}";
	}
}

template<typename T>
void loop_fp(int supp,Tree_Node<T> *root, std::vector<T> sorted) {
	list< list<int>* >* extract_list;
	list<int>* my_list;

	T cur_element;

//#pragma omp parallel for
	for(auto it = sorted.rbegin(); it != sorted.rend(); it++) {
		extract_list = new list<list<int>* >();
		my_list = NULL;
		cur_element = *it;
		cout << "Utilizando : " << cur_element << endl;
		build_full(cur_element, supp, root, sorted.rbegin(), sorted.rend(), extract_list, my_list);
		print_frequent_list(extract_list);
		cout << endl;
		extract_list->clear();
		delete extract_list;
		delete my_list;
	}
	cout << endl;

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

	delete nroot;

}

template<typename T>
Tree_Node<T>* build_conditional(T element, int supp, Tree_Node<T> *root) {
	Tree_Node<T> *n_root, *aux, *nxt, *par;
	header_table<int> *table;
	int count = -1;
	table = new header_table<int>();

	n_root = clone_tree(root, table);

	for(auto ele : *table){
		if (ele->data == element) {
			aux = ele;
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
		for(auto el : *table){
			aux = el;
			c = 0;
			while(aux != NULL) {
				c += aux->count;
				aux = aux->next;
			}

			aux = el;

			if (c < supp) {
				while(aux != NULL) {
					if (aux->count > 0) {
						nxt = aux->next;
						par = aux->parent;

						for(auto son : *(aux->get_children())){
							son->set_parent(par);
						}

						//Transfere os filhos do nó para o pai
						par->get_children()->splice(par->get_children()->end(), *(aux->get_children()));

						par->get_children()->remove(aux);
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

	auto it = node->get_children()->begin();
	while(it != node->get_children()->end()) {
		if ((*it)->data == val) {
			//node->count += (*it)->count;
			node->increment((*it)->count);
			//it = node->children.erase(it);
			it = node->get_children()->erase(it);
			found = true;
		} else {
			aux = podar(val, *it);
			if(aux) {
				found = true;
				node->increment((*it)->count);
				it++;
			} else {
				it = node->get_children()->erase(it);
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

	for(auto it : *header){
		if (it->data == node->data) {
			found = true;
			aux = it;
			while(aux->next != NULL) {
				aux = aux->next;
			}
			aux->next = node;
			node->prev = aux;
			break;
		}
	}	
	if (!found) {
		if(!node->is_root()){
			aux = new Tree_Node<T>(node->data);
			aux->next = node;
			node->prev = aux;
			header->push_back(aux);
		}
	}

	for(auto ch : *(fptree->get_children())){
		node->children.push_back(clone_tree(ch, header));
		node->children.back()->set_parent(node);
	}

	return node;
}


#endif
