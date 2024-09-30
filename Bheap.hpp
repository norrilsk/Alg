/*
* Binomial Heap Implementation
* BheapNode<T> - Node for Bheap - required only for decrease key method
* Methods:
*   1. bool compare_less(std::shared_ptr<BheapNode> r) - return this->key < r->key;
*   2. T& get_key() - return key
* Bheap<T> - Binomial heap class
* Methods:
*   1. size_t size() - return heap size
*   2. const T &get_min() - return min element, doesn't pop it
*       complexity: O(lg(N))
*   3. void add_heap (Bheap<T> &H2)  - merge H2 into heap
*       NOTE: H2 is invalidated after merge
*       complexity: O(lg(N))
*    4. NodePtr insert(const T &d) - insert new element to heap
*       return std::shared_ptr<BheapNode> required for decrease key only
*    5. void decrease_key(NodePtr &x,const T &new_key)
*       decrease key for x
*       complexity: O(lg(N))
*    6. T pop() - pop element from heap
*       return this element to user
*       complexity: O(lg(N))
*
*/
#include <memory>
#include <exception>
#include <stdexcept>

namespace alg {
    template <typename T> class Bheap;

    template<typename T>
    class BheapNode {
        using NodePtr = std::shared_ptr<BheapNode>;
        NodePtr p;
        NodePtr child;
        NodePtr sibling;
        unsigned long long degree = 0;
        T key;
    public:
        inline bool compare_less(const NodePtr &r) {
            return key < r->key;
        }
        inline T& get_key() {
            return key;
        }
        friend class Bheap<T>;
    };

    template <typename T>
    class Bheap {
        using NodePtr = std::shared_ptr<BheapNode<T>>;
        NodePtr head;
        std::allocator<int> alloc;
        size_t _size = 0;

        inline void link_nodes (NodePtr &y,
                                NodePtr &z) {
            y->p = z;
            y->sibling  = z->child;
            z->child = y;
            z->degree++;
         }

        NodePtr merge_heads(NodePtr &h1,
                            NodePtr &h2) {
            NodePtr new_head, inserted;
            new_head = h1;
            inserted = h2;
            if (head->degree > h2->degree)
                std::swap(new_head,inserted);
            auto insert_point  = new_head;
            while (inserted) {
                //find place for insertion
                while (insert_point->sibling
                       && insert_point->sibling->degree < inserted->degree)
                    insert_point = insert_point->sibling;
                auto tmp = insert_point -> sibling;
                insert_point->sibling = inserted;
                auto next_inserted = inserted->sibling;
                inserted->sibling = tmp;
                inserted = next_inserted;
            }
            return new_head;
        }
        void add_heap_head (NodePtr &h2head) {
            NodePtr new_head;
             // merge heaps
            if (!h2head)
                return;
            if (!head) {
                head = h2head;
                return;
            }
            new_head = merge_heads(head,h2head);
            NodePtr x, next_x, prev_x;
            x = new_head;
            next_x = x->sibling;
            while (next_x) {
                if (x->degree != next_x->degree
                    || (next_x->sibling && next_x->sibling->degree == x->degree)) {
                    prev_x = x; // case 1 and 2
                    x = next_x; // case 1 and 2
                } else {
                    if (x->compare_less(next_x)) {
                        x->sibling = next_x->sibling; // case 3
                        link_nodes(next_x, x); // case 3
                    } else {
                        if (!prev_x) { //case 4
                            new_head = next_x; // case 4
                        } else {
                            prev_x->sibling = next_x; //case 4
                        }
                        link_nodes(x,next_x); // case 4
                        x = next_x; // case 4
                    }
                }
                next_x = x->sibling;
            }

            head = new_head;
        };
        NodePtr &get_min_node() {
            NodePtr x,min;
            x  = head->sibling;
            min = head;
            while (x) {
                if (x->compare_less(min)) {
                    min = x;
                }
                x = x->sibling;
            }
            return min;
        };
    public:
        size_t size() {
            return _size;
        }
        // we don't copy data here
        // H2 is invalidated;
        void add_heap (Bheap<T> &H2) {
            add_heap_head(H2.head);
        }



        const T &get_min() {
            return get_min_node()->get_key();
        };

        NodePtr insert(const T &d) {
            NodePtr x = std::allocate_shared<BheapNode<T>>(alloc);
            x->key = d;
            add_heap_head(x);
            _size++;
            return x;
        };

        T pop() {
            if (_size < 1)
                throw std::out_of_range("Pop from empty heap");
            NodePtr x,prev_x,min,prev_min;
            x  = head->sibling;
            prev_min = head;
            prev_x = head;
            min = head;
            while (x) {
                if (x->compare_less(min)) {
                    min = x;
                    prev_min = prev_x;
                }
                prev_x = x;
                x = x->sibling;
            }

            if (head == min) {
                head = min->sibling;
            } else {
                prev_min->sibling = min->sibling;
            }

            NodePtr add_head = min->child;
            x = add_head;
            while (x) {
                x->p = nullptr;
                x = x->sibling;
            }
            add_heap_head(add_head);
            _size--;
            return min->key;
        };

        void decrease_key(NodePtr &x,const T &new_key) {
            if (x->key < new_key)
                throw std::out_of_range("Bheap key can't be increased");
            x->key = new_key;
            NodePtr &y = x;
            NodePtr &z = y->p;
            while (z != nullptr && y->compare_less(z)) {
                std::swap(y->key, z->key);
                y = z;
                z = y->p;
            }
        };
    };
}
