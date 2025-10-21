cmake_minimum_required(VERSION 3.20)
project(luabind LANGUAGES CXX)

# Build for LuaBind (module-aware / C++20 version)
# Updated to consume C++20 module interface units (.cppm) instead of plain headers
# and to build with C++20 (module support).
#
# Original author: Ryan Pavlik <rpavlik@iastate.edu>
# This file adapted to use C++20 modules & module interface units.

# Require C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Sources (implementation .cpp files stay as-is)
set(LUABIND_SRCS
    class.cpp
    class_info.cpp
    class_registry.cpp
    class_rep.cpp
    create_class.cpp
    error.cpp
    exception_handler.cpp
    function.cpp
    function_introspection.cpp
    inheritance.cpp
    link_compatibility.cpp
    object_rep.cpp
    open.cpp
    operator.cpp
    pcall.cpp
    scope.cpp
    set_package_preload.cpp
    stack_content_by_name.cpp
    weak_ref.cpp
    wrapper_base.cpp
)
source_group("Sources" FILES ${LUABIND_SRCS})

# Module interface units (converted from the old headers to .cppm)
# These are module interface files under ../luabind/...
# Note: keep same relative layout as your repo (these paths are relative to this CMakeLists directory).
set(LUABIND_MODULES
    ../luabind/back_reference_fwd.cppm
    ../luabind/back_reference.cppm
    ../luabind/class.cppm
    ../luabind/class_info.cppm
    ../luabind/config.cppm
    ../luabind/error.cppm
    ../luabind/error_callback_fun.cppm
    ../luabind/exception_handler.cppm
    ../luabind/from_stack.cppm
    ../luabind/function.cppm
    ../luabind/function_introspection.cppm
    ../luabind/get_main_thread.cppm
    ../luabind/pointer_traits.cppm
    ../luabind/handle.cppm
    ../luabind/luabind.cppm         # typically the umbrella module interface
    ../luabind/lua_argument_proxy.cppm
    ../luabind/lua_index_proxy.cppm
    ../luabind/lua_iterator_proxy.cppm
    ../luabind/lua_proxy_interface.cppm
    ../luabind/lua_state_fwd.cppm
    ../luabind/make_function.cppm
    ../luabind/nil.cppm
    ../luabind/object.cppm
    ../luabind/open.cppm
    ../luabind/operator.cppm
    ../luabind/prefix.cppm
    ../luabind/scope.cppm
    ../luabind/set_package_preload.cppm
    ../luabind/shared_ptr_converter.cppm
    ../luabind/tag_function.cppm
    ../luabind/typeid.cppm
    ../luabind/lua_proxy.cppm
    ../luabind/weak_ref.cppm
    ../luabind/wrapper_base.cppm
)
source_group("API MODULES" FILES ${LUABIND_MODULES})

set(LUABIND_DETAIL_MODULES
    ../luabind/detail/meta.cppm
    ../luabind/detail/call_traits.cppm
    ../luabind/detail/call_shared.cppm
    ../luabind/detail/crtp_iterator.cppm
    ../luabind/detail/call_function.cppm
    ../luabind/detail/call.cppm
    ../luabind/detail/call_member.cppm
    ../luabind/detail/class_registry.cppm
    ../luabind/detail/class_rep.cppm
    ../luabind/detail/constructor.cppm
    ../luabind/detail/conversion_storage.cppm
    ../luabind/detail/push_to_lua.cppm
    ../luabind/detail/debug.cppm
    ../luabind/detail/decorate_type.cppm
    ../luabind/detail/deduce_signature.cppm
    ../luabind/detail/enum_maker.cppm
    ../luabind/detail/format_signature.cppm
    ../luabind/detail/garbage_collector.cppm
    ../luabind/detail/inheritance.cppm
    ../luabind/detail/instance_holder.cppm
    ../luabind/detail/link_compatibility.cppm
    ../luabind/detail/make_instance.cppm
    ../luabind/detail/object.cppm
    ../luabind/detail/object_rep.cppm
    ../luabind/detail/open.cppm
    ../luabind/detail/operator_id.cppm
    ../luabind/detail/other.cppm
    ../luabind/detail/pcall.cppm
    ../luabind/detail/policy.cppm
    ../luabind/detail/primitives.cppm
    ../luabind/detail/property.cppm
    ../luabind/detail/ref.cppm
    ../luabind/detail/signature_match.cppm
    ../luabind/detail/stack_utils.cppm
    ../luabind/detail/type_traits.cppm
)
source_group("Detail MODULES" FILES ${LUABIND_DETAIL_MODULES})

set(LUABIND_DEFAULT_POLICY_MODULES
    ../luabind/detail/conversion_policies/pointer_converter.cppm
    ../luabind/detail/conversion_policies/reference_converter.cppm
    ../luabind/detail/conversion_policies/value_converter.cppm
    ../luabind/detail/conversion_policies/enum_converter.cppm
    ../luabind/detail/conversion_policies/function_converter.cppm
    ../luabind/detail/conversion_policies/conversion_base.cppm
    ../luabind/detail/conversion_policies/conversion_policies.cppm
    ../luabind/detail/conversion_policies/lua_proxy_converter.cppm
    ../luabind/detail/conversion_policies/native_converter.cppm
)
source_group("Default Policies MODULES" FILES ${LUABIND_DEFAULT_POLICY_MODULES})

set(LUABIND_USER_POLICY_MODULES
    ../luabind/yield_policy.cppm
    ../luabind/no_dependency.cppm
    ../luabind/iterator_policy.cppm
    ../luabind/container_policy.cppm
    ../luabind/copy_policy.cppm
    ../luabind/dependency_policy.cppm
    ../luabind/discard_result_policy.cppm
    ../luabind/adopt_policy.cppm
    ../luabind/out_value_policy.cppm
    ../luabind/return_reference_to_policy.cppm
    ../luabind/raw_policy.cppm
)
source_group("User Policies MODULES" FILES ${LUABIND_USER_POLICY_MODULES})

# Collect all module interface units
set(LUABIND_MODULE_UNITS
    ${LUABIND_MODULES}
    ${LUABIND_DETAIL_MODULES}
    ${LUABIND_DEFAULT_POLICY_MODULES}
    ${LUABIND_USER_POLICY_MODULES}
)

# Create library: implementation sources + module interface units
add_library(luabind)

target_sources(luabind
  PUBLIC
    FILE_SET cxx_modules TYPE CXX_MODULES FILES
    ${LUABIND_MODULE_UNITS}
)
target_sources(luabind PRIVATE ${LUABIND_SRCS})

# Ensure C++20 (and enable module support where the compiler supports it)
# Using target property to be conservative and local to the target
target_compile_features(luabind PUBLIC cxx_std_20)

# MSVC specific flags: suppress warning 4251 if desired (keeps previous behavior)
if(MSVC)
    target_compile_options(luabind PRIVATE /wd4251)
endif()

# Linker: Lua and possibly math library on GCC-like
if(CMAKE_COMPILER_IS_GNUCXX)
    list(APPEND LUABIND_EXTRA_LIBS m)
endif()

target_link_libraries(luabind PRIVATE ${LUA_LIBRARIES} ${LUABIND_EXTRA_LIBS})

if(UNIX)
    target_link_libraries(luabind PRIVATE dl)
endif()

# Preserve previous property usage if necessary
# (You may remove the next line if not needed — it mirrors original behavior)
set_property(TARGET luabind PROPERTY COMPILE_FLAGS ${CMAKE_SHARED_LIBRARY_C_FLAGS})
set_property(TARGET luabind PROPERTY PROJECT_LABEL "LuaBind Library")

# Expose lists to parent scope to preserve original CMake API
set(ALLHEADERS ${LUABIND_MODULE_UNITS} PARENT_SCOPE)
set(APIHEADERS ${LUABIND_MODULES} PARENT_SCOPE)

# Installation: install module interface units (instead of old .hpp files).
# Destination decision: keep placing module units under include/luabind so consumers can import them
if(LUABIND_INSTALL)
    if(NOT INCLUDE_DIR)
        set(INCLUDE_DIR include)
    endif()
    if(NOT LIB_DIR)
        set(LIB_DIR lib)
    endif()
    if(NOT ARCH_DIR)
        set(ARCH_DIR lib)
    endif()
    if(NOT BIN_DIR)
        set(BIN_DIR bin)
    endif()

    # Install top-level module interfaces to ${INCLUDE_DIR}/luabind
    install(FILES ${LUABIND_MODULES}
        DESTINATION ${INCLUDE_DIR}/luabind
        COMPONENT sdk
    )

    # Install user policy module interfaces to ${INCLUDE_DIR}/luabind
    install(FILES ${LUABIND_USER_POLICY_MODULES}
        DESTINATION ${INCLUDE_DIR}/luabind
        COMPONENT sdk
    )

    # Install detail module interfaces to ${INCLUDE_DIR}/luabind/detail
    install(FILES ${LUABIND_DETAIL_MODULES}
        DESTINATION ${INCLUDE_DIR}/luabind/detail
        COMPONENT sdk
    )

    # Install default policy module interfaces to ${INCLUDE_DIR}/luabind/detail/conversion_policies
    install(FILES ${LUABIND_DEFAULT_POLICY_MODULES}
        DESTINATION ${INCLUDE_DIR}/luabind/detail/conversion_policies
        COMPONENT sdk
    )

    #install(TARGETS luabind
    #    EXPORT luabind
    #    RUNTIME DESTINATION ${BIN_DIR} COMPONENT runtime
    #    LIBRARY DESTINATION ${LIB_DIR} COMPONENT runtime
    #    ARCHIVE DESTINATION ${ARCH_DIR} COMPONENT sdk
    #)
endif()
