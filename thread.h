#ifndef __THREAD_H
#define __THREAD_H


#include<abt.h>
#include<functional>
#include<cstdlib>
#include<iostream>
#include<algorithm>
#include<memory>
#include<tuple>
#include"thread_Singleton.h"

#define TEST 

namespace stdx 
{
	#ifdef TEST 
	template<class Ret, class ...Args>
	struct xthread_d_wrapper_args_t 
	{
		std::function<Ret(Args...)> func_;
		std::tuple<Args...> tuple_;
		shared_ptr<ABT_eventual> ev_ptr_;
	};

	template<class Ret, class ...Args> 
	void xthread_d_wrapper (void * ptr) 
	{
		stdx::xthread_d_wrapper_args_t<Ret, Args...>* xwa_ptr = (stdx::xthread_d_wrapper_args_t<Ret, Args...>*) ptr;
		std::apply(xwa_ptr->func_, xwa_ptr->tuple_);
		ABT_eventual_set((*xwa_ptr->ev_ptr_), nullptr, 0);
	}

	class thread 
	{
		public:
			class id 
			{
				shared_ptr<ABT_eventual> eventual_ptr_;
				public:
				id () noexcept 
				{
					// eventual_ptr_ = nullptr;
				}
				~id(){}

				private:
				friend class thread;
				friend bool operator==(thread::id id1, thread::id id2) noexcept;
				friend bool operator<(thread::id id1, thread::id id2) noexcept;
				friend bool operator!=(thread::id id1, thread::id id2) noexcept;
				friend bool operator>(thread::id id1, thread::id id2) noexcept;
				friend bool operator<=(thread::id id1, thread::id id2) noexcept;
				friend bool operator>=(thread::id id1, thread::id id2) noexcept;
				friend 
				ostream&
				operator<< (ostream& __out, thread::id id1);
			};
		public:
			thread_Singleton* psingleton;

			/* default constructor */
			thread() noexcept {};

			/* constructor with parameters */
			template<class Fn, class ...Args>
			thread (Fn func_in, Args ...args)
			{
				int rank;
				int flag;

				cout <<"Argobots thread_d" << endl;
				psingleton = thread_Singleton::instance();

				typedef typename std::invoke_result<decltype(func_in), Args...>::type mytype_; 
				xthread_d_wrapper_args_t<mytype_, Args...> * xdwargs_ptr;
				xdwargs_ptr = new xthread_d_wrapper_args_t<mytype_, Args...>;
				id_.eventual_ptr_ = make_shared<ABT_eventual>();
				xdwargs_ptr->func_ = func_in;
				xdwargs_ptr->tuple_ = std::make_tuple(args...);
				ABT_eventual_create(0, &(*id_.eventual_ptr_));
				xdwargs_ptr->ev_ptr_ = id_.eventual_ptr_;
				xdwargs_ptr_ = xdwargs_ptr;


				/* Initializing pools, schedulors and ESs in singleton class */
				/* And offer a handler to reach the resources for this ULT */
				ABT_xstream_self_rank(&rank);
				ABT_pool target_pool = psingleton->pools[rank]; 
				flag = ABT_thread_create(target_pool, xthread_d_wrapper<mytype_, Args...>, xdwargs_ptr,
						ABT_THREAD_ATTR_NULL, nullptr);

			}

			~thread () 
			{
				free(xdwargs_ptr_); 
				xdwargs_ptr_ = NULL;
			}

			void join()
			{
				if(id_ != id())
				{
					ABT_eventual_wait((*id_.eventual_ptr_), nullptr);
					ABT_eventual_free(&(*id_.eventual_ptr_));
				}
				id_ = id();
			}
			void detach() 
			{
				id_.eventual_ptr_ = nullptr;
			}
			bool joinable()
			{
				return !(id_ == id());
			}
			id get_id() const noexcept
			{
				return this->id_;
			}
			void swap(thread& other)
			{
				std::swap (this->id_, other.id_);
			} 
			thread& operator=(thread&& other)
			{
				if (this->joinable())
				{
					std::terminate();
				}
				this->swap(other);
				this->xdwargs_ptr_ = nullptr;
				std::swap(this->xdwargs_ptr_, other.xdwargs_ptr_);	
				return *this;
			}
			thread& operator=(const thread& other)=delete;

		private:
			void * xdwargs_ptr_;		
			id id_;
	};
	#endif


	#ifndef TEST 
	template<class Ret, class ...Args>
	struct xthread_wrapper_args_t 
	{
		std::function<Ret(Args...)> func_;
		std::tuple<Args...> tuple_;
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
					friend bool operator==(thread::id id1, thread::id id2) noexcept;
					friend bool operator<(thread::id id1, thread::id id2) noexcept;
					friend bool operator!=(thread::id id1, thread::id id2) noexcept;
					friend bool operator>(thread::id id1, thread::id id2) noexcept;
					friend bool operator<=(thread::id id1, thread::id id2) noexcept;
					friend bool operator>=(thread::id id1, thread::id id2) noexcept;
					friend 
					ostream&
					operator<< (ostream& __out, thread::id id1);
			};

		public:
			thread_Singleton* psingleton;

			/* default constructor */
			thread () noexcept 
			{
				xwargs_ptr_ = nullptr;
			}

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

			thread(thread&& other) 
			{
				swap (other);
				this->xwargs_ptr_ = nullptr;
				std::swap(this->xwargs_ptr_, other.xwargs_ptr_);	
			}
			thread(thread&) = delete;
			thread(const thread&) = delete;
			thread(const thread&&) = delete;

			~thread() 
			{
				free(xwargs_ptr_); 
				xwargs_ptr_ = NULL;
			}

			void join() 
			{
				if(this->joinable())
				{
					ABT_thread_free (&__id.ult);
				}
				__id = id();
			}
			void detach();
			bool joinable() 
			{
				// return !(__id == id());
				return __id < id();
			}
			id get_id() const noexcept 
			{
				return this->__id;
			}
			void swap(thread & other) 
			{
				std::swap (this->__id, other.__id);
			}
			thread& operator=(thread&& other) 
			{
				if (this->joinable())
				{
					std::terminate();
				}
				this->swap(other);
				this->xwargs_ptr_ = nullptr;
				std::swap(this->xwargs_ptr_, other.xwargs_ptr_);	
				return *this;
			}
			thread& operator=(const thread& other)=delete;

		private:
			void* xwargs_ptr_;
			id __id;
			friend class ostream;
	};
	#endif
	
	inline ostream& operator<<(ostream& __out, thread::id id1) 
	{
		if (id1 == stdx::thread::id())
			return __out << "thread::id of a non-executing thread";
		else
			#ifdef TEST
			return __out << id1.eventual_ptr_;
			#endif
			#ifndef TEST
			return __out << id1.ult;
			#endif
	}
	inline bool operator==(stdx::thread::id id1, stdx::thread::id id2) noexcept 
	{
		#ifdef TEST
		return id1.eventual_ptr_ == id2.eventual_ptr_;	
		#endif
		#ifndef TEST
		return id1.ult == id2.ult;	
		#endif
	}
	inline bool operator<(thread::id id1, thread::id id2) noexcept 
	{ 
		#ifdef TEST
		return id1.eventual_ptr_ < id2.eventual_ptr_;
		#endif
		#ifndef TEST
		return id1.ult < id2.ult;
		#endif
	}

	inline bool operator>(thread::id id1, thread::id id2) noexcept
	{ return id2 < id1;}
	inline bool operator!=(thread::id id1, thread::id id2) noexcept
	{ return !(id1 == id2);}
	inline bool operator>=(thread::id id1, thread::id id2) noexcept
	{ return !(id1 < id2);}
	inline bool operator<=(thread::id id1, thread::id id2) noexcept
	{ return !(id2 < id1);}
};
#endif

