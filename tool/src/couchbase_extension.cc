/**
 * # perform STORE and 20 iterations of GET commands with interval 3 seconds
 * ./a.out couchbase://localhost password Administrator 20 3
 */

#include <stdlib.h>
#include <string>
#include <unordered_map>
#include "include/dart_api.h"
#include "include/dart_native_api.h"
#include "dart_ext_helper.h"
#include "random_array.h"
#include "cluster.h"
#include "bucket.h"


std::unordered_map<std::string, Dart_NativeFunction> function_map;
std::unordered_map<std::string, Dart_NativeFunction> no_scope_function_map;

Dart_NativeFunction ResolveName(Dart_Handle name,
	int argc,
	bool* auto_setup_scope) {
	if (!Dart_IsString(name)) {
		return NULL;
	}
	Dart_NativeFunction result = NULL;
	if (auto_setup_scope == NULL) {
		return NULL;
	}

	Dart_EnterScope();
	const char* cname;
	HandleError(Dart_StringToCString(name, &cname));
    const std::string nameStr(cname);
    
    if (function_map.find(nameStr) != function_map.end()) {
        *auto_setup_scope = true;
        result = function_map[nameStr];
    }

	if (result != NULL) {
		Dart_ExitScope();
		return result;
	}
    
    if (no_scope_function_map.find(nameStr) != no_scope_function_map.end()) {
        *auto_setup_scope = false;
        result = no_scope_function_map[nameStr];
    }

	Dart_ExitScope();
	return result;
}

// The name of the initialization function is the extension name followed
// by _Init.
DART_EXPORT Dart_Handle couchbase_extension_Init(Dart_Handle parent_library) {
    if (Dart_IsError(parent_library)) return parent_library;

    function_map["RandomArray_ServicePort"] = randomArrayServicePort;
    function_map["ConnectToBucket_ServicePort"] = connectToBucketServicePort;
    function_map["N1QLQuery_ServicePort"] = connectToN1QLQueryServicePort;
    
//    no_scope_function_map.insert(std::make_pair("NoScopeSystemRand", SystemRand));
    
    Dart_Handle result_code =
    Dart_SetNativeResolver(parent_library, ResolveName, NULL);
    if (Dart_IsError(result_code)) return result_code;
    
    return Dart_Null();
}

// Initializer.
__attribute__((constructor))
static void initializer(void) {
//    printf("[%s] initializer()\n", __FILE__);
}
 
// Finalizer.
__attribute__((destructor))
static void finalizer(void) {
//    printf("[%s] finalizer()\n", __FILE__);
}
