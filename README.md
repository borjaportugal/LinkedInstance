# LinkedInstance
The LinkedInstance class allows the user to track all instances of a given type.

# How does it?
It is done using a linked list, each instance stores a _next_ and _prev_ pointer to other instances of this type.
The _head_ and _tail_ of the list are static, as the LinkedInstance class is templated we have a pair of these per type we want to track.
The current implementation uses a double linked list in order to track destruction of the instances, in some cases a singly linked list is enough.

# Why would you want that?
There are some examples in [example_main.cpp](https://github.com/borjaportugal/LinkedInstance/blob/master/examples_main.cpp).
Personally I found these features useful in big projects when you have to register meta data or creators for types that are all over the project. 
By using the principle of the LinkedInstance class you automatically have access to all this data to register it on the initialize function.


