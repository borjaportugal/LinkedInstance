
#include "src/LinkedInstance.h"
#include "src/LinkedInstanceIterator.h"

#include <iostream>

// for factory_example
#include <string>
#include <unordered_map>

/// \brief	This example simply shows how we can iterate over all instances of a type 
/// by using the LinkedInstance interface.
namespace simple_example
{
	struct Number : public LinkedInstance<Number>
	{
		Number(int ii) : i{ ii } {}
		int i;
	};

	void run()
	{
		Number a{ 0 }, b{ 1 }, c{ 2 }, d{ 3 };

		for (Number & n : linked_instance_range<Number>())
		{
			std::cout << n.i << std::endl;
			n.i *= 2;
		}

		const auto range = linked_instance_range<Number>();
		for (ConstLinkedInstanceIterator<Number> it = range.cbegin();
			 it != range.cend(); ++it)
		{
			std::cout << it->i << std::endl;
		}
	}
}

/// \brief	In this example we can see how the list changes dinamically when objects are created and destroyed.
namespace dynamic_example
{
	struct Number : public LinkedInstance<Number>
	{
		Number(int ii) : i{ ii } {}
		int i;
	};

	void run()
	{
		Number a{ 0 }, b{ 1 };

		for (const auto & n : linked_instance_range<Number>())
			std::cout << n.i << std::endl;
		std::cout << std::endl;

		{
			Number arr[3] = { 2, 3, 4 };

			for (const auto & n : linked_instance_range<Number>())
				std::cout << n.i << std::endl;
			std::cout << std::endl;
		}

		for (const auto & n : linked_instance_range<Number>())
			std::cout << n.i << std::endl;
	}
}

/// \brief	This example shows an real scenario where the LinkedInstance functionality comes handy.
namespace call_count_example
{
	struct FunctionCallCounter : public LinkedInstance<FunctionCallCounter>
	{
		explicit FunctionCallCounter(const char * fn_name)
			: m_function_name{ fn_name }
		{}

		unsigned m_call_num{ 0 };
		const char * m_function_name{ nullptr };
	};

#define TRACK_FUNCTION_CALLS()	\
	static FunctionCallCounter _call_counter_name_that_one_woudl_use{ __FUNCTION__ };	\
	_call_counter_name_that_one_woudl_use.m_call_num++

	void foo() { TRACK_FUNCTION_CALLS(); }
	void bar() { TRACK_FUNCTION_CALLS(); }

	void run()
	{
		// just make some calls to see results
		foo();
		foo();
		bar();
		foo();
		bar();

		for (unsigned i = 0; i < 10; ++i)
		{
			if (i % 2 == 0) foo();
			if (i % 3 == 0) bar();
		}

		// print our statistics
		for (const auto & call_counter : linked_instance_range<FunctionCallCounter>())
			std::cout << "Function '" << call_counter.m_function_name << "' was called #" << call_counter.m_call_num << " times.\n";
	}

#undef TRACK_FUNCTION_CALLS
}

/// \brief	Other example of a real scenario where we want to register some data for our factory to use.
///	This example shows an approach on how to register all the creators in our factory.
/// 
///	By using this method the user only needs to know about LinkedInstance and Creator.h, 
///	the factory then uses the LinkedInstance to get all creators for our types in our program.
namespace factory_example
{
	/// \brief Base class to be able to save our C++ objects to file.
	struct ISerializable
	{ 
		virtual ~ISerializable() = default;
		virtual void serialize(std::ostream & os) const = 0;
	};

	// ========= Creator.h ========= 

	/// \brief	Base class to create any instance of our system.
	struct ICreator
	{
		virtual ~ICreator() = default;
		virtual ISerializable * create() const = 0;
		virtual const char * get_name() const = 0;
	};

	template <typename T>
	struct CreatorT : public ICreator
	{
		explicit CreatorT(const char * name)
			: m_name{ name }
		{}

		T * create() const override { return new T{}; }
		const char * get_name() const { return m_name; }

	private:
		const char * m_name{ nullptr };
	};

	// here is where the LinkedInstance takes place
	struct ICreatorRegisterer : public LinkedInstance<ICreatorRegisterer>
	{
		virtual ~ICreatorRegisterer() = default;
		virtual const ICreator * create_creator() const = 0;
	};

	template <typename T>
	struct CreatorRegistererT : public ICreatorRegisterer
	{
		CreatorRegistererT(const char * name)
			: m_name{ name }
		{}

		const ICreator * create_creator() const 
		{ 
			// here we could return by allocating dynamically or by creating the creator staticaly,
			// best approach depends on the context, for this example we don't care
			static CreatorT<T> creator{ m_name };
			return &creator;
		}

	private:
		const char * m_name{ name };
	};

#define CREATOR_IMPLEMENTATION(T)	\
	static const CreatorRegistererT<T> creator_registrator ## T{ #T }

	// ========= Factory.h ========= 
	struct Factory
	{
		Factory()
		{
			// here is where we use the LinkedInstance
			for (const auto & creator_reg : linked_instance_range<ICreatorRegisterer>())
			{
				add_creator(creator_reg.create_creator());
			}
		}
		
		ISerializable * create(const std::string & type_name)
		{
			const auto it = m_creators.find(type_name);
			if (it != m_creators.end())
				return it->second->create();

			return  nullptr;
		}

	private:
		void add_creator(const ICreator * new_creator)
		{
			m_creators[new_creator->get_name()] = new_creator;
		}

		std::unordered_map<std::string, const ICreator *> m_creators;
	};

	// ========= Header files for each class in oru system that needs to be serialized ========= 
	struct SomeClass : public ISerializable
	{
		void serialize(std::ostream & os) const override
		{
			os << "Data from SomeClass.\n";
		}
	};
	struct SomeOtherClass : public ISerializable
	{
		void serialize(std::ostream & os) const override
		{
			os << "Data from SomeOtherClass.\n";
		}
	};

	// ========= Source for each class in oru system that needs to be serialized ========= 
	CREATOR_IMPLEMENTATION(SomeClass);
	CREATOR_IMPLEMENTATION(SomeOtherClass);

	// the test
	void run()
	{
		Factory factory;
		// at this point all creators are registered

		auto * some_class = factory.create("SomeClass");
		auto * some_other_class = factory.create("SomeOtherClass");

		some_class->serialize(std::cout);
		some_other_class->serialize(std::cout);

		delete some_other_class;
		delete some_class;
	}

#undef CREATOR_IMPLEMENTATION
}

int main()
{
	std::cout << "simple_example:\n";
	simple_example::run();

	std::cout << "\n\n\ndynamic_example:\n";
	dynamic_example::run();
	
	std::cout << "\n\n\ncall_count_example:\n";
	call_count_example::run();
	
	std::cout << "\n\n\nfactory_example:\n";
	factory_example::run();

	std::cin.get();
	return 0;
}
