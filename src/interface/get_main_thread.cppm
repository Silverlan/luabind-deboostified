// Copyright Daniel Wallin 2009. Use, modification and distribution is
// subject to the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

module;

#include "definitions.hpp"

export module luabind:get_main_thread;

export namespace luabind {

	LUABIND_API lua_State* get_main_thread(lua_State* L);

} // namespace luabind
