cmake_minimum_required(VERSION 3.20)
project(luabind LANGUAGES CXX)

# Build for LuaBind (module-aware / C++20 version)
# Updated to consume C++20 module interface units (.cppm) instead of plain headers
# and to build with C++20 (module support).
#
# Original author: Ryan Pavlik <rpavlik@iastate.edu>
# This file adapted to use C++20 modules & module interface units.

# Sources (implementation .cpp files stay as-is)
set(LUABIND_SRCS
    implementation/class.cpp
    implementation/class_info.cpp
    implementation/class_registry.cpp
    implementation/class_rep.cpp
    implementation/create_class.cpp
    implementation/error.cpp
    implementation/exception_handler.cpp
    implementation/function.cpp
    implementation/function_introspection.cpp
    implementation/inheritance.cpp
    implementation/link_compatibility.cpp
    implementation/object_rep.cpp
    implementation/open.cpp
    implementation/operator.cpp
    implementation/pcall.cpp
    implementation/scope.cpp
    implementation/set_package_preload.cpp
    implementation/stack_content_by_name.cpp
    implementation/weak_ref.cpp
    implementation/wrapper_base.cpp
)
source_group("Sources" FILES ${LUABIND_SRCS})

# Module interface units (converted from the old headers to .cppm)
# These are module interface files under interface/...
# Note: keep same relative layout as your repo (these paths are relative to this CMakeLists directory).
set(LUABIND_MODULES
    interface/back_reference_fwd.cppm
    interface/back_reference.cppm
    interface/class.cppm
    interface/class_info.cppm
    interface/config.cppm
    interface/error.cppm
    interface/error_callback_fun.cppm
    interface/exception_handler.cppm
    interface/from_stack.cppm
    interface/function.cppm
    interface/function_introspection.cppm
    interface/get_main_thread.cppm
    interface/pointer_traits.cppm
    interface/pseudo_traits.cppm
    interface/handle.cppm
    interface/luabind.cppm         # typically the umbrella module interface
    interface/lua_argument_proxy.cppm
    interface/lua_index_proxy.cppm
    interface/lua_iterator_proxy.cppm
    interface/lua_proxy_interface.cppm
    interface/lua_state_fwd.cppm
    interface/make_function.cppm
    interface/nil.cppm
    interface/object.cppm
    interface/open.cppm
    interface/operator.cppm
    interface/prefix.cppm
    interface/scope.cppm
    interface/set_package_preload.cppm
    interface/shared_ptr_converter.cppm
    interface/tag_function.cppm
    interface/typeid.cppm
    interface/lua_proxy.cppm
    interface/weak_ref.cppm
    interface/wrapper_base.cppm
)
source_group("API MODULES" FILES ${LUABIND_MODULES})

set(LUABIND_DETAIL_MODULES
    interface/detail/meta.cppm
    interface/detail/call_traits.cppm
    interface/detail/call_shared.cppm
    interface/detail/crtp_iterator.cppm
    interface/detail/call_function.cppm
    interface/detail/call.cppm
    interface/detail/call_member.cppm
    interface/detail/class_registry.cppm
    interface/detail/class_rep.cppm
    interface/detail/constructor.cppm
    interface/detail/conversion_storage.cppm
    interface/detail/push_to_lua.cppm
    interface/detail/debug.cppm
    interface/detail/decorate_type.cppm
    interface/detail/deduce_signature.cppm
    interface/detail/enum_maker.cppm
    interface/detail/format_signature.cppm
    interface/detail/garbage_collector.cppm
    interface/detail/inheritance.cppm
    interface/detail/instance_holder.cppm
    interface/detail/link_compatibility.cppm
    interface/detail/make_instance.cppm
    interface/detail/object.cppm
    interface/detail/object_rep.cppm
    interface/detail/open.cppm
    interface/detail/operator_id.cppm
    interface/detail/other.cppm
    interface/detail/pcall.cppm
    interface/detail/policy.cppm
    interface/detail/primitives.cppm
    interface/detail/property.cppm
    interface/detail/ref.cppm
    interface/detail/signature_match.cppm
    interface/detail/signature_types.cppm
    interface/detail/stack_utils.cppm
    interface/detail/type_info.cppm
    interface/detail/type_traits.cppm
)
source_group("Detail MODULES" FILES ${LUABIND_DETAIL_MODULES})

set(LUABIND_DEFAULT_POLICY_MODULES
    interface/detail/conversion_policies/pointer_converter.cppm
    interface/detail/conversion_policies/reference_converter.cppm
    interface/detail/conversion_policies/value_converter.cppm
    interface/detail/conversion_policies/enum_converter.cppm
    interface/detail/conversion_policies/function_converter.cppm
    interface/detail/conversion_policies/conversion_base.cppm
    interface/detail/conversion_policies/conversion_policies.cppm
    interface/detail/conversion_policies/lua_proxy_converter.cppm
    interface/detail/conversion_policies/native_converter.cppm
)
source_group("Default Policies MODULES" FILES ${LUABIND_DEFAULT_POLICY_MODULES})

set(LUABIND_USER_POLICY_MODULES
    interface/yield_policy.cppm
    interface/no_dependency.cppm
    interface/iterator_policy.cppm
    interface/container_policy.cppm
    interface/copy_policy.cppm
    interface/dependency_policy.cppm
    interface/discard_result_policy.cppm
    interface/adopt_policy.cppm
    interface/out_value_policy.cppm
    interface/return_reference_to_policy.cppm
    interface/raw_policy.cppm
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

target_compile_features(luabind PRIVATE cxx_std_20)
set_target_properties(luabind PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(luabind PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_sources(luabind
  PUBLIC
    FILE_SET cxx_modules TYPE CXX_MODULES FILES
    ${LUABIND_MODULE_UNITS}
)
target_sources(luabind PRIVATE ${LUABIND_SRCS})
target_include_directories(luabind PRIVATE "./")

target_compile_definitions(luabind PRIVATE "-DLUABIND_BUILDING")

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
