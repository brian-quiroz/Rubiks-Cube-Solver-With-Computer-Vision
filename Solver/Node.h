#ifndef NODE_H
#define NODE_H

template <typename T>
class Node {
	public:
		Node(T value);
		T getValue() const;
		void setValue(T value);
		Node<T>* getNext() const;
		void setNext(Node<T>* next);
	private:
		T m_value;
		Node<T>* m_next;
};

#include "Node.hpp"

#endif