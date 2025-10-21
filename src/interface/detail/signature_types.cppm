// Copyright Daniel Wallin 2008. Use, modification and distribution is
// subject to the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef LUABIND_SIGNATURE_TYPES_HPP
# define LUABIND_SIGNATURE_TYPES_HPP


#include <vector>

export module luabind:detail.signature_types;





namespace luabind {
	namespace adl {

		class object;
		class argument;
		template <class Base>
		struct table;

	} // namespace adl

	using adl::object;
	using adl::argument;
	using adl::table;
    template <class T, class Enable = void>
    struct get_type_info;

	namespace detail {
		template <typename T>
		using base_type = typename std::remove_cv_t<std::remove_pointer_t<std::remove_reference_t<T>>>;

		template<typename Test, template<typename...> class Ref>
			struct is_specialization : std::false_type {};
		template<template<typename...> class Ref, typename... Args>
			struct is_specialization<Ref<Args...>, Ref>: std::true_type {};

		template<typename T>
			struct is_template_type : std::false_type {};
		template<template<typename...> class Ref, typename... Args>
			struct is_template_type<Ref<Args...>>: std::true_type {};

		template<typename T>
			void get_base_type_info(lua_State* L,TypeInfo &outTypeInfo)
		{
			outTypeInfo.typeInfo = &typeid(T);
			/*outTypeInfo.isFinal = std::is_final_v<T>;
			outTypeInfo.isFundamental = std::is_fundamental_v<T>;
			outTypeInfo.isArithmetic = std::is_arithmetic_v<T>;
			outTypeInfo.isAbstract = std::is_abstract_v<T>;
			outTypeInfo.isPolymorphic = std::is_polymorphic_v<T>;
			outTypeInfo.isTrivial = std::is_trivial_v<T>;*/

			class_registry* r = class_registry::get_registry(L);
			outTypeInfo.crep = r->find_class(*outTypeInfo.typeInfo);
		}


            template <class T, class Enable = void>
            struct get_type_info
            {
                static void get(lua_State* L,TypeInfo &outTypeInfo)
                {
                    get_base_type_info<T>(L,outTypeInfo);
                }
            };
		template <typename T0,typename... Ts>
			void get_variadic_types(lua_State *L,TypeInfo &outTypeInfo)
		{
			outTypeInfo.templateTypes.push_back({});
			get_type_info<T0>::get(L,outTypeInfo.templateTypes.back());

			if constexpr (sizeof...(Ts) > 0)
				get_variadic_types<Ts...>(L,outTypeInfo);
		}

		template <typename T> struct get_variadic_types_impl;
		template <template <typename...> typename C, typename... Args>
		struct get_variadic_types_impl<C<Args...>> {
			static auto get_types(lua_State *l,TypeInfo &outTypeInfo) {
				outTypeInfo.templateTypes.reserve(sizeof...(Args));
				return get_variadic_types<Args...>(l,outTypeInfo);
			}
		};



		template <class T, class Enable = void>
		struct get_user_type_info
		{
			static void get(lua_State* L,TypeInfo &outTypeInfo) {}
		};

		template <class T> requires(is_template_type<base_type<T>>::value)
			void get_variadic_type_info(lua_State* L,TypeInfo &outTypeInfo)
			{
				get_base_type_info<T>(L,outTypeInfo);
				get_variadic_types_impl<base_type<T>>::get_types(L,outTypeInfo);

				get_user_type_info<T>::get(L,outTypeInfo);
			}

		template <class T> requires(std::is_enum_v<T>)
		struct get_type_info<T>
		{
			static void get(lua_State* L,TypeInfo &outTypeInfo)
			{
				get_base_type_info<T>(L,outTypeInfo);
				outTypeInfo.isEnum = true;
			}
		};

		template <class T> requires(is_template_type<base_type<T>>::value)
		struct get_type_info<T>
		{
			static void get(lua_State* L,TypeInfo &outTypeInfo)
			{
				get_variadic_type_info<T>(L,outTypeInfo);
			}
		};

		template <class T>
		struct get_type_info<T*>
		{
			static void get(lua_State* L,TypeInfo &outTypeInfo)
			{
				get_type_info<T>::get(L,outTypeInfo);
				outTypeInfo.isPointer = true;
			}
		};

		template <class T>
		struct get_type_info<T&>
		{
			static void get(lua_State* L,TypeInfo &outTypeInfo)
			{
				get_type_info<T>::get(L,outTypeInfo);
				outTypeInfo.isReference = true;
			}
		};

		template <class T>
		struct get_type_info<T const>
		{
			static void get(lua_State* L,TypeInfo &outTypeInfo)
			{
				get_type_info<T>::get(L,outTypeInfo);
				outTypeInfo.isConst = true;
			}
		};

# define LUABIND_TYPE_TO_TYPEINFO(x,typeId) \
    template <> \
    struct get_type_info<x> \
    { \
        static void get(lua_State* L,TypeInfo &outTypeInfo) \
        { \
			outTypeInfo.type = typeId; \
        } \
    };

    //Prefer standarized sizes over builtins. This is due to long int being diffrent sizes in diffrent platforms.
    LUABIND_TYPE_TO_TYPEINFO(std::int8_t,TypeInfo::FundamentalType::Char)
    LUABIND_TYPE_TO_TYPEINFO(std::uint8_t,TypeInfo::FundamentalType::UChar)

    LUABIND_TYPE_TO_TYPEINFO(std::int16_t,TypeInfo::FundamentalType::Short)
    LUABIND_TYPE_TO_TYPEINFO(std::uint16_t,TypeInfo::FundamentalType::UShort)

    LUABIND_TYPE_TO_TYPEINFO(std::int32_t,TypeInfo::FundamentalType::Int)
    LUABIND_TYPE_TO_TYPEINFO(std::uint32_t,TypeInfo::FundamentalType::UInt)

    LUABIND_TYPE_TO_TYPEINFO(std::int64_t,TypeInfo::FundamentalType::Long)
    LUABIND_TYPE_TO_TYPEINFO(std::uint64_t,TypeInfo::FundamentalType::ULong)
	LUABIND_TYPE_TO_TYPEINFO(void,TypeInfo::FundamentalType::Void)
	LUABIND_TYPE_TO_TYPEINFO(bool,TypeInfo::FundamentalType::Bool)
	LUABIND_TYPE_TO_TYPEINFO(float,TypeInfo::FundamentalType::Float)
	LUABIND_TYPE_TO_TYPEINFO(double,TypeInfo::FundamentalType::Float)
	LUABIND_TYPE_TO_TYPEINFO(long double,TypeInfo::FundamentalType::Float)
	LUABIND_TYPE_TO_TYPEINFO(std::string,TypeInfo::FundamentalType::String)
	LUABIND_TYPE_TO_TYPEINFO(lua_State,TypeInfo::FundamentalType::LuaState)
	LUABIND_TYPE_TO_TYPEINFO(luabind::object,TypeInfo::FundamentalType::LuabindObject)
	LUABIND_TYPE_TO_TYPEINFO(luabind::argument,TypeInfo::FundamentalType::LuabindArgument)

		template <class Base>
		struct get_type_info<table<Base> >
		{
			static void get(lua_State* L,TypeInfo &outTypeInfo)
			{
				outTypeInfo.type = TypeInfo::FundamentalType::Table;
			}
		};

		inline void get_type_aux(lua_State*, bool, meta::type_list< >,std::vector<TypeInfo> &outTypes)
		{}

		template <class Signature>
		void get_type_aux(lua_State* L, bool first, Signature,std::vector<TypeInfo> &outTypes)
		{
			TypeInfo typeInfo;
			using T = typename meta::front<Signature>::type;
			if constexpr(pointer_traits<T>::is_pointer)
			{
				get_type_info<typename pointer_traits<T>::value_type>::get(L,typeInfo);
				if constexpr(std::is_pointer_v<T>)
					typeInfo.isPointer = true;
				else
					typeInfo.isSmartPtr = true;
			}
			else
				get_type_info<T>::get(L,typeInfo);
			outTypes.push_back(std::move(typeInfo));
			get_type_aux(L, false, typename meta::pop_front<Signature>::type(),outTypes);
		}

		template <class Signature>
		void get_type(lua_State* L, char const* function, Signature,std::vector<TypeInfo> &outTypes)
		{
			using first = typename meta::front<Signature>::type;

			TypeInfo typeInfo;
			if constexpr(pointer_traits<first>::is_pointer)
			{
				get_type_info<typename pointer_traits<first>::value_type>::get(L,typeInfo);
				typeInfo.isSmartPtr = true;
			}
			else
				get_type_info<first>::get(L,typeInfo);
			outTypes.push_back(std::move(typeInfo));

			get_type_aux(
				L
				, true
				, typename meta::pop_front<Signature>::type(),
				outTypes
			);
		}

	} // namespace detail

} // namespace luabind

#endif // LUABIND_FORMAT_SIGNATURE_081014_HPP

