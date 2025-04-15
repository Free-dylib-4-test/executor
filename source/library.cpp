#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <vector>
#include <map>
#include <functional>
#include <memory>
#include <cstdlib>

// Lua headers
extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

// Simple lua_dostring implementation
static int luaL_dostring(lua_State* L, const char* str) {
    if (luau_load(L, "string", str, strlen(str), 0) != 0) {
        return 1;  // Compilation error
    }
    return lua_pcall(L, 0, LUA_MULTRET, 0);  // Execute and return status
}

// Simple script without raw string literals
const char* mainLuauScript = 
"-- This is the main Luau script that runs the executor\n"
"local workspaceDir = 'workspace'\n"
"local function setup()\n"
"    print(\"Setting up workspace...\")\n"
"    return true\n"
"end\n\n"
"-- Main function that executes when a player is detected\n"
"local function onPlayerAdded(player)\n"
"    print(\"Player added: \"..tostring(player))\n"
"    return true\n"
"end\n\n"
"local function initialize()\n"
"    setup()\n"
"    return onPlayerAdded\n"
"end\n\n"
"return initialize()";

// Ensure workspace directory exists - simple implementation
void ensureWorkspaceDirectory() {
    // Simple cross-platform implementation without std::filesystem
    #ifdef _WIN32
    system("if not exist workspace mkdir workspace");
    #else
    system("mkdir -p workspace");
    #endif
}

// Function to read a file as a string - simple replacement for std::filesystem
std::string readfile(lua_State* L) {
    const char* filename = lua_tostring(L, 1);
    if (!filename) {
        lua_pushnil(L);
        lua_pushstring(L, "No filename provided");
        return "";
    }
    
    ensureWorkspaceDirectory();
    
    // Construct full path in a simple way
    std::string fullPath = "workspace/";
    fullPath += filename;
    
    // Open and read the file
    std::ifstream file(fullPath.c_str());
    if (!file.is_open()) {
        lua_pushnil(L);
        lua_pushstring(L, "Failed to open file");
        return "";
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    
    // Return content
    lua_pushstring(L, buffer.str().c_str());
    return buffer.str();
}

// Register script functions to Lua
void registerExecutorFunctions(lua_State* L) {
    lua_register(L, "readfile", [](lua_State* L) -> int {
        readfile(L);
        return 1;
    });
    
    lua_register(L, "writefile", [](lua_State* L) -> int {
        const char* filename = lua_tostring(L, 1);
        const char* content = lua_tostring(L, 2);
        
        if (!filename || !content) {
            lua_pushboolean(L, 0);
            return 1;
        }
        
        ensureWorkspaceDirectory();
        
        // Construct full path
        std::string fullPath = "workspace/";
        fullPath += filename;
        
        // Create parent directories if needed (simple version)
        #ifdef _WIN32
        system("if not exist workspace mkdir workspace");
        #else
        system("mkdir -p workspace");
        #endif
        
        // Write the file
        std::ofstream file(fullPath.c_str());
        if (!file.is_open()) {
            lua_pushboolean(L, 0);
            return 1;
        }
        
        file << content;
        file.close();
        
        lua_pushboolean(L, 1);
        return 1;
    });
}

// Execute main Luau script
bool executeMainLuau(lua_State* L, const std::string& script) {
    // Execute the script
    if (luaL_dostring(L, script.c_str()) != 0) {
        // Get the error message
        std::string errorMsg = lua_tostring(L, -1);
        std::cerr << "Failed to execute script: " << errorMsg << std::endl;
        lua_pop(L, 1);  // Pop error message
        return false;
    }
    
    // Check if the script returned a valid function
    if (!lua_isfunction(L, -1)) {
        std::cerr << "Script did not return a function" << std::endl;
        lua_pop(L, 1);  // Pop return value
        return false;
    }
    
    return true;
}

// Hook the player added event
lua_State* hookPlayerAddedEvent(lua_State* L) {
    // Save the function reference
    lua_pushvalue(L, -1);
    int functionRef = luaL_ref(L, LUA_REGISTRYINDEX);
    
    // Return L for convenience
    return L;
}

// Handler for when a player is added
int playerAddedHandler(lua_State* L) {
    const char* playerName = lua_tolstring(L, 1, nullptr);
    if (!playerName) {
        playerName = "Unknown";
    }
    
    std::cout << "Player added: " << playerName << std::endl;
    return 0;
}

// Generate a script dynamically (for testing/demo purposes)
int generateScript(lua_State* L) {
    const char* template_str = lua_tostring(L, 1);
    if (!template_str) {
        lua_pushnil(L);
        return 1;
    }
    
    // Simple templating
    std::string result = template_str;
    
    // Push the result
    lua_pushstring(L, result.c_str());
    return 1;
}

// Scan for vulnerabilities (for demo purposes)
int scanVulnerabilities(lua_State* L) {
    const char* code = lua_tostring(L, 1);
    if (!code) {
        lua_pushnil(L);
        return 1;
    }
    
    // Simple "vulnerability" check
    bool hasVulnerability = strstr(code, "while true do") != nullptr;
    
    // Push the result
    lua_pushstring(L, hasVulnerability ? "Vulnerability found: Infinite loop" : "No vulnerabilities found");
    return 1;
}

// Library initialization
extern "C" int luaopen_mylibrary(lua_State* L) {
    // Setup workspace
    ensureWorkspaceDirectory();
    
    // Register functions
    registerExecutorFunctions(L);
    
    // Execute main Luau script
    if (executeMainLuau(L, mainLuauScript)) {
        // Hook player added event
        hookPlayerAddedEvent(L);
    }
    
    // Return 1 to indicate that we're returning a value
    return 1;
}
