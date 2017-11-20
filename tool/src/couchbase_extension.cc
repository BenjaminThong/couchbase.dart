/**
 * # perform STORE and 20 iterations of GET commands with interval 3 seconds
 * ./a.out couchbase://localhost password Administrator 20 3
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <unordered_map>
#include <iostream>
#include <libcouchbase/couchbase.h>
#include <libcouchbase/api3.h>
#include <libcouchbase/n1ql.h>
#include "include/dart_api.h"
#include "include/dart_native_api.h"

int nreq = 1;
int nresp = 1;
int interval = 0;
struct event *timer = NULL;

bool initialised = false;
lcb_io_opt_t ioops;
struct event_base *evbase;

std::unordered_map<std::string, Dart_NativeFunction> function_map;
std::unordered_map<std::string, Dart_NativeFunction> no_scope_function_map;
std::unordered_map<std::string, lcb_t> couchbase_connection_map;

static lcb_t* getInstance(std::string key) {
    if (couchbase_connection_map.find(key) == couchbase_connection_map.end()) {
        return nullptr;
    }
    return &couchbase_connection_map[key];
}

// Forward declaration of ResolveName function.
Dart_NativeFunction ResolveName(Dart_Handle name,
	int argc,
	bool* auto_setup_scope);
 


Dart_Handle HandleError(Dart_Handle handle) {
 if (Dart_IsError(handle)) Dart_PropagateError(handle);
 return handle;
}

void dartPostNull(Dart_Port reply_port_id) {
	Dart_CObject result;
	result.type = Dart_CObject_kNull;
	Dart_PostCObject(reply_port_id, &result);
}

/**************************
 *  Random Array
 **************************/

uint8_t* randomArray(int seed, int length) {
	if (length <= 0 || length > 10000000) {
		return NULL;
	}
	uint8_t* values = reinterpret_cast<uint8_t*>(malloc(length));
	if (NULL == values) {
		return NULL;
	}
	srand(seed);
	for (int i = 0; i < length; ++i) {
		values[i] = rand() % 256;
	}
	return values;
}

void wrappedRandomArray(Dart_Port dest_port_id,
	Dart_CObject* message) {
	Dart_Port reply_port_id = ILLEGAL_PORT;
	if (message->type == Dart_CObject_kArray &&
		3 == message->value.as_array.length) {
		// Use .as_array and .as_int32 to access the data in the Dart_CObject.
		Dart_CObject* param0 = message->value.as_array.values[0];
		Dart_CObject* param1 = message->value.as_array.values[1];
		Dart_CObject* param2 = message->value.as_array.values[2];
		if (param0->type == Dart_CObject_kInt32 &&
			param1->type == Dart_CObject_kInt32 &&
			param2->type == Dart_CObject_kSendPort) {
			int seed = param0->value.as_int32;
			int length = param1->value.as_int32;
			reply_port_id = param2->value.as_send_port.id;
			uint8_t* values = randomArray(seed, length);

			if (values != NULL) {
				Dart_CObject result;
				result.type = Dart_CObject_kTypedData;
				result.value.as_typed_data.type = Dart_TypedData_kUint8;
				result.value.as_typed_data.values = values;
				result.value.as_typed_data.length = length;
				Dart_PostCObject(reply_port_id, &result);
				free(values);
				// It is OK that result is destroyed when function exits.
				// Dart_PostCObject has copied its data.
				return;
			}
		}
	}
	dartPostNull(reply_port_id);
}

void randomArrayServicePort(Dart_NativeArguments arguments) {
	Dart_EnterScope();
	Dart_SetReturnValue(arguments, Dart_Null());
	Dart_Port service_port =
		Dart_NewNativePort("RandomArrayService", wrappedRandomArray, true);
	if (service_port != ILLEGAL_PORT) {
		Dart_Handle send_port = HandleError(Dart_NewSendPort(service_port));
		Dart_SetReturnValue(arguments, send_port);
	}
	Dart_ExitScope();
}

/**************************
 *  Connect to bucket
 **************************/
bool connectToBucket(char* connstr, char* username, char* password) {
    // check if connection was already established prior
    std::string combined = std::string(connstr) + '/' + username + '/' + password;
    if (couchbase_connection_map.find(combined) != couchbase_connection_map.end()) {
        return true;
    }
    
	lcb_t instance;
    lcb_error_t error;
    struct lcb_create_st copts;

    memset(&copts, 0, sizeof(copts));

    copts.version = 3;
	copts.v.v3.connstr = connstr;
	copts.v.v3.passwd = password;
	copts.v.v3.username = username;
    copts.v.v3.io = ioops;
    error = lcb_create(&instance, &copts);

    if (error != LCB_SUCCESS) {
        fprintf(stderr, "Failed to create a libcouchbase instance: %s\n", lcb_strerror(NULL, error));
        return false;
    }

    /* Set up the callbacks */
//    lcb_install_callback3(instance, LCB_CALLBACK_GET, get_callback);
//    lcb_install_callback3(instance, LCB_CALLBACK_STORE, store_callback);

    if ((error = lcb_connect(instance)) != LCB_SUCCESS) {
        fprintf(stderr, "Failed to connect libcouchbase instance: %s\n", lcb_strerror(NULL, error));
        lcb_destroy(instance);
		return false;
    }

	lcb_wait(instance);
    error = lcb_get_bootstrap_status(instance);
    if (error != LCB_SUCCESS) {
        fprintf(stderr, "ERROR: %s\n", lcb_strerror(instance, error));
		lcb_destroy(instance);
		return false;
    }

//    printf("successfully bootstrapped\n");
    // fflush(stdout);
    couchbase_connection_map[combined] = instance;
	return true;
}

void wrappedConnectToBucket(Dart_Port dest_port_id,
	Dart_CObject* message) {
	Dart_Port reply_port_id = ILLEGAL_PORT;
	if (message->type == Dart_CObject_kArray &&
		4 == message->value.as_array.length) {
		// Use .as_array and .as_int32 to access the data in the Dart_CObject.
		Dart_CObject* param0 = message->value.as_array.values[0];
		Dart_CObject* param1 = message->value.as_array.values[1];
		Dart_CObject* param2 = message->value.as_array.values[2];
		Dart_CObject* param3 = message->value.as_array.values[3];
		if (param0->type == Dart_CObject_kString &&
			param1->type == Dart_CObject_kString &&
            param2->type == Dart_CObject_kString &&
			param3->type == Dart_CObject_kSendPort) {
            char* connstr = param0->value.as_string;
            char* username = param1->value.as_string;
            char* password = param2->value.as_string;
			reply_port_id = param3->value.as_send_port.id;

			Dart_CObject result;
            result.type = Dart_CObject_kBool;
            result.value.as_bool = connectToBucket(connstr, username, password);
			Dart_PostCObject(reply_port_id, &result);
			return;
		}
	}
	Dart_CObject result;
	result.type = Dart_CObject_kBool;
    result.value.as_bool = false;
	Dart_PostCObject(reply_port_id, &result);
}

void connectToBucketServicePort(Dart_NativeArguments arguments) {
	Dart_EnterScope();
	Dart_SetReturnValue(arguments, Dart_Null());
	Dart_Port service_port =
		Dart_NewNativePort("ConnectToBucketService", wrappedConnectToBucket, true);
	if (service_port != ILLEGAL_PORT) {
		Dart_Handle send_port = HandleError(Dart_NewSendPort(service_port));
		Dart_SetReturnValue(arguments, send_port);
	}
	Dart_ExitScope();
}


/**************************
 *  N1QL Query
 **************************/
static void rowCallback(lcb_t instance, int cbtype, const lcb_RESPN1QL *resp) {
    Dart_Port *reply_port_id = static_cast<Dart_Port *>(resp->cookie);
    
    Dart_CObject result;
    result.type = Dart_CObject_kString;
    std::string r(resp->row, resp->nrow);
//    std::cout << r << std::endl;
    result.value.as_string = (char *)r.c_str();
    Dart_PostCObject(*reply_port_id, &result);
    
    if (! (resp->rflags & LCB_RESP_F_FINAL)) {
//        printf("Row: %.*s\n", (int)resp->nrow, resp->row);
    } else {
        dartPostNull(*reply_port_id);
//        printf("Got metadata: %.*s\n", (int)resp->nrow, resp->row);
    }
    fflush(stdout);
}

void n1qlQuery(char* querystr, lcb_t instance, Dart_Port reply_port_id) {
    lcb_error_t rc;
    lcb_CMDN1QL cmd = { 0 };
    lcb_N1QLPARAMS *params = lcb_n1p_new();
    
    rc = lcb_n1p_setstmtz(params, querystr);
//    rc = lcb_n1p_namedparamz(params, "$any", "testvalue");
    
    cmd.callback = rowCallback;
    
    rc = lcb_n1p_mkcmd(params, &cmd);
    rc = lcb_n1ql_query(instance, &reply_port_id, &cmd);
    lcb_n1p_free(params);
    lcb_wait(instance);
}

void wrappedN1QLQuery(Dart_Port dest_port_id,
                            Dart_CObject* message) {
    Dart_Port reply_port_id = ILLEGAL_PORT;
    if (message->type == Dart_CObject_kArray &&
        3 == message->value.as_array.length) {
        // Use .as_array and .as_int32 to access the data in the Dart_CObject.
        Dart_CObject* param0 = message->value.as_array.values[0];
        Dart_CObject* param1 = message->value.as_array.values[1];
        Dart_CObject* param2 = message->value.as_array.values[2];
        if (param0->type == Dart_CObject_kString &&
            param1->type == Dart_CObject_kString &&
            param2->type == Dart_CObject_kSendPort) {
            char* query = param0->value.as_string;
            char* instanceKey = param1->value.as_string;
            reply_port_id = param2->value.as_send_port.id;
            lcb_t* instance = getInstance(std::string(instanceKey));
            if (instance == nullptr) {
				dartPostNull(reply_port_id);
            }
            return n1qlQuery(query, *instance, reply_port_id);
        }
    }
	dartPostNull(reply_port_id);
}

void connectToN1QLQueryServicePort(Dart_NativeArguments arguments) {
    Dart_EnterScope();
    Dart_SetReturnValue(arguments, Dart_Null());
    Dart_Port service_port =
    Dart_NewNativePort("ConnectToN1QLService", wrappedN1QLQuery, true);
    if (service_port != ILLEGAL_PORT) {
        Dart_Handle send_port = HandleError(Dart_NewSendPort(service_port));
        Dart_SetReturnValue(arguments, send_port);
    }
    Dart_ExitScope();
}

/*********************************************/

struct FunctionLookup {
	const char* name;
	Dart_NativeFunction function;
};

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
