#include <iterator>
#include "fp_tree.hpp"
#include "transaction.hpp"

//================= Parallel Build

template<typename T, typename Iterator>
struct thread_b_and_m_args {
	Iterator tr_begin;
	Iterator tr_end;
	map<T, int> freqs;
	freq_order_class<T> freq_obj;
	int supp;
	int my_id;
	Tree_Node<T>** sub_trees;
	pthread_t *threads;

	thread_b_and_m_args(Iterator _tr_begin, Iterator _tr_end, map<T,int> _freqs, freq_order_class<T> _freq_obj, int _supp, int _my_id, Tree_Node<T>** _sub_trees, pthread_t* _threads) :
		tr_begin(_tr_begin), tr_end(_tr_end), freqs(_freqs), freq_obj(_freq_obj), supp(_supp), my_id(_my_id), sub_trees(_sub_trees), threads(_threads) {}

};

template<typename T, typename Iterator>
void* thread_build_and_merge(void* args) {
	thread_b_and_m_args<T, Iterator>* m_args = (thread_b_and_m_args<T, Iterator>*) args;
	Iterator tr_begin = (Iterator) m_args->tr_begin;
	Iterator tr_end = (Iterator) m_args->tr_end;
	map<T, int> freqs = (map<T, int>) m_args->freqs;
	freq_order_class<T> freq_obj = (freq_order_class<T>) m_args->freq_obj;
	int supp = (int) m_args->supp;
	int my_id = (int) m_args->my_id;
	Tree_Node<T>** sub_trees = (Tree_Node<T>**) m_args->sub_trees;
	pthread_t *threads = (pthread_t*) m_args->threads;

	Tree_Node<T> *m_root;

	 m_root = build_fptree<T, Iterator>(tr_begin, tr_end, freqs, freq_obj, supp);


	 int offset = 1;

	 int pair_id = my_id + offset;
	 void* status;

	 if(my_id % (2*offset) == 0) {
		 while(pair_id < THREAD_COUNT) {
			 pthread_join(threads[pair_id - 1], &status);
			 merge_tree(sub_trees[pair_id], m_root);
			
			 offset <<= 1;
			 pair_id = my_id + offset;
		 }
	 }

	 sub_trees[my_id] = m_root;

	 if (my_id != 0) {
		 pthread_exit(NULL);
	 }

	 return NULL;

}

template<typename T, typename Iterator>
Tree_Node<T>** build_tree_parallel(table_transaction_t<T> tr_table, map<T, int> freqs, int supp) {

	int size = ceil(  (  (double) tr_table.size()  ) / THREAD_COUNT );

	freq_order_class<T> freq_obj(freqs);

	Tree_Node<T>** sub_trees = new Tree_Node<T>*[THREAD_COUNT];

	pthread_t *threads = new pthread_t[THREAD_COUNT - 1];


	thread_b_and_m_args<T, Iterator >** th_args;
	th_args = new thread_b_and_m_args<T, Iterator >*[THREAD_COUNT];

	int th_id = 0;

	auto tr_split1 = tr_table.begin();
	auto tr_split2 = tr_split1;

	std::advance(tr_split2, size);

	for (int i = 0; i < THREAD_COUNT - 1; i++) {
		th_id++;
		th_args[th_id] = new thread_b_and_m_args<T, Iterator >(tr_split1, tr_split2, freqs, freq_obj, supp, th_id, sub_trees, threads);
		pthread_create(&threads[i], NULL, thread_build_and_merge<T, Iterator>, (void *) th_args[th_id]);
		std::advance(tr_split1, size);
		std::advance(tr_split2, size);
	}

	th_args[0] = new thread_b_and_m_args<T, Iterator >(tr_split1, tr_table.end(), freqs, freq_obj, supp, 0, sub_trees, threads);
	thread_build_and_merge<T, Iterator >( (void*) th_args[0]  );

	if(th_args[0]!=NULL)
		delete th_args[0];

	for(int i=1;i<THREAD_COUNT;i++)
	{
		if(th_args[i]!=NULL)
			delete th_args[i];
		if(sub_trees[i]!=NULL)
			delete sub_trees[i];
	}

	delete [] th_args;
	delete [] threads;

	return sub_trees;
}

//======================= Parallel extract

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
		local_extract_list = new list<freq_set_ptr<T> >();
		build_full(sorted[i], supp, root, sorted.rbegin(), sorted.rend(), local_extract_list, my_list);

		m_args->local_extract_list->push_back(local_extract_list);
	}


	pthread_exit(NULL);

}

template<typename T>
list< list< freq_set_ptr<T> >* >* parallel_loop_fp(int supp,Tree_Node<T> *root, const std::vector<T> &sorted) {

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

	list< list< freq_set_ptr<T> >* >* all_extracts = new list< list< freq_set_ptr<T> >* >;

	void *status;
	for (int i = 0; i < THREAD_COUNT; i++) {
		pthread_join(threads[i], &status);
		all_extracts->splice(all_extracts->end(), *(td[i]->local_extract_list));
	}


	for (int i = 0; i < THREAD_COUNT; i++) {
		delete td[i];
	}

	delete [] td;

	return all_extracts;


}


