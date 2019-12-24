#ifndef QUEUE_H
#define QUEUE_H

#include "Node.h"
#include "PreconditionViolationException.h"

template <typename T>
class Queue {
	public:
		Queue();
		~Queue();
		bool isEmpty() const;
		int getLength();
		void enqueue(const T value) throw(PreconditionViolationException);
		void dequeue() throw(PreconditionViolationException);
		T peekFront() const throw(PreconditionViolationException);
	private:
		Node<T>* q_front;
		int q_length;
};

#include "Queue.hpp"

#endif