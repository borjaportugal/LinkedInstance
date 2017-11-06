
#pragma once

#include "LinkedInstance.h"

#define LINKED_INSTANCE_ASSERT(x)	do { if (!(x)) __debugbreak(); } while(0)

#include <type_traits>	// std::is_base_of, std::remove_const_t

namespace impl
{
	/// \brief	Base implementation of the iterator, non const iterators have 
	/// all their functionality just with this class.
	template <typename T>
	class LinkedInstanceIterator_impl
	{
	public:
		using value_type = T;
		using bare_value_type = std::remove_const_t<T>;
		static_assert(std::is_base_of<LinkedInstance<bare_value_type>, bare_value_type>::value, "");

	public:
		LinkedInstanceIterator_impl() = default;
		explicit LinkedInstanceIterator_impl(T * curr)
			: m_curr{ curr }
		{ }

		operator bool() const { return valid(); }

		T & operator*() const { return *checked_current(); }
		T * operator->() const { return checked_current(); }

		LinkedInstanceIterator_impl & operator++()
		{
			move_forward();
			return *this;
		}

		LinkedInstanceIterator_impl & operator++(int)
		{
			auto ret = *this;
			move_forward();
			return ret;
		}

		LinkedInstanceIterator_impl & operator--()
		{
			move_backward();
			return *this;
		}

		LinkedInstanceIterator_impl & operator--(int)
		{
			auto ret = *this;
			move_backward();
			return ret;
		}

		bool operator==(const LinkedInstanceIterator_impl & rhs) const
		{
			return m_curr == rhs.m_curr;
		}
		bool operator!=(const LinkedInstanceIterator_impl & rhs) const
		{
			return !operator==(rhs);
		}

		T * current() const { return m_curr; }
		T * checked_current() const { LINKED_INSTANCE_ASSERT(m_curr); return m_curr; }
		bool valid() const { return m_curr != nullptr; }

	private:
		void move_forward() { m_curr = linked_instance_next(**this); }
		void move_backward() { m_curr = linked_instance_prev(**this); }

		T * m_curr{ nullptr };
	};
}

template <typename T>
using LinkedInstanceIterator = impl::LinkedInstanceIterator_impl<T>;

/// \brief	Iterators that return non modifiable objects, need conversion operator from 
/// non-const iterators to const ones and comparisons.
template <typename T>
class ConstLinkedInstanceIterator : public impl::LinkedInstanceIterator_impl<const T>
{
public:
	using Base = LinkedInstanceIterator_impl<const T>;

public:
	ConstLinkedInstanceIterator() = default;
	ConstLinkedInstanceIterator(const LinkedInstanceIterator<T> & other)
		: Base{ other.current() }
	{}

	ConstLinkedInstanceIterator & operator=(const LinkedInstanceIterator<T> & other)
	{
		*this = ConstLinkedInstanceIterator{ other.current() };
		return *this;
	}

	bool operator==(const LinkedInstanceIterator<T> & rhs) const
	{
		return current() == rhs.current();
	}
	bool operator!=(const LinkedInstanceIterator<T> & rhs) const
	{
		return !operator==(rhs);
	}
};

/// \brief	Object that can be used in C++ range based for loops.
template <typename T>
class LinkedInstanceRange
{
public:
	using iterator = LinkedInstanceIterator<T>;
	using const_iterator = ConstLinkedInstanceIterator<T>;

public:
	LinkedInstanceRange(iterator  first, iterator  last)
		: m_first{ first }
		, m_last{ last }
	{}

	iterator begin() const { return m_first; }
	iterator end() const { return m_last; }

	const_iterator cbegin() const { return m_first; }
	const_iterator cend() const { return m_last; }

private:
	iterator m_first, m_last;

};

/// \brief Helper function to iterate over all instances of T.
/// Usage example:
/// 
/// for (const auto & instance : linked_instance_range<T>())
/// {
///		// ...
///	}
/// 
template <typename T>
LinkedInstanceRange<T> linked_instance_range()
{
	using iterator = LinkedInstanceIterator<T>;
	
	// nullptr to represent the end of the list ('node->m_next == nullptr' is the end)
	const iterator first{ linked_instance_head<T>() };
	const iterator last{ nullptr };

	return LinkedInstanceRange<T>{ first, last };
}
