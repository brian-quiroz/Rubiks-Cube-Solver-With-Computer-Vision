template <typename T>
LinkedList<T>::LinkedList() {
	m_front = nullptr;
	m_length = 0;
}

template <typename T>
LinkedList<T>::~LinkedList() {
	while (!isEmpty()) {
		removeBack();
	}
}

template <typename T>
int LinkedList<T>::getLength() const {
	return (m_length);
}

template <typename T>
bool LinkedList<T>::isEmpty() const {
	if (m_length == 0) {
		return (true);
	}
	return (false);
}

template <typename T>
void LinkedList<T>::addFront(T value) {
	Node<T>* temp = new Node<T>(value);
	if(!isEmpty()) {
		temp->setNext(m_front);
	}
	m_front = temp;
	m_length++;
}

template <typename T>
void LinkedList<T>::addBack(T value) {
	Node<T>* temp = new Node<T>(value);
	Node<T>* jumper = m_front;
	if (isEmpty()) {
		m_front = temp;
	} else {
		for (int i = 1; i < m_length; i++) {
			jumper = jumper->getNext();
		}
		jumper->setNext(temp);
	}
	m_length++;
}

template <typename T>
void LinkedList<T>::insertAt(int pos, T value) {
	if ((isEmpty()) || (pos == 1)) {
		addFront(value);
	} else if (pos == m_length + 1) {
		addBack(value);
	} else if ((pos < 1) || (pos > m_length + 1)) {
		throw(PreconditionViolationException("Error! insertAt attempted on an empty list or position out of range!\n"));
	} else {
		Node<T>* temp = new Node<T>(value);
		Node<T>* jumper = m_front;
		for (int i = 2; i < pos; i++) {
			jumper = jumper->getNext();
		}
		temp->setNext(jumper->getNext());
		jumper->setNext(temp);
		m_length++;
	}
}

template <typename T>
void LinkedList<T>::removeFront() throw(PreconditionViolationException) {
	if (isEmpty()) {
		throw(PreconditionViolationException("Error! removeFront attempted on an empty list!\n"));
	} else {
		Node<T>* temp = m_front;
		m_front = m_front->getNext();
		delete temp;
		m_length--;
	}
}

template <typename T>
void LinkedList<T>::removeBack() throw(PreconditionViolationException) {
	if (isEmpty()) {
		throw(PreconditionViolationException("Error! removeBack attempted on an empty list!\n"));
	} else if (m_length == 1) {
		removeFront();
	} else {
		Node<T>* temp = nullptr;
		Node<T>* jumper = m_front;
		for (int i = 1; i < m_length; i++) {
			temp = jumper;
			jumper = jumper->getNext();
		}
		delete jumper;
		temp->setNext(nullptr);
		m_length--;
	}
}

template <typename T>
void LinkedList<T>::removeAt(int position) throw(PreconditionViolationException) {
	if ((isEmpty()) || (position < 1) || (position > getLength())) {
		throw(PreconditionViolationException("Error! removeAt attempted on an empty list or position out of range!\n"));
	} else if (position == 1) {
		removeFront();
	} else if (position == getLength()) {
		removeBack();
	} else {
		Node<T>* temp = getNodeAt(position-1);
		Node<T>* temp2 = temp->getNext();
		temp->setNext(temp2->getNext());
		delete temp2;
		m_length--;
	}
}

template <typename T>
T LinkedList<T>::getEntry(int pos) const throw(PreconditionViolationException) {
	Node<T>* temp = m_front;
	if (pos < 1 || pos > m_length) {
		throw(PreconditionViolationException("Error! getEntry attempted on an empty list or position out of range!\n"));
	} else {
		for (int i = 1; i < pos; i++) {
			temp = temp->getNext();
		}
	}
 	return (temp->getValue());
}

template <typename T>
Node<T>* LinkedList<T>::getNodeAt(int pos) const {
	Node<T>* curPtr = m_front;
	for (int i = 1; i < pos; i++) {
		curPtr = curPtr->getNext();
	}
	return (curPtr);
}