/*
* Fibonacci Heap Implementation
* FibHeapNode<T> - Node for FibHeap - required only for decrease key method
* Methods:
*   1. bool compare_less(std::shared_ptr<FibHeapNode> r) - return this->key < r->key;
*   2. T& get_key() - return key
* FibHeap<T> - Binomial heap class
* Methods:
*   1. size_t size() - return heap size
*   2. const T &get_min() - return min element, doesn't pop it
*       complexity: O(1)
*   3. void add_heap (FibHeap<T> &H2)  - merge H2 into heap
*       NOTE: H2 is invalidated after merge
*       complexity: O(lg(N))*
*    4. NodePtr insert(const T &d) - insert new element to heap
*       return std::shared_ptr<FibHeapNode> required for decrease key only
*    5. void decrease_key(NodePtr &x,const T &new_key)
*       decrease key for x
*       complexity: O(1)**
*    6. T pop() - pop element from heap
*       return this element to user
*       complexity: O(lg(N))*
*    * in worst case O(N)
*    ** in worst case O(lg(N))
*/
#ifndef _ALG_FIB_HEAP
#define _AlG_FIB_HEAP

#include <memory>
#include <exception>
#include <stdexcept>
#include <vector>

namespace alg {
    template <typename T> class FibHeap;

    template<typename T>
    class FibHeapNode {
        using NodePtr = std::shared_ptr<FibHeapNode>;:
        NodePtr p;
        NodePtr child;
        NodePtr left;
        NodePtr right;
        size_t degree = 0;
        T key;
        bool mark = false;
    public:
        inline bool compare_less(const NodePtr &r) const noexcept {
            return key < r->key;
        }
        inline T &get_key() const noexcept {
            return key;
        }
        friend class FibHeap<T>;
    };

    template <typename T>
    class FibHeap {
        using NodePtr = std::shared_ptr<FibHeapNode<T>>;
        std::allocator<T> alloc;
        NodePtr min;
        size_t _size = 0;

        void insert_node(NodePtr &x) noexcept {
            if (!min)
                min = x;
            auto l = min->left;
            l->right = x;
            x->left = l;
            min->left = x;
            x->right = min;

            if (x->compare_less(min)) {
                min = x;
            }
        }

        void fib_link(NodePtr &y,  NodePtr &x) noexcept {
            // remove y from H
            y->right->left = y->left;
            y->left->right = y->right;
            y->left = y;
            y->right = y;
            if (!x->child) {
                x->child = y;
                y->p = x;
            } else {
                auto r = x->child->right;
                x->child->right = y;
                y->left = x->child;
                r->left = y;
                y->right = r;
                y->p = x;
            }
            x->degree++;
            // TODO: why we need this false marking?
            y->mark = false;
        }

        void consolidate() noexcept {
            //calc num_trees probably can be optimized
            size_t num_trees = 0;
            NodePtr x = min;
            std::vector<NodePtr> A, orig_nodes;
            size_t max_d = 0;
            if (x) do {
                max_d = std::max(max_d,x->degree);
                num_trees++;
                orig_nodes.push_back(x);
                x = x->right;
            } while (x != min);
            //TODO: may be ambigious
            size_t node_vec_size = max_d + num_trees;
            A.resize(node_vec_size);
            for (auto &x : orig_nodes) {
                auto d = x->degree;
                while (A[d] != nullptr) {
                    auto y = A[d];
                    if (y->compare_less(x))
                        std::swap(x,y);
                    fib_link(y,x);
                    A[d] = nullptr;
                    d++;
                }
                A[d] = x;
            }
            min = nullptr;
            for (int i = 0; i < node_vec_size; i++) {
                if (A[i]) {
                    insert_node(A[i]);
                }
            }
        }
        void cut (NodePtr &x, NodePtr &y) noexcept {
            //delete x from y;
            if (x->right !=x) {
                y->child = x->right;
                x->right->left = x->left;
                x->left->right = x->right;
            } else {
                y->child = nullptr;
            }
            y->degree--;
            x->p = nullptr;
            x->mark = false;
            insert_node(x);
        }

        void cascading_cut(NodePtr &y) noexcept {
            auto z = y->p;
            if (!z)
                return;
            if (y->mark) {
                cut(y,z);
                cascading_cut(z);
            } else {
                y->mark = true;
            }
        }

    public:
        size_t size() const noexcept {
            return _size;
        }
        NodePtr insert(const T &key) {
            NodePtr x = std::allocate_shared<FibHeapNode<T>>(alloc);
            x->key = key;
            x->left = x;
            x->right = x;
            insert_node(x);
            _size++;
            return x;
        }
        // NOTE: H is invalidated after merge
        void add_heap(FibHeap<T> &H) {
            NodePtr merged = H.min;
            if (!merged)
                return;
            do {
                insert_node(merged);
                merged = merged->right;
            } while (merged != H.min);
            return;
        }
        T &get_min() const noexcept {
            return min->key;
        }

        T pop() {
            auto z = min;
            if (!z)
                throw std::out_of_range("Pop from empty FibHeap");
            auto x = z->child;
            auto next_x = x;
            if (x) do {
                x = next_x;
                x->p = nullptr;
                next_x = x->right;
                x->left->right = x->left;
                x->right->left = x->right;
                insert_node(x);
            } while (next_x != x);
            min = z->right;
            if (min == z)
                min = nullptr;
            if (z->right  != z)
                z->right->left = z->left;
            if (z->left != z)
                z->left->right = z->right;
            _size--;
            consolidate();
            return z->key;
        };

        void decrease_key(NodePtr &N, const T &new_key) {
            if (N->key < new_key)
                throw std::out_of_range("FibHeap key can't be increased");
            N->key = new_key;
            auto y = N->p;
            if (y != nullptr && N->compare_less(y)) {
                cut(N,y);
                cascading_cut(y);
            }
            if (N->compare_less(min))
                min = N;
        }
    };
};

#endif // _AlG_FIB_HEAP
