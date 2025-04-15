#!/bin/bash
# Comprehensive final Lua compatibility fixes

echo "==== Applying final Lua compatibility fixes ===="

# 1. First ensure lua_wrapper.c uses the correct constants
sed -i 's/EXECUTOR_LUA_TNIL/LUA_TNIL/g' source/lua_wrapper.c

# 2. Check if lfs.c has any other issues to fix
grep -n "include" source/lfs.c

# 3. Create lua_wrapper implementations for new functions
cat > source/lua_wrapper_impl.c << 'EOL'
// Implementation of additional Lua functions needed for lfs.c
#include "lua_stub/lua.h"
#include "lua_stub/lualib.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

// Required by lfs.c

// Push formatted string - Required by lfs.c
const char* lua_pushfstring(lua_State* L, const char* fmt, ...) {
    static char buffer[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    
    // Call lua_pushstring with the formatted result
    lua_pushstring(L, buffer);
    return buffer;
}

// Implementation for lua_pushboolean
void lua_pushboolean(lua_State* L, int b) {
    // Stub implementation
    printf("lua_pushboolean(%p, %d) called\n", L, b);
}

// Implementation for luaL_checkstring
const char* luaL_checkstring(lua_State* L, int numArg) {
    // Simplified wrapper around luaL_checklstring
    return luaL_checklstring(L, numArg, NULL);
}

// Implementation for lua_newuserdata
void* lua_newuserdata(lua_State* L, size_t size) {
    // Simple stub implementation that just allocates memory
    // This won't be linked to any actual Lua state
    return malloc(size);
}

// Implementation for luaL_checkudata
void* luaL_checkudata(lua_State* L, int ud, const char* tname) {
    // Simple stub that returns a dummy pointer
    static char dummy[1024];
    return dummy;
}

// Implementation for luaL_getmetatable
void luaL_getmetatable(lua_State* L, const char* tname) {
    // Simplified implementation that does nothing
    printf("luaL_getmetatable(%p, %s) called\n", L, tname);
}

// Implementation for lua_setmetatable
void lua_setmetatable(lua_State* L, int idx) {
    // Simplified implementation that does nothing
    printf("lua_setmetatable(%p, %d) called\n", L, idx);
}

// Implementation for luaL_checkoption
int luaL_checkoption(lua_State* L, int narg, const char* def, const char* const lst[]) {
    // Simple implementation that always returns 0 (first option)
    return 0;
}
EOL

# 4. Update CMakeLists.txt to include our new implementation
if ! grep -q "lua_wrapper_impl.c" CMakeLists.txt; then
    sed -i '/add_library(lua_wrapper/,/)/c\
add_library(lua_wrapper STATIC \
    source/lua_wrapper.c \
    source/lua_wrapper_impl.c \
)' CMakeLists.txt
fi

# 5. Update target_include_directories for lua_wrapper if needed
if grep -q "target_include_directories(lua_wrapper" CMakeLists.txt; then
    sed -i '/target_include_directories(lua_wrapper/,/)/c\
target_include_directories(lua_wrapper PUBLIC \
    source \
    source/lua_stub \
)' CMakeLists.txt
else
    # Add it if it doesn't exist
    sed -i '/add_library(lua_wrapper/a\
target_include_directories(lua_wrapper PUBLIC \
    source \
    source/lua_stub \
)' CMakeLists.txt
fi

echo "==== Final Lua compatibility fixes applied ===="
