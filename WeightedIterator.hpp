/*
 * An iterator for sorting a family of subsets by weight. 
 * Adapted from the weighted_iterator implementation of SAPPOROBDD::ZBDD in the Graphillion library
 */ 

#pragma once

#include <set>
#include <map>
#include <vector>
#include <iterator>

#include <ZBDD.h>
#include <SBDD_helper.h>

// weighted_iterator for ZBDD
template<typename V = int, typename T = V>
class weighted_iterator : public std::iterator<std::forward_iterator_tag,std::set<int32_t>> {
    ZBDD zdd_;
    V curr_weight_;
    int32_t num_elems_;
    bool is_maximizing;
    std::set<int32_t> s_;
    std::vector<T> weights_;    // weights_[0] is not used
    
public:
    weighted_iterator(const ZBDD& zdd_, std::vector<T> weights, bool is_maximizing=true) : 
        zdd_(zdd_), s_(std::set<int32_t>()), weights_(weights), 
        num_elems_(zdd_.Top()), is_maximizing(is_maximizing) {
        if (!is_maximizing) {
            std::vector<T> inverted_weights_;
            for (typename std::vector<T>::const_iterator i = weights.begin(); i != weights.end(); ++i)
                inverted_weights_.push_back(-1 * (*i));
            weights_ = inverted_weights_;
        }
        this->next();
    }

    bool operator==(const weighted_iterator& i) const { 
        return (this->zdd_ == i.zdd_) && 
                (this->weights_ == i.weights_) && 
                (this->is_maximizing == i.is_maximizing); 
    }
    
    bool operator!=(const weighted_iterator& i) const {
        return (this->zdd_ != i.zdd_) || 
                (this->weights_ != i.weights_) || 
                (this->is_maximizing != i.is_maximizing); 
    }

    std::set<int32_t>& operator*() { 
        return this->s_; 
    }
    
    weighted_iterator& operator++() {
        this->next();
        return *this;
    }

    V curr_weight() {
        return this->curr_weight_;
    }

    void next() {
        if (this->zdd_ == null() || this->zdd_ == bot()) {
            this->zdd_ = null();
            this->curr_weight_ = (is_maximizing ? std::numeric_limits<V>::min() : std::numeric_limits<V>::max());
            this->s_ = std::set<int32_t>();
        } else {
            V curr_weight;
            std::set<int32_t> s;
            this->zdd_ -= choose_best(this->zdd_, this->weights_, &s, curr_weight);
            this->curr_weight_ = (is_maximizing ? curr_weight : - curr_weight);
            this->s_ = s;
        }
    }

private:
    inline ZBDD null() { return ZBDD(-1); }
    inline ZBDD bot() { return ZBDD(0); }
    inline ZBDD top() { return ZBDD(1); }
    inline bddword id(ZBDD f) { return f.GetID(); }
    inline bool is_term(ZBDD f) { return f.Top() == 0; }

    inline ZBDD lo(ZBDD f) {
        assert(!is_term(f));
        return f.OffSet(f.Top());
    }

    inline ZBDD hi(ZBDD f) {
        assert(!is_term(f));
        return f.OnSet0(f.Top());
    }

    inline int32_t elem(ZBDD f) {
        assert(!is_term(f));
        return f.Top();
    }

    ZBDD single(int32_t e) {
        assert(e > 0);
        return top().Change(e);
    }

    ZBDD choose_best(ZBDD f, const std::vector<T>& weights, std::set<int32_t>* s, V& curr_weight) {
        assert(s != NULL);
        if (f == bot()) return bot();
        std::vector<bool> x;
        algo_b(f, weights, &x);
        ZBDD g = top();
        curr_weight = 0;
        s->clear();
        for (int32_t j = 1; j < static_cast<int32_t>(x.size()); ++j) {
            if (x[j]) {
                curr_weight += weights[j];
                g = g * single(j);
                s->insert(j);
            }
        }
        return g;
    }

    // Algorithm B modified for ZDD, from Knuth Vol. 4 Fascicle 1 Sec. 7.1.4.
    void algo_b(ZBDD f, const std::vector<T>& w, std::vector<bool>* x) {
        assert(x != NULL);
        assert(f != bot());
        if (f == top()) return;
        std::vector<std::vector<ZBDD>> stacks(num_elems_ + 1);
        std::set<bddword> visited;
        int32_t max_elem = 0;
        sort_zdd(f, &stacks, &visited, &max_elem);
        assert(w.size() > static_cast<size_t>(max_elem));
        x->clear();
        x->resize(max_elem + 1, false);
        std::map<bddword,bool> t;
        std::map<bddword,V> ms;
        ms[id(bot())] = std::numeric_limits<V>::min();
        ms[id(top())] = 0;
        for (int32_t v = 1; v <= max_elem; ++v) {        // Reversed from Graphillion
            while (!stacks[v].empty()) {
                ZBDD g = stacks[v].back();
                stacks[v].pop_back();
                bddword k = id(g);
                int32_t v = elem(g);
                bddword l = id(lo(g));
                bddword h = id(hi(g));
                if (lo(g) != bot())
                    ms[k] = ms.at(l);
                if (hi(g) != bot()) {
                    V m = ms.at(h) + w[v];
                    if (lo(g) == bot() || m > ms.at(k)) {
                        ms[k] = m;
                        t[k] = true;
                    }
                }
            }
        }
        while (!is_term(f)) {
            bddword k = id(f);
            int32_t v = elem(f);
            if (t.find(k) == t.end())
                t[k] = false;
            (*x)[v] = t.at(k);
            f = !t.at(k) ? lo(f) : hi(f);
        }
    }

    void sort_zdd(ZBDD f, std::vector<std::vector<ZBDD>>* stacks, std::set<bddword>* visited, int32_t* max_elem) {
        assert(stacks != NULL && visited != NULL);
        if (is_term(f)) return;
        if (visited->find(id(f)) != visited->end()) return;
        (*stacks)[elem(f)].push_back(f);
        visited->insert(id(f));
        if (max_elem != NULL && elem(f) > *max_elem) *max_elem = elem(f);
        sort_zdd(lo(f), stacks, visited, max_elem);
        sort_zdd(hi(f), stacks, visited, max_elem);
    }
};