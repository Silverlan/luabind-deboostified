// Copyright Daniel Wallin 2008. Use, modification and distribution is
// subject to the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

module;

# include <typeinfo>
#include <cstring>

export module luabind:typeid_type;

export import :detail.type_traits;

export namespace luabind {

	class type_id
	{
	public:
		type_id()
			: id(&typeid(null_type))
		{}

		type_id(std::type_info const& id)
			: id(&id)
		{}

		bool operator!=(type_id const& other) const
		{
			return *id != *other.id;
		}

		bool operator==(type_id const& other) const
		{
			return *id == *other.id;
		}

		bool operator<(type_id const& other) const
		{
			return id->before(*other.id);
		}

		size_t hash_code() const
		{
			return id->hash_code();
		}

		char const* name() const
		{
			return id->name();
		}

	private:
		std::type_info const* id;
	};

} // namespace luabind
