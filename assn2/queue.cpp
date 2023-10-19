/**
   Project: Implementation of a Queue in C++.
   Programmer: Karim Naqvi
   Course: enel452
*/

#include "queue.h"
#include <iostream>
#include <cstdlib>              // for exit

using namespace std;

Queue::Queue()
{
    head = 0;
    tail = 0;
    nelements = 0;
    verbose = false;
}

Queue::~Queue()
{
    for (QElement* qe = head; qe != 0;)
    {
	QElement* temp = qe;
	qe = qe->next;
	delete(temp);
    }
}

void Queue::remove(Data* d)
{
    if (size() > 0)
    {
        QElement* qe = head;
        head = head->next;
        nelements--;
        *d = qe->data;
	delete qe;
    }
    else
    {
        cerr << "Error: Queue is empty.\n";
        exit(1);
    }
}

void Queue::insert(Data d)
{
    if (verbose) std::cout << "insert(d)\n";
    QElement* el = new QElement(d);
    if (size() > 0)
    {
        tail->next = el;
    }
    else
    {
        head = el;
    }
    tail = el;
    nelements++;
}

// NEW STUFF ***********************************************************
void Queue::insert(Data d, unsigned pos)
{
    // break if pos < 0 or pos > size, 
    // meaining insert before the start or after the end
    // exit with error code 3
    if (pos > size())
    {
        fprintf(stderr, "insert: range error.");
        exit(3);
    }
    if (verbose) std::cout << "insert(d)\n";
    QElement* el = new QElement(d);
    QElement* cur = head;
    if (size() > 0)
    {
        for (unsigned i = 0; i < pos - 1; i++)
        {
            cur = cur->next;
        }
        el->next = cur->next;
        cur->next = el;
    }
    else
    {
        head = el;
    }
    if (pos == size()) tail = el;
    nelements++;
}

void testFunction(Queue q1)
{
    q1.insert(Data(1, 2));
    q1.insert(Data(3, 4));
    q1.insert(Data(5, 6));
    q1.insert(Data(-2, -3), 1);
    q1.insert(Data(-4, -5), 3);

    q1.print();
}
// END OF NEW STUFF ****************************************************

bool Queue::search(Data otherData) const
{
    QElement* insideEl = head;
    for (int i = 0; i < nelements; i++)
    {
        if (insideEl->data.equals(otherData))
            return true;
        insideEl = insideEl->next;
    }
    return false;
}

void Queue::print() const
{
    QElement* qe = head;
    if (size() > 0)
    {
        for (unsigned i = 0; i < size(); i++)
        {
            cout << i << ":(" << qe->data.x << "," << qe->data.y << ") ";
            qe = qe->next;
        }
    }
    cout << "\n";
}

unsigned Queue::size() const
{
    return nelements;
}
