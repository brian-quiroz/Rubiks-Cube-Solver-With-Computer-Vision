template <typename T>
Queue<T>::Queue() {
	q_front = nullptr;
	q_length = 0;
}

template <typename T>
Queue<T>::~Queue() {
	while (!isEmpty()) {
		dequeue();
	}
}

template <typename T>
bool Queue<T>::isEmpty() const {
	if (q_length == 0) {
		return (true);
	}
	return (false);
}

template <typename T>
int Queue<T>::getLength() {
	return (q_length);
}

template <typename T>
void Queue<T>::enqueue(const T value) throw(PreconditionViolationException) {
	Node<T>* temp = new Node<T>(value);
	Node<T>* jumper = q_front;
	if (isEmpty()) {
		q_front = temp;
	} else {
		for (int i = 1; i < getLength(); i++) {
			jumper = jumper->getNext();
		}
		jumper->setNext(temp);
	}
	q_length++;
}

template <typename T>
void Queue<T>::dequeue() throw(PreconditionViolationException) {
	Node<T>* temp = nullptr;
	if (isEmpty()) {
		throw(PreconditionViolationException("Error. Dequeue attempted on an empty queue!\n"));
	} else {
		temp = q_front;
		q_front = q_front->getNext();
		delete temp;
		q_length--;
	}
}

template <typename T>
T Queue<T>::peekFront() const throw(PreconditionViolationException) {
	if (isEmpty()) {
		throw(PreconditionViolationException("Error. PeekFront attempted on an empty queue!\n"));
	} else {
		return(q_front->getValue());
	}
}