#ifndef __THREAD_H
#define __THREAD_H



#include<abt.h>
#include<functional>
#include<cstdlib>
#include<iostream>
#include<algorithm>
#include<tuple>
#include"thread_Singleton.h"


namespace stdx 
{
	template<class Ret, class ...Args>
	struct xthread_d_wrapper_args_t 
	{
		std::function<Ret(Args...)> func_;
		std::tuple<Args...> tuple_;
		ABT_eventual* ev_ptr_;
		// Ret ret_;
	};

	template<class Ret, class ...Args> 
	void xthread_d_wrapper (void * ptr) 
	{
		stdx::xthread_d_wrapper_args_t<Ret, Args...>* xwa_ptr = (stdx::xthread_d_wrapper_args_t<Ret, Args...>*) ptr;
		std::apply(xwa_ptr->func_, xwa_ptr->tuple_);
		ABT_eventual_set((*xwa_ptr->ev_ptr_), nullptr, 0);
	}

	class thread_d 
	{
		public:
			thread_Singleton* psingleton;

			/* default constructor */
			thread_d () noexcept {};

			/* constructor with parameters */
			template<class Fn, class ...Args>
			thread_d (Fn func_in, Args ...args)
			{

				int rank;
				int flag;

				psingleton = thread_Singleton::instance();

				typedef typename std::result_of<decltype(func_in)&(Args...)>::type mytype_; 
				xthread_d_wrapper_args_t<mytype_, Args...> * xwargs_ptr;
				xwargs_ptr = new xthread_d_wrapper_args_t<mytype_, Args...>;
				xwargs_ptr->func_ = func_in;
				xwargs_ptr->tuple_ = std::make_tuple(args...);
				ABT_eventual_create(0, &eventual);
				xwargs_ptr->ev_ptr_ = &eventual;
				ptr_= xwargs_ptr;


				/* Initializing pools, schedulors and ESs in singleton class */
				/* And offer a handler to reach the resources for this ULT */
				ABT_xstream_self_rank(&rank);
				ABT_pool target_pool = psingleton->pools[rank]; 
				// xthread_wrapper<mytype_, Args...> (&())
				flag = ABT_thread_create(target_pool, xthread_d_wrapper<mytype_, Args...>, xwargs_ptr,
						ABT_THREAD_ATTR_NULL, nullptr);

			}

			~thread_d () 
			{
				free(ptr_);
				ABT_eventual_free(&eventual);
			}

			void wait();

		private:
			void * ptr_;		
			ABT_eventual eventual;
	};

	template<class Ret, class ...Args>
	struct xthread_wrapper_args_t 
	{
		std::function<Ret(Args...)> func_;
		std::tuple<Args...> tuple_;
		// Ret ret_;
	};

	template<class Ret, class ...Args> 
	void xthread_wrapper (void * ptr) 
	{
		stdx::xthread_wrapper_args_t<Ret, Args...>* xwa_ptr = (stdx::xthread_wrapper_args_t<Ret, Args...>*) ptr;
		std::apply(xwa_ptr->func_, xwa_ptr->tuple_);
	}

	class thread
	{
		public:
			class id 
			{
				ABT_thread ult;
				public:
				id () noexcept : ult (){}
				id (ABT_thread other): ult(other) {}

				private:
					friend class thread;
					friend bool operator== (thread::id id1, thread::id id2) noexcept;
					friend bool operator< (thread::id id1, thread::id id2) noexcept;

					friend 
					ostream&
					operator<< (ostream& __out, thread::id id1);
			};

		public:
			thread_Singleton* psingleton;

			/* default constructor */
			thread () noexcept; 

			/* constructor with parameters */
			template<class Fn, class ...Args>
			thread (Fn func_in, Args ...args)
			{
				int rank;
				int flag;
			
				psingleton = thread_Singleton::instance();

				typedef typename std::result_of<decltype(func_in)&(Args...)>::type mytype_; 
				xthread_wrapper_args_t<mytype_, Args...> * xwargs_ptr;
				xwargs_ptr = new xthread_wrapper_args_t<mytype_, Args...>;
				xwargs_ptr->func_ = func_in;
				xwargs_ptr->tuple_ = std::make_tuple(args...);
				xwargs_ptr_ = xwargs_ptr;

				/* Initializing pools, schedulors and ESs in singleton class */
				/* And offer a handler to reach the resources for this ULT */
				ABT_xstream_self_rank(&rank);
				ABT_pool target_pool = psingleton->pools[rank]; 
				flag = ABT_thread_create(target_pool, xthread_wrapper<mytype_, Args...>, xwargs_ptr,
						ABT_THREAD_ATTR_NULL, &__id.ult);
			}

			thread(thread&& other);
			thread(thread&) = delete;
			thread(const thread&) = delete;
			thread(const thread&&) = delete;

			~thread() 
			{
				free(xwargs_ptr_); 
				xwargs_ptr_ = NULL;
			}

			void join();
			void detach();
			bool joinable();
			id get_id() const noexcept;
			void swap(thread & other);

			thread& operator=(thread&& other);
			thread& operator=(const thread& other)=delete;

		private:
			void* xwargs_ptr_;
			id __id;
	};

	ostream& operator<<(ostream& __out, thread::id id2);
	bool operator==(thread::id id1, thread::id id2) noexcept;
	bool operator<(thread::id id1, thread::id id2) noexcept;
	bool operator>(thread::id id1, thread::id id2) noexcept;
	bool operator!=(thread::id id1, thread::id id2) noexcept;
	bool operator>=(thread::id id1, thread::id id2) noexcept;
	bool operator<=(thread::id id1, thread::id id2) noexcept;
};

#endif


