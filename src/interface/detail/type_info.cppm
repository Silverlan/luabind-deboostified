// Copyright Daniel Wallin 2008. Use, modification and distribution is
// subject to the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

module;

#include "definitions.hpp"
#include <cinttypes>
#include <typeinfo>
#include <optional>
#include <string>
#include <vector>

export module luabind:detail.type_info;

export import : detail.class_rep;

export namespace luabind {
	namespace detail
	{
		struct LUABIND_API TypeInfo
		{
			enum class FundamentalType : uint8_t
			{
				UserType = 0,
				Char,UChar,
				Short,UShort,
				Int,UInt,
				Long,ULong,
				Void,
				Bool,
				Float,
				String,
				Table,
				LuaState,
				LuabindObject,
				LuabindArgument
			};

			const std::type_info *typeInfo = nullptr;
			class_rep *crep = nullptr;
			bool isConst = false;
			bool isPointer = false;
			bool isReference = false;
			bool isSmartPtr = false;
			bool isPseudoType = false;
			bool isEnum = false;
			bool isOptional = false;
			/*bool isFinal = false;
			bool isFundamental = false;
			bool isArithmetic = false;
			bool isAbstract = false;
			bool isPolymorphic = false;
			bool isTrivial = false;*/
			FundamentalType type = FundamentalType::UserType;
			std::optional<std::string> typeIdentifier {};
			std::vector<TypeInfo> templateTypes;
		};
	};
} // namespace luabind
