#ifndef TRANSACTION_H
#define TRANSACTION_H

#include <vector>
#include <map>
#include <string>

template<typename T>
struct transaction_t
{
	int TID;
	std::vector<T> items;
	transaction_t(int _TID, std::vector<T> _items): TID(_TID),items(_items){}
	friend std::ostream& operator<<(std::ostream& os,const transaction_t& it)
    {
        os << it.TID << ' ';
        for(unsigned int i=0; i<it.items.size(); i++)
        {
            os << it.items[i];
            if(i!=it.items.size()-1)
            {
                os << std::string("->");
            }
        }
        return os;
    }
};

template<typename T>
using table_transaction_t = std::vector<transaction_t<T> >;

template<typename T>
std::map<T, int> find_frequency(table_transaction_t<T> mem) {
	std::map<T,int> freqs;
	
	for(int i = 0; i < mem.size(); i++) 
	{
		for(int j = 0; j < mem[i].items.size(); j++) 
		{
			if(freqs.find(mem[i].items[j]) != freqs.end()) 
			{
				freqs[mem[i].items[j]]++;
			} 
			else 
			{
				freqs[mem[i].items[j]] = 1;
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
