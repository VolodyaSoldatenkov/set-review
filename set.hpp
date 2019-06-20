#pragma once

#include <cstddef>
#include <utility>
#include <iterator>


template<typename T>
class Set {
private:
    enum Color : bool {
        black,
        red
    };

    struct Node {
        T value;
        Node* left;
        Node* right;
        Node* parent;
        Color color;
    };

public:
    class iterator {
        friend class Set<T>;

    private:
        Node* cur;
        const Set<T>* set;

    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = const T;
        using difference_type = ptrdiff_t;
        using pointer = value_type* const;
        using reference = value_type&;

    private:
        iterator(Node* cur, const Set<T>* set) : cur(cur), set(set) {}

    public:
        iterator() : cur(nullptr), set(nullptr) {}

        friend bool operator==(const iterator& lhs, const iterator& rhs) {
            return lhs.cur == rhs.cur;
        }

        friend bool operator!=(const iterator& lhs, const iterator& rhs) {
            return !(lhs == rhs);
        }

        reference operator*() const {
            return cur->value;
        }

        pointer operator->() const {
            return &cur->value;
        }

        iterator& operator++() {
            cur = set->next(cur);
            return *this;
        }

        const iterator operator++(int) {
            iterator old = *this;
            ++*this;
            return old;
        }

        iterator& operator--() {
            cur = set->prev(cur);
            return *this;
        }

        const iterator operator--(int) {
            iterator old = *this;
            --*this;
            return old;
        }
    };

private:
    Node* _root;
    Node* _begin;
    size_t _size;

private:
    static Color& color(Node* n) {
        static Color c;
        return n ? n->color : (c = black);
    }

    static Node* brother(Node* n) {
        Node* p = n->parent;
        if (n == p->left)
            return p->right;
        else
            return p->left;
    }

    Node* next(Node* v) const {
        if (!v) {
            v = _root;
            while (v && v->left)
                v = v->left;
            return v;
        }
        if (v->right) {
            v = v->right;
            while (v->left)
                v = v->left;
            return v;
        }
        Node* p = v->parent;
        while (p && v == p->right) {
            v = p;
            p = p->parent;
        }
        return p;
    }

    Node* prev(Node* v) const {
        if (!v) {
            v = _root;
            while (v && v->right)
                v = v->right;
            return v;
        }
        if (v->left) {
            v = v->left;
            while (v->right)
                v = v->right;
            return v;
        }
        Node* p = v->parent;
        while (p && v == p->left) {
            v = p;
            p = p->parent;
        }
        return p;
    }

    void rotate_left(Node* v) {
        Node* son = v->right;
        Node* parent = v->parent;

        if (parent) {
            if (parent->left == v)
                parent->left = son;
            else
                parent->right = son;
        }
        son->parent = parent;

        v->right = son->left;
        if (son->left)
            son->left->parent = v;

        son->left = v;
        v->parent = son;

        if (_root == v)
            _root = v->parent;
    }

    void rotate_right(Node* v) {
        Node* son = v->left;
        Node* parent = v->parent;

        if (parent) {
            if (parent->left == v)
                parent->left = son;
            else
                parent->right = son;
        }
        son->parent = parent;

        v->left = son->right;
        if (son->right)
            son->right->parent = v;

        son->right = v;
        v->parent = son;

        if (_root == v)
            _root = v->parent;
    }

    void insert_rebalance(Node* v) {
        if (!v->parent) {
            v->color = black;
            return;
        }

        if (color(v->parent) == black)
            return;

        Node* p = v->parent;
        Node* gp = p->parent;
        Node* u = brother(p);
        if (color(u) == red) {
            color(p) = black;
            color(u) = black;
            color(gp) = red;
            return insert_rebalance(gp);
        }

        if (gp->left == p) {
            if (p->right == v) {
                rotate_left(p);
                p = gp->left;
                v = p->left;
            }
            color(gp) = red;
            color(p) = black;
            rotate_right(gp);
        } else {
            if (p->left == v) {
                rotate_right(p);
                p = gp->right;
                v = p->right;
            }
            color(gp) = red;
            color(p) = black;
            rotate_left(gp);
        }
    }

    void erase_rebalance(Node* v) {
        Node* p = v->parent;
        if (!p || color(v) == red) {
            color(v) = black;
            return;
        }

        Node* b = brother(v);
        if (color(b) == red) {
            color(b) = black;
            color(p) = red;
            if (p->right == v)
                rotate_right(p);
            else
                rotate_left(p);
            b = brother(v);
        }

        if (color(b->left) == black &&
            color(b->right) == black) {
            if (color(p) == black) {
                color(b) = red;
                return erase_rebalance(p);
            } else {
                color(p) = black;
                color(b) = red;
                return;
            }
        }

        if (v == p->left && color(b->right) == black) {
            color(b) = red;
            color(b->left) = black;
            rotate_right(b);
            b = brother(v);
        } else if (v == p->right && color(b->left) == black) {
            color(b) = red;
            color(b->right) = black;
            rotate_left(b);
            b = brother(v);
        }

        color(b) = color(p);
        color(p) = black;
        if (v == p->left) {
            color(b->right) = black;
            rotate_left(p);
        } else {
            color(b->left) = black;
            rotate_right(p);
        }
    }

    void replace_with_child(Node* v) {
        Node* s = v->left ? v->left : v->right;
        Node*& p_ptr = (v->parent
                        ? v == v->parent->left
                          ? v->parent->left
                          : v->parent->right
                        : _root);

        if (color(v) == red) {
            p_ptr = s;
            if (s)
                s->parent = v->parent;
        } else {
            if (s) {
                p_ptr = s;
                s->parent = v->parent;
                erase_rebalance(s);
            } else {
                erase_rebalance(v);
                p_ptr = nullptr;
            }
        }
    }

    static void clear(Node* v) {
        if (!v)
            return;
        clear(v->left);
        clear(v->right);
        delete v;
    }

    static Node* clone(Node* v, Node* p = nullptr) {
        if (!v)
            return v;
        Node* res = new Node{v->value, nullptr, nullptr, p, v->color};
        try {
            res->left = clone(v->left, res);
            res->right = clone(v->right, res);
            return res;
        } catch (...) {
            clear(res->left);
            clear(res->right);
            throw;
        }
    }

public:
    Set() : _root(nullptr), _begin(nullptr), _size(0) {}

    Set(const Set& src) : Set() {
        _root = clone(src._root);
        _size = src._size;
        _begin = _root;

        if (_begin)
            while (_begin->left)
                _begin = _begin->left;
    }

    Set(Set&& src) : Set() {
        swap(*this, src);
    }

    Set& operator=(Set rhs) {
        swap(*this, rhs);
        return *this;
    }

    ~Set() {
        clear();
    }

    template<typename IIt>
    Set(IIt first, IIt last) : Set() {
        while (first != last)
            insert(*first++);
    }

    Set(std::initializer_list<T> init) : Set(init.begin(), init.end()) {}

    iterator begin() const {
        return iterator(_begin, this);
    }

    iterator end() const {
        return iterator(nullptr, this);
    }

    size_t size() const {
        return _size;
    }

    bool empty() const {
        return _size == 0;
    }

    iterator lower_bound(const T& value) const {
        Node* v = _root;
        Node* prev = nullptr;
        while (v) {
            if (v->value < value) {
                v = v->right;
            } else {
                prev = v;
                v = v->left;
            }
        }
        if (prev == nullptr)
            return end();
        return iterator(prev, this);
    }

    iterator find(const T& value) const {
        Node* v = _root;
        while (v) {
            if (value < v->value)
                v = v->left;
            else if (v->value < value)
                v = v->right;
            else
                return iterator(v, this);
        }
        return end();
    }

    void insert(T value) {
        Node* prev = nullptr;
        Node** cur = &_root;
        while (*cur) {
            if (value < (*cur)->value) {
                prev = *cur;
                cur = &(*cur)->left;
            } else if ((*cur)->value < value) {
                prev = *cur;
                cur = &(*cur)->right;
            } else
                return;
        }
        ++_size;
        *cur = new Node{std::move(value), nullptr, nullptr, prev, red};
        insert_rebalance(*cur);

        if (!_begin || value < _begin->value)
            _begin = *cur;
    }

    void erase(const T& value) {
        auto it = find(value);
        if (it == end())
            return;

        --_size;
        Node* v = it.cur;
        Node* u = v;
        if (v->left && v->right) {
            u = v->left;
            while (u->right)
                u = u->right;
        }

        std::swap(v->value, u->value);

        replace_with_child(u);
        delete u;

        if (_begin == u) {
            _begin = _root;
            if (_begin)
                while (_begin->left)
                    _begin = _begin->left;
        }
    }

    void clear() {
        clear(_root);
        _begin = nullptr;
        _root = nullptr;
        _size = 0;
    }

    friend void swap(Set& a, Set& b) {
        std::swap(a._root, b._root);
        std::swap(a._begin, b._begin);
        std::swap(a._size, b._size);
    }
};