#include <bits/stdc++.h>
using namespace std;

// ESet: a set-like container implemented with a randomized treap.
// Supports iterator with O(log n) ++/-- via root-descend searches.

template <class Key, class Compare = std::less<Key>>
class ESet {
    struct Node {
        Key key;
        uint32_t prio;
        int sz;
        Node *l, *r;
        Node(const Key &k, uint32_t p) : key(k), prio(p), sz(1), l(nullptr), r(nullptr) {}
    };

    Node *root = nullptr;
    Compare comp;

    static int getsz(Node *x) { return x ? x->sz : 0; }
    static void pull(Node *x) {
        if (x) x->sz = 1 + getsz(x->l) + getsz(x->r);
    }

    static Node* rotate_right(Node *y) {
        Node *x = y->l; // assume x
        y->l = x->r;
        x->r = y;
        pull(y); pull(x);
        return x;
    }
    static Node* rotate_left(Node *x) {
        Node *y = x->r;
        x->r = y->l;
        y->l = x;
        pull(x); pull(y);
        return y;
    }

    // insert unique; returns pair<root, inserted>
    pair<Node*, bool> insert_unique(Node *cur, const Key &k, std::mt19937 &rng) {
        if (!cur) {
            return { new Node(k, rng()), true }; // prio from rng
        }
        if (!comp(k, cur->key) && !comp(cur->key, k)) {
            return { cur, false };
        }
        if (comp(k, cur->key)) {
            auto res = insert_unique(cur->l, k, rng);
            cur->l = res.first;
            pull(cur);
            if (cur->l && cur->l->prio > cur->prio) cur = rotate_right(cur);
            pull(cur);
            return { cur, res.second };
        } else {
            auto res = insert_unique(cur->r, k, rng);
            cur->r = res.first;
            pull(cur);
            if (cur->r && cur->r->prio > cur->prio) cur = rotate_left(cur);
            pull(cur);
            return { cur, res.second };
        }
    }

    // erase key; returns pair<root, erased>
    pair<Node*, bool> erase_key(Node *cur, const Key &k) {
        if (!cur) return { nullptr, false };
        if (!comp(k, cur->key) && !comp(cur->key, k)) {
            Node *ret = merge(cur->l, cur->r);
            delete cur;
            return { ret, true };
        }
        if (comp(k, cur->key)) {
            auto res = erase_key(cur->l, k);
            cur->l = res.first; pull(cur);
            return { cur, res.second };
        } else {
            auto res = erase_key(cur->r, k);
            cur->r = res.first; pull(cur);
            return { cur, res.second };
        }
    }

    static Node* merge(Node *a, Node *b) {
        if (!a) return b; if (!b) return a;
        if (a->prio > b->prio) {
            a->r = merge(a->r, b); pull(a); return a;
        } else {
            b->l = merge(a, b->l); pull(b); return b;
        }
    }

    static Node* find_node(Node *cur, const Key &k, Compare &comp) {
        while (cur) {
            if (!comp(k, cur->key) && !comp(cur->key, k)) return cur;
            if (comp(k, cur->key)) cur = cur->l; else cur = cur->r;
        }
        return nullptr;
    }

    static Node* lower_node(Node *cur, const Key &k, Compare &comp) {
        Node *ans = nullptr;
        while (cur) {
            if (!comp(cur->key, k)) { ans = cur; cur = cur->l; }
            else cur = cur->r;
        }
        return ans;
    }
    static Node* upper_node(Node *cur, const Key &k, Compare &comp) {
        Node *ans = nullptr;
        while (cur) {
            if (comp(k, cur->key)) { ans = cur; cur = cur->l; }
            else cur = cur->r;
        }
        return ans;
    }

    static Node* min_node(Node *cur) {
        if (!cur) return nullptr; while (cur->l) cur = cur->l; return cur;
    }
    static Node* max_node(Node *cur) {
        if (!cur) return nullptr; while (cur->r) cur = cur->r; return cur;
    }

    static Node* successor(Node *root, const Key &k, Compare &comp) {
        Node *ans = nullptr; Node *cur = root;
        while (cur) {
            if (comp(k, cur->key)) { ans = cur; cur = cur->l; }
            else cur = cur->r;
        }
        return ans;
    }
    static Node* predecessor(Node *root, const Key &k, Compare &comp) {
        Node *ans = nullptr; Node *cur = root;
        while (cur) {
            if (comp(cur->key, k)) { ans = cur; cur = cur->r; }
            else cur = cur->l;
        }
        return ans;
    }

    static int count_lt(Node *cur, const Key &k, Compare &comp) {
        int res = 0;
        while (cur) {
            if (comp(cur->key, k)) { res += 1 + getsz(cur->l); cur = cur->r; }
            else cur = cur->l;
        }
        return res;
    }

    static void destroy(Node *cur) {
        if (!cur) return;
        destroy(cur->l); destroy(cur->r);
        delete cur;
    }

    static Node* clone(Node *cur) {
        if (!cur) return nullptr;
        Node *n = new Node(cur->key, cur->prio);
        n->l = clone(cur->l);
        n->r = clone(cur->r);
        pull(n);
        return n;
    }

    std::mt19937 rng;

public:
    class iterator {
        const ESet *owner = nullptr;
        Key val;
        bool is_end = true;
    public:
        iterator() = default;
        iterator(const ESet *o, bool e, const Key &v) : owner(o), val(v), is_end(e) {}
        const Key &operator*() const {
            if (!owner || is_end) throw std::out_of_range("Dereferencing end iterator");
            return val;
        }
        iterator& operator++() {
            if (!owner) return *this;
            if (is_end) return *this;
            auto nxt = successor(owner->root, val, owner->comp);
            if (!nxt) { is_end = true; }
            else { val = nxt->key; }
            return *this;
        }
        iterator operator++(int) { iterator tmp=*this; ++(*this); return tmp; }
        iterator& operator--() {
            if (!owner) return *this;
            if (is_end) {
                auto mx = max_node(owner->root);
                if (mx) { is_end = false; val = mx->key; }
                return *this;
            }
            auto prv = predecessor(owner->root, val, owner->comp);
            if (prv) { val = prv->key; }
            return *this;
        }
        iterator operator--(int) { iterator tmp=*this; --(*this); return tmp; }
        bool operator==(const iterator &o) const {
            if (owner != o.owner) return false;
            if (is_end && o.is_end) return true;
            if (is_end != o.is_end) return false;
            // both not end
            return (!owner->comp(val, o.val) && !owner->comp(o.val, val));
        }
        bool operator!=(const iterator &o) const { return !(*this == o); }
    };

    ESet() : root(nullptr), comp(Compare()), rng(0xC0FFEEu) {}
    ~ESet() { destroy(root); root = nullptr; }

    ESet(const ESet &other) : comp(other.comp), rng(0xC0FFEEu) {
        root = clone(other.root);
    }
    ESet& operator=(const ESet &other) {
        if (this == &other) return *this;
        destroy(root); root = clone(other.root);
        comp = other.comp; // comparator copied by value
        return *this;
    }

    ESet(ESet &&other) noexcept : root(other.root), comp(std::move(other.comp)), rng(0xC0FFEEu) {
        other.root = nullptr;
    }
    ESet& operator=(ESet &&other) noexcept {
        if (this == &other) return *this;
        destroy(root);
        root = other.root; comp = std::move(other.comp);
        other.root = nullptr;
        return *this;
    }

    pair<iterator,bool> emplace(const Key &k) {
        auto res = insert_unique(root, k, rng);
        root = res.first;
        if (!res.second) {
            Node *f = find_node(root, k, comp);
            return { iterator(this, f==nullptr, f?f->key:Key{}), false };
        }
        // inserted, iterator at k
        return { iterator(this, false, k), true };
    }

    size_t erase(const Key &k) {
        auto res = erase_key(root, k);
        root = res.first;
        return res.second ? 1u : 0u;
    }

    iterator find(const Key &k) const {
        Node *f = find_node(root, k, const_cast<Compare&>(comp));
        if (!f) return end();
        return iterator(this, false, f->key);
        }

    size_t size() const noexcept { return (size_t)getsz(root); }

    iterator lower_bound(const Key &k) const {
        Node *n = lower_node(root, k, const_cast<Compare&>(comp));
        if (!n) return end();
        return iterator(this, false, n->key);
    }
    iterator upper_bound(const Key &k) const {
        Node *n = upper_node(root, k, const_cast<Compare&>(comp));
        if (!n) return end();
        return iterator(this, false, n->key);
    }

    iterator begin() const noexcept {
        Node *mn = min_node(root);
        if (!mn) return end();
        return iterator(this, false, mn->key);
    }
    iterator end() const noexcept { return iterator(this, true, Key{}); }

    size_t range(const Key &l, const Key &r) const {
        // return count in [l,r]
        if (comp(r, l)) return 0;
        int lt_l = count_lt(root, l, const_cast<Compare&>(comp));
        // check existence of r
        int le_r = count_lt(root, r, const_cast<Compare&>(comp));
        Node *fr = find_node(root, r, const_cast<Compare&>(comp));
        if (fr) le_r += 1;
        int res = le_r - lt_l;
        if (res < 0) res = 0;
        return (size_t)res;
    }
};

int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    // Implement the speedtest interface as per ESet test harness.
    ESet<long long> s[25];
    typename ESet<long long>::iterator it;
    int op; int lst=0; int it_a=-1; int valid=0; int cnt=1;
    while ( (cin >> op) ) {
        long long a,b,c;
        switch(op){
            case 0:{
                cin >> a >> b;
                auto p = s[a].emplace(b);
                if (p.second) {
                    it_a = (int)a;
                    it = p.first;
                    valid = 1;
                }
                break;
            }
            case 1:{
                cin >> a >> b;
                if (valid && it_a==(int)a) {
                    try {
                        if (*it == b) valid = 0;
                    } catch (...) {
                        // end iterator deref: ignore
                    }
                }
                s[a].erase(b);
                break;
            }
            case 2:{
                cin >> a;
                s[++lst] = s[a];
                break;
            }
            case 3:{
                cin >> a >> b;
                auto it2 = s[a].find(b);
                if (it2 != s[a].end()) {
                    cout << "true\n";
                    it_a = (int)a;
                    it = it2;
                    valid = 1;
                } else {
                    cout << "false\n";
                }
                cnt++;
                break;
            }
            case 4:{
                cin >> a >> b >> c;
                cout << s[a].range(b, c) << "\n";
                cnt++;
                break;
            }
            case 5:{
                if (valid){
                    auto it2 = it;
                    auto tmp = it2; --tmp; // pre-check as in reference
                    if (it == tmp) valid = 0;
                }
                if (valid) {
                    auto t = --it; // move
                    (void)t; // unused temp
                    try { cout << *it << "\n"; }
                    catch(...) { cout << -1 << "\n"; }
                } else {
                    cout << -1 << "\n";
                }
                cnt++;
                break;
            }
            case 6:{
                if (valid) {
                    auto it2 = ++it;
                    (void)it2;
                    auto it3 = it; ++it3;
                    if (it == it3) valid = 0; else {
                        try { cout << *it << "\n"; }
                        catch(...) { cout << -1 << "\n"; }
                    }
                }
                if (!valid) cout << -1 << "\n";
                cnt++;
                break;
            }
            default: break;
        }
    }
    return 0;
}
