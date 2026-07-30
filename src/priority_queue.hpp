#ifndef SJTU_PRIORITY_QUEUE_HPP
#define SJTU_PRIORITY_QUEUE_HPP

#include <cstddef>
#include <functional>
#include "exceptions.hpp"

namespace sjtu {
/**
 * @brief a container like std::priority_queue which is a heap internal.
 * **Exception Safety**: The `Compare` operation might throw exceptions for certain data.
 * In such cases, any ongoing operation should be terminated, and the priority queue should be restored to its original state before the operation began.
 */
template<typename T, class Compare = std::less<T>>
class priority_queue {
private:
	struct node {
		T value;
		node *left;
		node *right;
		std::size_t dist;
		std::size_t refcnt;

		explicit node(const T &v) : value(v), left(nullptr), right(nullptr), dist(1), refcnt(1) {}
	};

	node *root;
	std::size_t cnt;
	Compare comp;

	static std::size_t npl(node *p) {
		return p ? p->dist : 0;
	}

	static void add_ref(node *p) {
		if (p) {
			++p->refcnt;
		}
	}

	static void release(node *p) {
		if (!p) {
			return;
		}
		if (--p->refcnt == 0) {
			release(p->left);
			release(p->right);
			delete p;
		}
	}

	static void swap_ptr(node *&a, node *&b) {
		node *tmp = a;
		a = b;
		b = tmp;
	}

	node *merge_nodes(node *a, node *b) const {
		if (!a) {
			add_ref(b);
			return b;
		}
		if (!b) {
			add_ref(a);
			return a;
		}

		bool take_b;
		try {
			take_b = comp(a->value, b->value);
		} catch (...) {
			throw runtime_error();
		}
		if (take_b) {
			swap_ptr(a, b);
		}

		node *res = new node(a->value);
		add_ref(a->left);
		res->left = a->left;
		try {
			res->right = merge_nodes(a->right, b);
			if (npl(res->left) < npl(res->right)) {
				swap_ptr(res->left, res->right);
			}
			res->dist = npl(res->right) + 1;
		} catch (...) {
			release(res->left);
			delete res;
			throw;
		}
		return res;
	}

	void clear() {
		release(root);
		root = nullptr;
		cnt = 0;
	}

public:
	/**
	 * @brief default constructor
	 */
	priority_queue() : root(nullptr), cnt(0), comp(Compare()) {}

	/**
	 * @brief copy constructor
	 * @param other the priority_queue to be copied
	 */
	priority_queue(const priority_queue &other) : root(other.root), cnt(other.cnt), comp(other.comp) {
		add_ref(root);
	}

	/**
	 * @brief deconstructor
	 */
	~priority_queue() {
		clear();
	}

	/**
	 * @brief Assignment operator
	 * @param other the priority_queue to be assigned from
	 * @return a reference to this priority_queue after assignment
	 */
	priority_queue &operator=(const priority_queue &other) {
		if (this != &other) {
			release(root);
			root = other.root;
			cnt = other.cnt;
			comp = other.comp;
			add_ref(root);
		}
		return *this;
	}

	/**
	 * @brief get the top element of the priority queue.
	 * @return a reference of the top element.
	 * @throws container_is_empty if empty() returns true
	 */
	const T & top() const {
		if (empty()) {
			throw container_is_empty();
		}
		return root->value;
	}

	/**
	 * @brief push new element to the priority queue.
	 * @param e the element to be pushed
	 */
	void push(const T &e) {
		node *single = nullptr;
		node *old_root = root;
		try {
			single = new node(e);
			root = merge_nodes(old_root, single);
			++cnt;
			release(old_root);
			release(single);
		} catch (...) {
			release(single);
			throw;
		}
	}

	/**
	 * @brief delete the top element from the priority queue.
	 * @throws container_is_empty if empty() returns true
	 */
	void pop() {
		if (empty()) {
			throw container_is_empty();
		}
		node *old = root;
		node *merged = nullptr;
		try {
			merged = merge_nodes(root->left, root->right);
			root = merged;
			--cnt;
			release(old);
		} catch (...) {
			throw;
		}
	}

	/**
	 * @brief return the number of elements in the priority queue.
	 * @return the number of elements.
	 */
	std::size_t size() const {
		return cnt;
	}

	/**
	 * @brief check if the container is empty.
	 * @return true if it is empty, false otherwise.
	 */
	bool empty() const {
		return cnt == 0;
	}

	/**
	 * @brief merge another priority_queue into this one.
	 * The other priority_queue will be cleared after merging.
	 * The complexity is at most O(logn).
	 * @param other the priority_queue to be merged.
	 */
	void merge(priority_queue &other) {
		if (this == &other || other.empty()) {
			return;
		}
		if (empty()) {
			root = other.root;
			cnt = other.cnt;
			other.root = nullptr;
			other.cnt = 0;
			return;
		}

		node *old_root = root;
		node *other_root = other.root;
		node *new_root = merge_nodes(old_root, other_root);
		root = new_root;
		cnt += other.cnt;
		release(old_root);
		release(other_root);
		other.root = nullptr;
		other.cnt = 0;
	}
};

}

#endif
