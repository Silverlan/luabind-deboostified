module;

#include "lua_include.hpp"
#include <cassert>

export module luabind:lua_proxy_interface;

import :detail.decorate_type;
import :detail.push_to_lua;
import :detail.stack_utils;
export import :detail.type_traits;
import :error;
import :lua_proxy;
export import :typeid_type;

// object.cppm
export import :from_stack;

#if LUA_VERSION_NUM < 502
# define lua_compare(L, index1, index2, fn) fn(L, index1, index2)
# define LUA_OPEQ lua_equal
# define LUA_OPLT lua_lessthan
# define lua_rawlen lua_objlen
# define lua_pushglobaltable(L) lua_pushvalue(L, LUA_GLOBALSINDEX)
#endif

export namespace luabind {

	namespace adl {
		class object;

		template <class T>
		class lua_proxy_interface;

		namespace check_object_interface
		{
			template <class T>
			std::true_type  check(lua_proxy_interface<T>*);
			std::false_type check(void*);
		} // namespace is_object_interface_aux

		template <class T>
		struct is_object_interface : public decltype(check_object_interface::check((remove_const_reference_t<T>*)nullptr))
		{};

		template <class R, class T, class U>
		struct enable_binary
			: std::enable_if< is_object_interface<T>::value || is_object_interface<U>::value, R >
		{};

		template<class T, class U>
		int binary_interpreter(lua_State*& L, T const& lhs, U const& rhs, std::true_type, std::true_type)
		{
			L = lua_proxy_traits<T>::interpreter(lhs);
			lua_State* L2 = lua_proxy_traits<U>::interpreter(rhs);

			// you are comparing objects with different interpreters
			// that's not allowed.
			assert(L == L2 || L == 0 || L2 == 0);

			// if the two objects we compare have different interpreters
			// then they

			if(L != L2) return -1;
			if(L == 0) return 1;
			return 0;
		}

		template<class T, class U>
		int binary_interpreter(lua_State*& L, T const& x, U const&, std::true_type, std::false_type)
		{
			L = lua_proxy_traits<T>::interpreter(x);
			return 0;
		}

		template<class T, class U>
		int binary_interpreter(lua_State*& L, T const&, U const& x, std::false_type, std::true_type)
		{
			L = lua_proxy_traits<U>::interpreter(x);
			return 0;
		}

		template<class T, class U>
		int binary_interpreter(lua_State*& L, T const& x, U const& y)
		{
			return binary_interpreter(L, x, y, is_lua_proxy_type<T>(), is_lua_proxy_type<U>());
		}

		template<class LHS, class RHS>
		typename enable_binary<bool, LHS, RHS>::type
			operator==(LHS&& lhs, RHS&& rhs)
		{
			lua_State* L = 0;
			switch(binary_interpreter(L, lhs, rhs)) {
			case  1: return true;
			case-1: return false;
			}
			assert(L);
			detail::stack_pop pop1(L, 1);
			detail::push_to_lua(L, std::forward<LHS>(lhs));
			detail::stack_pop pop2(L, 1);
			detail::push_to_lua(L, std::forward<RHS>(rhs));
			return lua_compare(L, -1, -2, LUA_OPEQ) != 0;
		}

		template<class LHS, class RHS>
		typename enable_binary<bool, LHS, RHS>::type
			operator<(LHS&& lhs, RHS&& rhs)
		{
			lua_State* L = 0;
			switch(binary_interpreter(L, lhs, rhs)) {
			case  1: return true;
			case-1: return false;
			}
			assert(L);
			detail::stack_pop pop1(L, 1);
			detail::push_to_lua(L, std::forward<LHS>(lhs));
			detail::stack_pop pop2(L, 1);
			detail::push_to_lua(L, std::forward<RHS>(rhs));
			return lua_compare(L, -1, -2, LUA_OPLT) != 0;
		}

		template<class ValueWrapper>
		std::ostream& operator<<(std::ostream& os, lua_proxy_interface<ValueWrapper> const& v)
		{
			using namespace luabind;
			lua_State* interpreter = lua_proxy_traits<ValueWrapper>::interpreter(
				static_cast<ValueWrapper const&>(v));
			detail::stack_pop pop(interpreter, 1);
			lua_proxy_traits<ValueWrapper>::unwrap(interpreter, static_cast<ValueWrapper const&>(v));
			char const* p = lua_tostring(interpreter, -1);
			std::size_t len = lua_rawlen(interpreter, -1);
			os.write(p, len);
			//std::copy(p, p+len, std::ostream_iterator<char>(os));
			return os;
		}


		template<class LHS, class RHS>
		typename enable_binary<bool, LHS, RHS>::type
			operator>(LHS const& lhs, RHS const& rhs)
		{
			return !(lhs < rhs || lhs == rhs);
		}

		template<class LHS, class RHS>
		typename enable_binary<bool, LHS, RHS>::type
			operator<=(LHS const& lhs, RHS const& rhs)
		{
			return lhs < rhs || lhs == rhs;
		}

		template<class LHS, class RHS>
		typename enable_binary<bool, LHS, RHS>::type
			operator>=(LHS const& lhs, RHS const& rhs)
		{
			return !(lhs < rhs);
		}

		template<class LHS, class RHS>
		typename enable_binary<bool, LHS, RHS>::type
			operator!=(LHS const& lhs, RHS const& rhs)
		{
			return !(lhs == rhs);
		}

		template<class Derived>
		class lua_proxy_interface
		{
		public:
			~lua_proxy_interface() {}

			// defined in luabind/detail/object.hpp
			template<typename... Args>
			object operator()(Args&&... args);

			// defined in luabind/detail/object.hpp
			template<typename PolicyList, typename... Args>
			object call(Args&&... args);

			explicit operator bool() const
			{
				lua_State* L = lua_proxy_traits<Derived>::interpreter(derived());
				if(!L) return 0;
				lua_proxy_traits<Derived>::unwrap(L, derived());
				detail::stack_pop pop(L, 1);
				return lua_toboolean(L, -1) == 1;
			}

		private:
			Derived& derived() { return *static_cast<Derived*>(this); }
			Derived const& derived() const { return *static_cast<Derived const*>(this); }
		};

	}

	template<class ValueWrapper>
	std::string to_string(adl::lua_proxy_interface<ValueWrapper> const& v)
	{
		using namespace luabind;
		lua_State* interpreter = lua_proxy_traits<ValueWrapper>::interpreter(static_cast<ValueWrapper const&>(v));
		detail::stack_pop pop(interpreter, 1);
		lua_proxy_traits<ValueWrapper>::unwrap(interpreter, static_cast<ValueWrapper const&>(v));
		char const* p = lua_tostring(interpreter, -1);
		std::size_t len = lua_rawlen(interpreter, -1);
		return std::string(p, len);
	}

	namespace detail
	{
		template<class T, class ValueWrapper, class Policies, class ErrorPolicy, class ReturnType >
		ReturnType object_cast_aux(ValueWrapper const& value_wrapper, T*, Policies*, ErrorPolicy error_policy, ReturnType*)
		{
			lua_State* interpreter = lua_proxy_traits<ValueWrapper>::interpreter(value_wrapper);

#ifndef LUABIND_NO_ERROR_CHECKING
			if(!interpreter)
				return error_policy.handle_error(interpreter, typeid(void));
#endif
			lua_proxy_traits<ValueWrapper>::unwrap(interpreter, value_wrapper);
			detail::stack_pop pop(interpreter, 1);
			specialized_converter_policy_n<0, Policies, T, lua_to_cpp> cv;

			if(cv.match(interpreter, decorate_type_t<T>(), -1) < 0) {
				return error_policy.handle_error(interpreter, typeid(T));
			}
			return cv.to_cpp(interpreter, decorate_type_t<T>(), -1);
		}

		template<class T>
		struct throw_error_policy
		{
			T handle_error(lua_State* interpreter, type_id const& type_info)
			{
#ifndef LUABIND_NO_EXCEPTIONS
				throw cast_failed(interpreter, type_info);
#else
				cast_failed_callback_fun e = get_cast_failed_callback();
				if(e) e(interpreter, type_info);

				assert(0 && "object_cast failed. If you want to handle this error use "
					"luabind::set_error_callback()");
				std::terminate();
#endif
				//return *(typename std::remove_reference<T>::type*)0; //DEAD CODE!
			}
		};

		template<class T>
		struct nothrow_error_policy
		{
			nothrow_error_policy(T rhs) : value(rhs) {}

			T handle_error(lua_State*, type_id const&)
			{
				return value;
			}
		private:
			T value;
		};
	} // namespace detail

	template<class T, class ValueWrapper> inline
		T object_cast(ValueWrapper const& value_wrapper)
	{
		return detail::object_cast_aux(value_wrapper, (T*)0, (no_policies*)0, detail::throw_error_policy<T>(), (T*)0);
	}

	template<class T, class ValueWrapper, class Policies> inline
		T object_cast(ValueWrapper const& value_wrapper, Policies const&)
	{
		return detail::object_cast_aux(value_wrapper, (T*)0, (Policies*)0, detail::throw_error_policy<T>(), (T*)0);
	}

	template<typename T, typename ValueWrapper, typename ReturnValue> inline
		ReturnValue object_cast_nothrow(ValueWrapper const& value_wrapper, ReturnValue default_value)
	{
		return detail::object_cast_aux(value_wrapper, (T*)0, (no_policies*)0, detail::nothrow_error_policy<ReturnValue>(default_value), (ReturnValue*)0);
	}

	template<typename T, typename ValueWrapper, typename Policies, typename ReturnValue> inline
		ReturnValue object_cast_nothrow(ValueWrapper const& value_wrapper, Policies const&, ReturnValue default_value)
	{
		return detail::object_cast_aux(value_wrapper, (T*)0, (Policies*)0, detail::nothrow_error_policy<ReturnValue>(default_value), (ReturnValue*)0);
	}

	template <class ValueWrapper>
	inline lua_CFunction tocfunction(ValueWrapper const& value)
	{
		lua_State* interpreter = lua_proxy_traits<ValueWrapper>::interpreter(value);
		lua_proxy_traits<ValueWrapper>::unwrap(interpreter, value);
		detail::stack_pop pop(interpreter, 1);
		return lua_tocfunction(interpreter, -1);
	}

	template <class T, class ValueWrapper>
	inline T* touserdata(ValueWrapper const& value)
	{
		lua_State* interpreter = lua_proxy_traits<ValueWrapper>::interpreter(value);

		lua_proxy_traits<ValueWrapper>::unwrap(interpreter, value);
		detail::stack_pop pop(interpreter, 1);
		return static_cast<T*>(lua_touserdata(interpreter, -1));
	}
}

#if LUA_VERSION_NUM < 502
# undef lua_compare
# undef LUA_OPEQ
# undef LUA_OPLT
# undef lua_rawlen
# undef lua_pushglobaltable
#endif

// object.cppm

#if LUA_VERSION_NUM < 502
# define lua_pushglobaltable(L) lua_pushvalue(L, LUA_GLOBALSINDEX)
#endif

export namespace luabind {
	namespace adl {
		template<class Next>
		class index_proxy;

		// An object holds a reference to a Lua value residing
		// in the registry.
		class object :
			public lua_proxy_interface<object>
		{
		public:
			object()
			{}

			explicit object(handle const& other)
				: m_handle(other)
			{}

			explicit object(from_stack const& stack_reference)
				: m_handle(stack_reference.interpreter, stack_reference.index)
			{
			}

			template<class T>
			object(lua_State* interpreter, T&& value)
			{
				detail::push_to_lua(interpreter, std::forward<T>(value));
				detail::stack_pop pop(interpreter, 1);
				handle(interpreter, -1).swap(m_handle);
			}

			template<class T, class Policies>
			object(lua_State* interpreter, T&& value, Policies const&)
			{
				detail::push_to_lua<1, Policies>(interpreter, std::forward<T>(value));
				detail::stack_pop pop(interpreter, 1);
				handle(interpreter, -1).swap(m_handle);
			}

			void push(lua_State* interpreter) const;
			lua_State* interpreter() const;
			bool is_valid() const;

			template<class T>
			index_proxy<object> operator[](T const& key) const;

			void swap(object& other)
			{
				m_handle.swap(other.m_handle);
			}

		private:
			handle m_handle;
		};

		inline void object::push(lua_State* interpreter) const
		{
			m_handle.push(interpreter);
		}

		inline lua_State* object::interpreter() const
		{
			return m_handle.interpreter();
		}

		inline bool object::is_valid() const
		{
			return m_handle.interpreter() != 0;
		}

	} // namespace adl

	using adl::object;

	template<>
	struct lua_proxy_traits<object>
	{
		using is_specialized = std::true_type;

		static lua_State* interpreter(object const& value)
		{
			return value.interpreter();
		}

		static void unwrap(lua_State* interpreter, object const& value)
		{
			value.push(interpreter);
		}

		static bool check(...)
		{
			return true;
		}
	};

	template<class R, typename PolicyList = no_policies, typename... Args>
	R call_function(luabind::object const& obj, Args&&... args)
	{
		obj.push(obj.interpreter());
		return call_pushed_function<R, PolicyList>(obj.interpreter(), std::forward<Args>(args)...);
	}

	template<class R, typename PolicyList = no_policies, typename... Args>
	R resume_function(luabind::object const& obj, Args&&... args)
	{
		obj.push(obj.interpreter());
		return resume_pushed_function<R, PolicyList>(obj.interpreter(), std::forward<Args>(args)...);
	}

	namespace adl {
		// Simple value_wrapper adaptor with the sole purpose of helping with
		// overload resolution. Use this as a function parameter type instead
		// of "object" or "argument" to restrict the parameter to Lua tables.
		template <class Base = object>
		struct table : Base
		{
			table(from_stack const& stack_reference)
				: Base(stack_reference)
			{}
		};

	} // namespace adl

	using adl::table;

	template <class Base>
	struct lua_proxy_traits<adl::table<Base> >
		: lua_proxy_traits<Base>
	{
		static bool check(lua_State* L, int idx)
		{
			return lua_proxy_traits<Base>::check(L, idx) &&
				lua_istable(L, idx);
		}
	};

	inline object newtable(lua_State* interpreter)
	{
		lua_newtable(interpreter);
		detail::stack_pop pop(interpreter, 1);
		return object(from_stack(interpreter, -1));
	}

	// this could be optimized by returning a proxy
	inline object globals(lua_State* interpreter)
	{
		lua_pushglobaltable(interpreter);
		detail::stack_pop pop(interpreter, 1);
		return object(from_stack(interpreter, -1));
	}

	// this could be optimized by returning a proxy
	inline object registry(lua_State* interpreter)
	{
		lua_pushvalue(interpreter, LUA_REGISTRYINDEX);
		detail::stack_pop pop(interpreter, 1);
		return object(from_stack(interpreter, -1));
	}

	template<class ValueWrapper, class K>
	inline object gettable(ValueWrapper const& table, K&& key)
	{
		lua_State* interpreter = lua_proxy_traits<ValueWrapper>::interpreter(table);

		lua_proxy_traits<ValueWrapper>::unwrap(interpreter, table);
		detail::stack_pop pop(interpreter, 2);
		detail::push_to_lua(interpreter, std::forward<K>(key));
		lua_gettable(interpreter, -2);
		return object(from_stack(interpreter, -1));
	}

	template<class ValueWrapper, class K, class T>
	inline void settable(ValueWrapper const& table, K&& key, T&& value)
	{
		lua_State* interpreter = lua_proxy_traits<ValueWrapper>::interpreter(table);

		// TODO: Exception safe?

		lua_proxy_traits<ValueWrapper>::unwrap(interpreter, table);
		detail::stack_pop pop(interpreter, 1);
		detail::push_to_lua(interpreter, std::forward<K>(key));
		detail::push_to_lua(interpreter, std::forward<T>(value));
		lua_settable(interpreter, -3);
	}

	template<class ValueWrapper, class K>
	inline object rawget(ValueWrapper const& table, K&& key)
	{
		lua_State* interpreter = lua_proxy_traits<ValueWrapper>::interpreter(
			table
		);

		lua_proxy_traits<ValueWrapper>::unwrap(interpreter, table);
		detail::stack_pop pop(interpreter, 2);
		detail::push_to_lua(interpreter, std::forward<K>(key));
		lua_rawget(interpreter, -2);
		return object(from_stack(interpreter, -1));
	}

	template<class ValueWrapper, class K, class T>
	inline void rawset(ValueWrapper const& table, K&& key, T&& value)
	{
		lua_State* interpreter = lua_proxy_traits<ValueWrapper>::interpreter(
			table
		);

		// TODO: Exception safe?

		lua_proxy_traits<ValueWrapper>::unwrap(interpreter, table);
		detail::stack_pop pop(interpreter, 1);
		detail::push_to_lua(interpreter, std::forward<K>(key));
		detail::push_to_lua(interpreter, std::forward<T>(value));
		lua_rawset(interpreter, -3);
	}

	template<class ValueWrapper>
	inline int type(ValueWrapper const& value)
	{
		lua_State* interpreter = lua_proxy_traits<ValueWrapper>::interpreter(value);

		lua_proxy_traits<ValueWrapper>::unwrap(interpreter, value);
		detail::stack_pop pop(interpreter, 1);
		return lua_type(interpreter, -1);
	}

	template <class ValueWrapper>
	inline object getmetatable(ValueWrapper const& obj)
	{
		lua_State* interpreter = lua_proxy_traits<ValueWrapper>::interpreter(obj);
		lua_proxy_traits<ValueWrapper>::unwrap(interpreter, obj);
		detail::stack_pop pop(interpreter, 2);
		lua_getmetatable(interpreter, -1);
		return object(from_stack(interpreter, -1));
	}

	template <class ValueWrapper1, class ValueWrapper2>
	inline void setmetatable(ValueWrapper1 const& obj, ValueWrapper2 const& metatable)
	{
		lua_State* interpreter = lua_proxy_traits<ValueWrapper1>::interpreter(obj);
		lua_proxy_traits<ValueWrapper1>::unwrap(interpreter, obj);
		detail::stack_pop pop(interpreter, 1);
		lua_proxy_traits<ValueWrapper2>::unwrap(interpreter, metatable);
		lua_setmetatable(interpreter, -2);
	}

	template <class ValueWrapper>
	inline std::tuple<const char*, object> getupvalue(ValueWrapper const& value, int index)
	{
		lua_State* interpreter = lua_proxy_traits<ValueWrapper>::interpreter(value);
		lua_proxy_traits<ValueWrapper>::unwrap(interpreter, value);
		detail::stack_pop pop(interpreter, 2);
		const char* name = lua_getupvalue(interpreter, -1, index);
		return std::make_tuple(name, object(from_stack(interpreter, -1)));
	}

	template <class ValueWrapper1, class ValueWrapper2>
	inline void setupvalue(ValueWrapper1 const& function, int index, ValueWrapper2 const& value)
	{
		lua_State* interpreter = lua_proxy_traits<ValueWrapper1>::interpreter(function);

		lua_proxy_traits<ValueWrapper1>::unwrap(interpreter, function);
		detail::stack_pop pop(interpreter, 1);
		lua_proxy_traits<ValueWrapper2>::unwrap(interpreter, value);
		lua_setupvalue(interpreter, -2, index);
	}

	template <class GetValueWrapper>
	object property(GetValueWrapper const& get)
	{
		lua_State* interpreter = lua_proxy_traits<GetValueWrapper>::interpreter(get);
		lua_proxy_traits<GetValueWrapper>::unwrap(interpreter, get);
		lua_pushnil(interpreter);
		lua_pushcclosure(interpreter, &detail::property_tag, 2);
		detail::stack_pop pop(interpreter, 1);
		return object(from_stack(interpreter, -1));
	}

	template <class GetValueWrapper, class SetValueWrapper>
	object property(GetValueWrapper const& get, SetValueWrapper const& set)
	{
		lua_State* interpreter = lua_proxy_traits<GetValueWrapper>::interpreter(get);
		lua_proxy_traits<GetValueWrapper>::unwrap(interpreter, get);
		lua_proxy_traits<SetValueWrapper>::unwrap(interpreter, set);
		lua_pushcclosure(interpreter, &detail::property_tag, 2);
		detail::stack_pop pop(interpreter, 1);
		return object(from_stack(interpreter, -1));
	}

#ifndef __clang__
    // For clang definition, see lua_index_proxy.cppm
	template<class T>
	adl::index_proxy<adl::object> adl::object::operator[](T const& key) const
	{
		return index_proxy<object>(
			*this, m_handle.interpreter(), key
			);
	}
#endif

	template<typename ProxyType>
	template<typename PolicyList, typename... Args>
	object adl::lua_proxy_interface<ProxyType>::call(Args&&... args)
	{
		return call_function<object, PolicyList>(derived(), std::forward<Args>(args)...);
	}

	template<typename ProxyType>
	template<typename... Args>
	object adl::lua_proxy_interface<ProxyType>::operator()(Args&&... args)
	{
		return call<no_policies>(std::forward<Args>(args)...);
	}

} // namespace luabind

#if LUA_VERSION_NUM < 502
#undef lua_pushglobaltable
#endif
