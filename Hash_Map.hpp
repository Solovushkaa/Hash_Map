#include <vector>
#include <exception>
#include "Hash_List.hpp"

namespace mls
{
//=================================================================================================
    constexpr int prime_numbers_count = 72;
    constexpr size_t prime_numbers[prime_numbers_count] = {
                                        193ull, 383ull, 767ull, 1503ull, 3071ull, 6151ull, 12289ull, 24593ull, 49157ull, 98317ull, 196613ull,
                                        393241ull, 786433ull, 1572869ull, 3145739ull, 6291469ull, 12582917ull, 25165843ull, 50331653ull,
                                        100663319ull, 201326611ull, 402653189ull, 805306457ull, 1610612741ull, 3221225473ull, 4294967291ull,
                                        8243798341ull, 16492674417ull, 32985348837ull, 64424509211ull, 128849018931ull, 25769803751ull,
                                        51539607551ull, 103079215087ull, 206158430209ull, 412316860441ull, 824633720831ull, 1649267441631ull,
                                        3298534883231ull, 64424509212111ull, 128849018932127ull, 257698037521259ull, 515396075521231ull, 1030792150821297ull,
                                        206158430202417ull, 412316860442147ull, 824633720842287ull, 1649267441644141ull, 3298534883242369ull, 6442450921242431ull,
                                        12884901893243359ull, 25769803752244939ull, 51539607552245831ull, 10307921508226897ull, 20615843020257717ull,
                                        41231686044428321ull, 82463372085485473ull, 164926744166458689ull, 329853488326495737ull, 644245092126496729ull,
                                        128849018932946847ull, 257698037523493859ull, 515396075524986831ull, 103079215082597697ull, 206158430203792717ull,
                                        41231686044838321ull, 824633720866846847ull, 1649267441694696817ull, 3298534883304979649ull, 6442450921304989441ull,
                                        12884901893309794817ull,18446744073709551615ull
                                    };
//=================================================================================================
    size_t find_nearest(size_t num)
    {
        for(int i = 0; i < prime_numbers_count; ++i)
            if(num < prime_numbers[i])
                return prime_numbers[i];
        return prime_numbers[prime_numbers_count-1];
    }
//=================================================================================================
    template<typename Key, typename Value, typename Hasher = std::hash<Key>>
    class Hash_Map
    {
    private:
        using iterator = typename mls::Hash_List<std::pair<Key,Value>>::iterator;

    private:
        size_t _sz;
        size_t _cap;
        double _max_load_factor;
        int _ind_prime_num;

        mls::Hash_List<std::pair<Key, Value>> _table;
        std::vector<iterator> _bucket_storage;

        Hasher _hasher;

    public:
        Hash_Map();
        Hash_Map(size_t capacity);

        Hash_Map(const Hash_Map<Key,Value,Hasher>& cp_map);

        Hash_Map<Key,Value,Hasher>& operator=(const Hash_Map<Key,Value,Hasher>& cp_map);

        void rehash();

        iterator find(const Key& key);

        template<typename P = std::pair<Key,Value>>
        iterator insert(P&& pair);

        iterator erase(iterator pos);

        template<typename K = Key>
        Value& operator[](K&& key);

        size_t size() const { return _sz; }
        size_t capacity() const { return _cap; }
        double max_load_factor() const { return _max_load_factor; }
        void set_max_load_factor(double max_l_f) { _max_load_factor = max_l_f; }

        iterator begin() { return _table.begin(); }
        iterator end() { return _table.end(); }

        ~Hash_Map() {}
    };
//-------------------------------------------------------------------------------------------------
    template<typename Key, typename Value, typename Hasher>
    Hash_Map<Key, Value, Hasher>::Hash_Map():   _sz(0) 
                                                ,_cap(prime_numbers[0])
                                                ,_max_load_factor(1.35)
                                                ,_ind_prime_num(0)
                                                ,_table()
                                                ,_bucket_storage(_cap,_table.real_begin()) 
    {   
        _table.real_begin()->_hash = -1ull;
        _table.end()->_hash = -1ull; 
    }

    template<typename Key, typename Value, typename Hasher>
    Hash_Map<Key, Value, Hasher>::Hash_Map(size_t capacity):_sz(0)
                                                            ,_cap(find_nearest(capacity))
                                                            ,_max_load_factor(1.35)
                                                            ,_ind_prime_num(0)
                                                            ,_table()
                                                            ,_bucket_storage(_cap,_table.real_begin()) 
    {   
        _table.real_begin()->_hash = -1ull;
        _table.end()->_hash = -1ull; 
    }

    template<typename Key, typename Value, typename Hasher>
    Hash_Map<Key, Value, Hasher>::Hash_Map(const Hash_Map& cp_map) : Hash_Map()
    {
        *this = cp_map;
    }

    template<typename Key, typename Value, typename Hasher>
    Hash_Map<Key,Value,Hasher>& Hash_Map<Key, Value, Hasher>::operator=(const Hash_Map& cp_map)
    {
        if(this == &cp_map) { return *this; }
        _table = cp_map._table;
        _bucket_storage.clear();
        _bucket_storage.resize(cp_map._cap, _table.real_begin());

        _sz = cp_map._sz;
        _cap = cp_map._cap;

        size_t hash = 0;
        auto sv_it = _table.begin(); 
        for(auto it = ++(_table.begin()); it != _table.end(); ++it, ++sv_it){
            hash = it->_hash%_cap;
            if(sv_it->_hash != hash){
                _bucket_storage[hash] = sv_it;
            }
        }

        _max_load_factor = cp_map._max_load_factor;
        _ind_prime_num = cp_map._ind_prime_num;

        _hasher = cp_map._hasher;

        return *this;
    }

    template<typename Key, typename Value, typename Hasher>
    void Hash_Map<Key, Value, Hasher>::rehash()
    {
        ++_ind_prime_num;
        if(_ind_prime_num >= prime_numbers_count) { throw std::bad_array_new_length(); }

        _cap = prime_numbers[_ind_prime_num];
        _bucket_storage.clear();
        _bucket_storage.resize(_cap, _table.real_begin());
        _table.real_begin()->_hash = -1ull;
        _table.end()->_hash = -1ull;
        
        auto sv_it = _table.begin();
        size_t i = 1;
        size_t save_hash = -1ull;
        for(auto it = sv_it; i < _sz; ++i, it = sv_it){
            _table.move_after(_bucket_storage[it->_next->_hash%_cap], it);

            save_hash = -1ull;
            if(it->_hash != -1ull && _table.begin()->_hash%_cap != it->_hash%_cap) { save_hash = _table.begin()->_hash%_cap; }

            if(save_hash != -1ull) { _bucket_storage[save_hash] = _table.begin(); }
        }

    }

    template<typename Key, typename Value, typename Hasher>
    typename Hash_Map<Key, Value, Hasher>::iterator Hash_Map<Key, Value, Hasher>::find(const Key& key)
    {
        size_t hash = _hasher(key);
        size_t input_hash = hash%_cap;
        auto it = _bucket_storage[input_hash];
        ++it;
        if(it->_hash == -1ull) { return _table.end(); }
        for(; it->_hash%_cap == input_hash && it != _table.end(); ++it){
            if(it->_data.first == key) { return it; }
        }
        return end();
    }

    template<typename Key, typename Value, typename Hasher>
    template<typename P>
    typename Hash_Map<Key, Value, Hasher>::iterator Hash_Map<Key, Value, Hasher>::insert(P&& pair)
    {
        size_t hash = _hasher(pair.first);
        size_t input_hash = hash%_cap;

        size_t save_hash = -1ull;
        if(hash != -1ull && _table.begin()->_hash%_cap != input_hash) { save_hash = _table.begin()->_hash%_cap; }

        auto it = find(pair.first);
        if(it == _table.end())
        {
            it = _table.insert_after(_bucket_storage[input_hash], pair, hash);
            if(save_hash != -1ull) { _bucket_storage[save_hash] = _table.begin(); }
            ++_sz;
        }

        if(_sz > _cap*_max_load_factor) { rehash(); }

        return it;
    }

    template<typename Key, typename Value, typename Hasher>
    typename Hash_Map<Key, Value, Hasher>::iterator Hash_Map<Key, Value, Hasher>::erase(iterator pos)
    {
        size_t input_hash = pos->_hash%_cap;

        auto check_it = _bucket_storage[input_hash];
        if((++check_it)->_hash%_cap != input_hash) { return end(); }

        check_it = _bucket_storage[input_hash];
        auto it = check_it;
        ++it;
        for(; it != _table.end() && it->_hash%_cap == input_hash; ++it){
            if(it->_data.first == pos->_data.first){
                _table.erase_after(check_it);
                --_sz;
                return check_it;
            }
            ++check_it;
        }
        return end();
    }

    template<typename Key, typename Value, typename Hasher>
    template<typename K>
    Value& Hash_Map<Key, Value, Hasher>::operator[](K&& key)
    {
        auto it = insert(std::move(std::pair<Key,Value>(key, Value())));
        return it->_data.second;
    }
//=================================================================================================
}