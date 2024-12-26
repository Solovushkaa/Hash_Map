#include <utility>
#include <memory>
#include <type_traits>

namespace mls {
//=================================================================================================
    // template<typename Pair>
    // struct Hash_Node {
    //     Hash_Node* _next;
    //     typename std::remove_reference_t<Pair> _data;
    //     size_t _hash;

    //     Hash_Node() = default;

    //     template<typename P = Pair>
    //     Hash_Node(Hash_Node* next, P&& data, size_t hash)
    //         : _next(next), _data(std::forward<P>(data)), _hash(hash) {}
    // };
    template<typename Pair>
    struct Hash_Node {
        Hash_Node* _next;
        Pair _data;
        size_t _hash;

        Hash_Node() = default;

        template<typename P = Pair>
        Hash_Node(Hash_Node* next, P&& data, size_t hash)
            : _next(next), _data(std::forward<P>(data)), _hash(hash) {}

        Hash_Node(const Hash_Node& node) = delete;
        Hash_Node& operator=(const Hash_Node& node) = delete;
        ~Hash_Node() = default;
    };
//=================================================================================================
    // template<typename Pair>
    // struct Forward_Iterator {

    //     Hash_Node<Pair>* _node;

    //     Forward_Iterator(Hash_Node<Pair>* node) : _node(node) {}
    //     Forward_Iterator(const Forward_Iterator& it) = default;
    //     Forward_Iterator& operator=(const Forward_Iterator& it) = default;

    //     Forward_Iterator& operator++() {
    //         _node = _node->_next;
    //         return *this;
    //     }

    //     Forward_Iterator& operator++(int) {
    //         Forward_Iterator tmp = *this;
    //         _node = _node->_next;
    //         return tmp;
    //     }

    //     typename std::remove_reference_t<Pair>& operator*() {
    //         return _node->_data;
    //     }

    //     typename std::remove_reference_t<Hash_Node<Pair>>* operator->() {
    //         return _node;
    //     }

    //     bool operator==(Forward_Iterator<Pair> it) const {
    //         return (_node == it._node);
    //     }

    //     bool operator!=(Forward_Iterator<Pair> it) const {
    //         return !(_node == it._node);
    //     }
    // };

    template<typename Pair, bool IsConst>
    struct Forward_Iterator {
        using pointer = std::conditional_t<IsConst, const Hash_Node<Pair>*, Hash_Node<Pair>*>;
        using reference = std::conditional_t<IsConst, const Pair&, Pair&>;

        pointer _node;

        Forward_Iterator(pointer node) : _node(node) {}
        Forward_Iterator(const Forward_Iterator& it) = default;
        Forward_Iterator& operator=(const Forward_Iterator& it) = default;

        Forward_Iterator& operator++() {
            _node = _node->_next;
            return *this;
        }

        Forward_Iterator operator++(int) {
            Forward_Iterator tmp = *this;
            _node = _node->_next;
            return tmp;
        }

        reference operator*() {
            return _node->_data;
        }

        pointer operator->() {
            return _node;
        }

        bool operator==(Forward_Iterator<Pair, IsConst> it) const {
            return (_node == it._node);
        }

        bool operator!=(Forward_Iterator<Pair, IsConst> it) const {
            return !(*this == it);
        }
    };
//=================================================================================================
    template<typename Pair>
    class Hash_List {
    private:
        size_t _sz;
        Hash_Node<Pair> _p_beg;
        Hash_Node<Pair> _p_end;
        std::allocator<Hash_Node<Pair>> _alloc;

    public:
        using iterator = Forward_Iterator<Pair, false>;
        using const_iterator = Forward_Iterator<Pair, true>;

        Hash_List();

        Hash_List(const Hash_List<Pair>& cp_list);

        Hash_List<Pair>& operator=(const Hash_List<Pair>& cp_list);

        template<typename P = Pair>
        Hash_Node<Pair>* obj_construct(P&& el, size_t hash);

        void obj_destruct(Hash_Node<Pair>* node);

        template<typename P = Pair>
        void push_front(P&& el, size_t hash);
        void pop_front();

        template<typename P = Pair>
        iterator insert_after(iterator it, P&& el, size_t hash);

        void move_after(iterator after, iterator mv);

        iterator erase_after(iterator it);

        size_t size() const { return _sz; }
        bool empty() const { return !_sz; }

        void clear();

        iterator begin() { return { _p_beg._next }; }
        iterator real_begin() { return { &_p_beg }; }
        iterator end() { return { &_p_end }; }

        const_iterator cbegin() const { return { _p_beg._next }; }
        const_iterator cend() const { return { &_p_end }; }

        ~Hash_List() { clear(); }
    };
//-------------------------------------------------------------------------------------------------
    template <typename Pair>
    Hash_List<Pair>::Hash_List() :  _sz(0)
                                    ,_p_beg{&_p_end, typename std::remove_reference_t<Pair>{}, 0}
                                    ,_p_end{&_p_end, typename std::remove_reference_t<Pair>{}, 0} 
    {}

    template <typename Pair>
    Hash_List<Pair>::Hash_List(const Hash_List<Pair>& cp_list) : Hash_List() 
    {
        *this = cp_list;
    }

    template <typename Pair>
    Hash_List<Pair>& Hash_List<Pair>::operator=(const Hash_List<Pair>& cp_list)
    {
        if (this == &cp_list) {
            return *this;
        }
        clear();

        auto sv_it = real_begin();
        for(auto it = cp_list.cbegin(); it != cp_list.cend(); ++it, ++sv_it){
            insert_after(sv_it, it->_data, it->_hash);
        }
        return *this;
    }

    template <typename Pair>
    template <typename P>
    Hash_Node<Pair> *Hash_List<Pair>::obj_construct(P&& el, size_t hash)
    {
        Hash_Node<Pair>* new_node = _alloc.allocate(1);
        try {
            _alloc.construct(new_node, nullptr, std::forward<P>(el), hash);
        } catch (...) {
            _alloc.deallocate(new_node, 1);
            throw;
        }
        return new_node;
    }

    template <typename Pair>
    void Hash_List<Pair>::obj_destruct(Hash_Node<Pair>* node)
    {
        _alloc.destroy(node);
        _alloc.deallocate(node, 1);
    }

    template <typename Pair>
    template <typename P>
    void Hash_List<Pair>::push_front(P&& el, size_t hash)
    {
        insert_after(real_begin(), std::forward<P>(el), hash);
    }

    template <typename Pair>
    void Hash_List<Pair>::pop_front()
    {
        if (_sz == 0) { return; }

        Hash_Node<Pair>* del = _p_beg._next;
        _p_beg._next = del->_next;
        obj_destruct(del);
        --_sz;
    }

    template <typename Pair>
    template <typename P>
    typename Hash_List<Pair>::iterator Hash_List<Pair>::insert_after(iterator it, P&& el, size_t hash)
    {
        Hash_Node<Pair>* new_node = obj_construct(std::forward<P>(el), hash);
        new_node->_next = it->_next;
        it->_next = new_node;
        ++_sz;
        return iterator(new_node);
    }

    template <typename Pair>
    void Hash_List<Pair>::move_after(iterator after, iterator mv)
    {
        auto sv_it = mv;
        ++sv_it;
        mv->_next = sv_it->_next;
        sv_it->_next = after->_next;
        after->_next = sv_it._node;
    }

    template <typename Pair>
    typename Hash_List<Pair>::iterator Hash_List<Pair>::erase_after(iterator it)
    {
        if (it == end() || it._node->_next == end()._node) { return end(); }

        Hash_Node<Pair>* del = it._node->_next;
        it._node->_next = del->_next;
        obj_destruct(del);
        --_sz;
        return iterator(it._node->_next);
    }

    template <typename Pair>
    void Hash_List<Pair>::clear()
    {
        while (_sz > 0) {
            pop_front();
        }
    }
//=================================================================================================
}