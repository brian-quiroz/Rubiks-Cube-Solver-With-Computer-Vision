#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include "Node.h"
#include "PreconditionViolationException.h"

template <typename T>
class LinkedList {
	public:
		LinkedList();
		~LinkedList();
		int getLength() const;
		bool isEmpty() const;
		void addFront(T value);
		void addBack(T value);
		void insertAt(int pos, T value);
		void removeFront() throw(PreconditionViolationException);
		void removeBack() throw(PreconditionViolationException);
		void removeAt(int pos) throw(PreconditionViolationException);
		T getEntry(int pos) const throw(PreconditionViolationException);
	private:
		Node<T>* getNodeAt(int pos) const;
		Node<T>* m_front;
		int m_length;
};

#include "LinkedList.hpp"

#endif