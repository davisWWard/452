Davis Ward
200426114
ENEL 452 - Assignment 2

Functions added:
		Queue::insert(Data d, unsigned pos)
	this function inserts a data point at position pos
	pos is always positive, so no error checking is needed 
	for below the limit, but error checking does exist 
	for if the insert position is above the size of the list

		testFunction(Queue q1)
	This function tests the basic insert function, without
	checking errors, since errors throw an error

Errors:
	Errors can be checked by attempting to insert past the 
	current size limit