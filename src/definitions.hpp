// Copyright (c) 2003 Daniel Wallin and Arvid Norberg

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

#ifndef __LUABIND_DEFINITIONS_HPP__
#define __LUABIND_DEFINITIONS_HPP__

#include "lua_include.hpp"

# define LUABIND_VERSION 900

// Each component uses two digits, so:
//
//   major = LUABIND_VERSION / 10000
//   minor = LUABIND_VERSION / 100 % 100
//   patch = LUABIND_VERSION % 100

// the maximum number of arguments of functions that's
// registered. Must at least be 2
#ifndef LUABIND_MAX_ARITY
#define LUABIND_MAX_ARITY 100
#elif LUABIND_MAX_ARITY <= 1
#undef LUABIND_MAX_ARITY
#define LUABIND_MAX_ARITY 2
#endif

// the maximum number of classes one class
// can derive from
// max bases must at least be 1
#ifndef LUABIND_MAX_BASES
#define LUABIND_MAX_BASES 100
#elif LUABIND_MAX_BASES <= 0
#undef LUABIND_MAX_BASES
#define LUABIND_MAX_BASES 1
#endif

// LUABIND_NO_ERROR_CHECKING
// define this to remove all error checks
// this will improve performance and memory
// footprint.
// if it is defined matchers will only be called on
// overloaded functions, functions that's
// not overloaded will be called directly. The
// parameters on the lua stack are assumed
// to match those of the function.
// exceptions will still be catched when there's
// no error checking.

// LUABIND_NOT_THREADSAFE
// this define will make luabind non-thread safe. That is,
// it will rely on a static variable. You can still have
// multiple lua states and use coroutines, but only
// one of your real threads may run lua code.

// LUABIND_NO_EXCEPTIONS
// this define will disable all usage of try, catch and throw in
// luabind. This will in many cases disable runtime-errors, such
// as invalid casts, when calling lua-functions that fails or
// returns values that cannot be converted by the given policy.
// Luabind requires that no function called directly or indirectly
// by luabind throws an exception (throwing exceptions through
// C code has undefined behavior, lua is written in C).

#ifdef LUABIND_DYNAMIC_LINK
# if defined (_WIN32)
#  ifdef LUABIND_BUILDING
#   define LUABIND_API __declspec(dllexport)
#  else
#   define LUABIND_API __declspec(dllimport)
#  endif
# elif defined (__CYGWIN__)
#  ifdef LUABIND_BUILDING
#   define LUABIND_API __attribute__ ((dllexport))
#  else
#   define LUABIND_API __attribute__ ((dllimport))
#  endif
# else
#  if defined(_GNUC_) && _GNUC_ >=4
#   define LUABIND_API __attribute__ ((visibility("default")))
#  endif
# endif
#endif

#ifndef LUABIND_API
# define LUABIND_API
#endif

#ifndef NDEBUG
#define LUABIND_CHECK_STACK(L) luabind::detail::stack_checker_type stack_checker_object(L)
#else
#define LUABIND_CHECK_STACK(L) do {} while (0)
#endif

// This switches between using tag arguments / structure specialization for code size tests
#define LUABIND_NO_INTERNAL_TAG_ARGUMENTS

#endif

