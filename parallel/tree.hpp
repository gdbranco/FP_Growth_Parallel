#ifndef TREE_H
#define TREE_H

#include <iostream>
#include <list>
#include <iterator>
#include <memory>
#include "transaction.hpp"
#include <pthread.h>
using namespace std;
template<typename T>
class Tree_Node;

template<typename T>
using header_table = std::list< Tree_Node<T>* >;

#define THREAD_COUNT 4

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

	void add_child(Tree_Node<T>* _child)
	{
		this->children.push_back(_child);
	}
	bool has_children()
	{
		return !children.empty();
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
using freq_set = std::list<T>;

template<typename T>
using freq_set_ptr = std::shared_ptr< std::list<T> >;

template<typename T>
struct thread_loop_args {
	int supp;
	Tree_Node<T> *root;
	int m_vec_begin;
	int m_vec_len;
	std::vector<T> sorted;
	list< list<freq_set_ptr<T> >* >* local_extract_list;
	~thread_loop_args();

	thread_loop_args();
	thread_loop_args(int _supp, Tree_Node<T>* _root, int _beg, int _vec_len, std::vector<T> _sorted) :
		supp(_supp), root(_root), m_vec_begin(_beg), m_vec_len(_vec_len), sorted(_sorted) {}
};

template<typename T>
thread_loop_args<T>::~thread_loop_args() {
	auto lista_freq_el_it = local_extract_list->begin();
	while(lista_freq_el_it != local_extract_list->end()) {
		auto freq_st = (*lista_freq_el_it)->begin();
		while(freq_st != (*lista_freq_el_it)->end()) {
			auto el = (*freq_st)->begin();
			while(el != (*freq_st)->end()) {
				el = (*freq_st)->erase(el);
			}
			freq_st = (*lista_freq_el_it)->erase(freq_st);
		}
		delete *lista_freq_el_it;
		lista_freq_el_it = local_extract_list->erase(lista_freq_el_it);
	}
	
	delete local_extract_list;
}

template<typename T>
void* thread_loop_fp(void* args);

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
void print_frequent_list(const list< freq_set_ptr<T> >* extract_list)
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
void loop_fp(int supp,Tree_Node<T> *root, const std::vector<T> &sorted) {

	pthread_t threads[THREAD_COUNT];
	thread_loop_args<int>** td ;
	td = new thread_loop_args<int>*[THREAD_COUNT];

	int j = 0;
	int local_beg = 0;
	int vec_share = ceil((double) sorted.size()/THREAD_COUNT) ;
	for (j = 0; j < THREAD_COUNT; j++) {
		local_beg = j*vec_share;
		td[j] = new thread_loop_args<int>(supp, root, local_beg, vec_share, sorted);
		pthread_create(&threads[j], NULL, thread_loop_fp<int>, (void *) td[j]);
	}

	list< list< freq_set_ptr<T> >* > all_extracts;

	void *status;
	for (int i = 0; i < THREAD_COUNT; i++) {
		pthread_join(threads[i], &status);
		all_extracts.splice(all_extracts.end(), *(td[i]->local_extract_list));
	}

	//for(auto it = all_extracts.begin(); it != all_extracts.end(); it++) {
		//print_frequent_list(*it);
		//cout << endl;
	//}

	for (int i = 0; i < THREAD_COUNT; i++) {
		delete td[i];
	}

	delete [] td;

	auto it1 = all_extracts.begin();
	while(it1 != all_extracts.end()) {
		auto it2 = (*it1)->begin();
		while(it2 != (*it1)->end()) {
			auto it3 = (*it2)->begin();
			while(it3 != (*it2)->end()) {
				it3 = (*it2)->erase(it3);
			}
			it2 = (*it1)->erase(it2);
		}
		delete *it1;
		it1 = all_extracts.erase(it1);
	}
	//delete all_extracts;

}


template<typename T>
void* thread_loop_fp(void* args) {
	thread_loop_args<T> *m_args;
	m_args = (thread_loop_args<T> *) args;
	int supp = (int) m_args->supp;
	Tree_Node<T> *root = (Tree_Node<T>*) m_args->root;
	const std::vector<T> sorted = (std::vector<T>) m_args->sorted;

	int my_vec_begin = (int) m_args->m_vec_begin;
	int my_vec_len = (int) m_args->m_vec_len;

	list<freq_set_ptr<T> >* local_extract_list;
	freq_set_ptr<T> my_list(new freq_set<T>());

	m_args->local_extract_list = new list< list<freq_set_ptr<T> >* >();
	int sort_size = sorted.size();
	int my_vec_end = my_vec_begin + my_vec_len;
	for(int i = my_vec_begin; (i != my_vec_end) && (i < sort_size) ; i++) {
		cout <<"Extraindo : " << sorted[i] << endl;
		local_extract_list = new list<freq_set_ptr<T> >();
		build_full(sorted[i], supp, root, sorted.rbegin(), sorted.rend(), local_extract_list, my_list);

		m_args->local_extract_list->push_back(local_extract_list);
	}


	pthread_exit(NULL);

}

template<typename T, typename Iterator>
void build_full(T element, int supp, Tree_Node<T>* root, Iterator sort_beg, Iterator sort_end, std::list<freq_set_ptr<T> >* freq_list, freq_set_ptr<T> caller_list) {
	Tree_Node<T> *nroot;

	

	nroot = build_conditional(element, supp, root);

	freq_set_ptr<T> m_list( new freq_set<T>(caller_list->begin(), caller_list->end()) );
	m_list->push_back(element);

	if (nroot != NULL) {
		for(auto el = sort_beg; el != sort_end; el++) {
			build_full(*el, supp, nroot, el, sort_end, freq_list, m_list);

		}

		if (freq_list != NULL) {
			freq_list->push_back(m_list);
		} else {
			m_list.reset();
		
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
						delete aux;
						aux = nxt;
					} else {
						aux = aux->next;
					}
				}
			}
		}

		for(auto it : *table) {
			delete it;
		}

		delete table;

		return n_root;

	} else {
		for(auto it : *table) {
			delete it;
		}

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
			node->increment((*it)->count);
			delete *it;
			it = node->get_children()->erase(it);
			found = true;
		} else {
			aux = podar(val, *it);
			if(aux) {
				found = true;
				node->increment((*it)->count);
				it++;
			} else {
				delete *it;
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
