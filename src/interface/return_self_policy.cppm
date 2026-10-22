// Copyright (c) 2026 Silverlan

// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF
// ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
// TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
// PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT
// SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
// ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
// ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
// OR OTHER DEALINGS IN THE SOFTWARE.

module;

#include "lua_include.hpp"

export module luabind:policies.return_self_policy;

export import :detail.policy;

export namespace luabind {
	namespace detail {

		template<class T, class Policies = no_policies>
		struct return_self_converter
		{
			using base_converter = specialized_converter_policy_n<1, Policies, T, lua_to_cpp>;
			enum { consumed_args = base_converter::consumed_args };

			template<class Decorated>
			decltype(auto) to_cpp(lua_State* L, Decorated d, int index)
			{
				return m_converter.to_cpp(L, d, index);
			}

			template<class Decorated>
			int match(lua_State* L, Decorated d, int index)
			{
				return m_converter.match(L, d, index);
			}

			template<class Decorated>
			void converter_postcall(lua_State* L, Decorated d, int index)
			{
				m_converter.converter_postcall(L, d, index);
				lua_pushvalue(L, index);
			}

		private:
			base_converter m_converter;
		};

		template<class Policies = no_policies>
		struct return_self_policy
		{
			template<class T, class Direction>
			struct specialize
			{
				static_assert(std::is_same<Direction, lua_to_cpp>::value,
					"return_self can only be applied to arguments");
				using type = return_self_converter<T, Policies>;
			};
		};
	}

	using return_self = meta::type_list<converter_policy_injector<1, detail::return_self_policy<>>>;
}


