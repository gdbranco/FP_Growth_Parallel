#ifndef TRANSACTION_H
#define TRANSACTION_H

#include <list>
#include <map>
#include <string>

template<typename T>
struct transaction_t
{
	int TID;
	std::list<T> items;
	transaction_t(int _TID, std::list<T> _items): TID(_TID),items(_items){}
	friend std::ostream& operator<<(std::ostream& os,const transaction_t& it)
    {
        os << it.TID << ' ';
		for(auto i : it.items)
		{
            os << i;
            if(i!=it.items.back())
            {
                os << std::string("->");
            }
        }
        return os;
    }
};

template<typename T>
using table_transaction_t = std::list<transaction_t<T> >;

template<typename T>
std::map<T, int> find_frequency(table_transaction_t<T> mem) {
	std::map<T,int> freqs;
	
	for(auto tr = mem.begin(); tr != mem.end(); tr++) {
		for(auto it = tr->items.begin(); it != tr->items.end(); it++) {
			if(freqs.find(*it) != freqs.end()) 
			{
				freqs[*it]++;
			} 
			else 
			{
				freqs[*it] = 1;
			}
		}
	}
	return freqs;
}

template<typename T>
struct freq_order_class {
	std::map<T, int> reference_table;
	bool operator() (T i,T j) { return (reference_table[i] > reference_table[j]);}
	freq_order_class(std::map<T, int> _freq_table) : reference_table(_freq_table) {}
};
#endif
