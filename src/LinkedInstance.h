
#pragma once

#include <type_traits>	// std::enable_if

/// \brief	This class manages all instances of type T in a linked list.
template <typename T>
class LinkedInstance
{
public:
	using value_type = T;

public:
	LinkedInstance() { link(); }
	LinkedInstance(const LinkedInstance &) { link(); }
	virtual ~LinkedInstance() { unlink(); }

private:
	void link()
	{
		static_assert(std::is_base_of<LinkedInstance<T>, T>::value,
					  "LinkedInstance<T> is meant to be the base class of T");

		m_prev = s_tail;
		if (s_tail)	s_tail->m_next = static_cast<T *>(this);
		s_tail = static_cast<T *>(this);

		if (!s_head) s_head = static_cast<T *>(this);
	}
	void unlink()
	{
		if (m_prev)
			m_prev->m_next = m_next;
		else
			s_head = m_next;

		m_next = nullptr;
		m_prev = nullptr;
	}

	T * m_next{ nullptr };
	T * m_prev{ nullptr };

	static T * s_head;
	static T * s_tail;

	// accessors for the members of this class 
	// putting the variables as public or protected would pollute the user type T
	template <typename T> friend T * linked_instance_head();
	template <typename T> friend T * linked_instance_tail();
	template <typename T> friend T * linked_instance_prev(const LinkedInstance<T> & instance);
	template <typename T> friend T * linked_instance_next(const LinkedInstance<T> & instance);
};

template <typename T>
T * LinkedInstance<T>::s_head = nullptr;
template <typename T>
T * LinkedInstance<T>::s_tail = nullptr;

template <typename T>
T * linked_instance_head()
{
	static_assert(std::is_base_of<LinkedInstance<T>, T>::value,
				  "LinkedInstance<T> is meant to be the base class of T");
	return LinkedInstance<T>::s_head;
}
template <typename T>
T * linked_instance_tail()
{
	static_assert(std::is_base_of<LinkedInstance<T>, T>::value,
				  "LinkedInstance<T> is meant to be the base class of T");
	return LinkedInstance<T>::s_tail;
}
template <typename T>
T * linked_instance_next(const LinkedInstance<T> & instance)
{
	return instance.m_next;
}
template <typename T>
T * linked_instance_prev(const LinkedInstance<T> & instance)
{
	return instance.m_prev;
}


