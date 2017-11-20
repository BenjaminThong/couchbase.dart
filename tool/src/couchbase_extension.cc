/**
 * # perform STORE and 20 iterations of GET commands with interval 3 seconds
 * ./a.out couchbase://localhost password Administrator 20 3
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <unordered_map>
#include <libcouchbase/couchbase.h>
#include <libcouchbase/api3.h>
#include <libcouchbase/n1ql.h>
#include <event2/event.h>
#include "include/dart_api.h"
#include "include/dart_native_api.h"

//const char key[] = "foo";
//lcb_SIZE nkey = sizeof(key);
//
//const char val[] = "{\"answer\":42}";
//lcb_SIZE nval = sizeof(val);

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

//static void bootstrap_callback(lcb_t instance, lcb_error_t err)
//{
////    lcb_CMDSTORE cmd = {0};
//    if (err != LCB_SUCCESS) {
//        fprintf(stderr, "ERROR: %s\n", lcb_strerror(instance, err));
//        // exit(EXIT_FAILURE);
//    }
//    // /* Since we've got our configuration, let's go ahead and store a value */
//    // LCB_CMD_SET_KEY(&cmd, key, nkey);
//    // LCB_CMD_SET_VALUE(&cmd, val, nval);
//    // cmd.operation = LCB_SET;
//    // err = lcb_store3(instance, NULL, &cmd);
//    // if (err != LCB_SUCCESS) {
//    //     fprintf(stderr, "Failed to set up store request: %s\n", lcb_strerror(instance, err));
//    //     exit(EXIT_FAILURE);
//    // }
//}

//static void get_callback(lcb_t instance, int cbtype, const lcb_RESPBASE *rb)
//{
//    const lcb_RESPGET *rg = (const lcb_RESPGET *)rb;
//    if (rg->rc != LCB_SUCCESS) {
//        fprintf(stderr, "Failed to get key: %s\n", lcb_strerror(instance, rg->rc));
//        exit(EXIT_FAILURE);
//    }
//
//    printf("%d. retrieved the key 'foo', value: %.*s\n", nresp, (int)rg->nvalue, rg->value);
//    fflush(stdout);
//    nresp--;
////    if (nresp == 0) {
////        printf("stopping the loop\n");
////        event_base_loopbreak((void *)lcb_get_cookie(instance));
////    }
//    (void)cbtype;
//}

//static void schedule_timer();
//
//static void timer_callback(int fd, short event, void *arg)
//{
//    lcb_t instance = (lcb_t)arg;
//    lcb_error_t rc;
//    lcb_CMDGET gcmd = {0};
//
//    LCB_CMD_SET_KEY(&gcmd, key, nkey);
//    rc = lcb_get3(instance, NULL, &gcmd);
//    if (rc != LCB_SUCCESS) {
//        fprintf(stderr, "Failed to schedule get request: %s\n", lcb_strerror(NULL, rc));
//        exit(EXIT_FAILURE);
//    }
//    (void)fd;
//    (void)event;
//    schedule_timer();
//}
//
//static void schedule_timer()
//{
//    struct timeval tv;
//
//    if (!nreq) {
//        return;
//    }
//    tv.tv_sec = interval;
//    tv.tv_usec = 0;
//    evtimer_add(timer, &tv);
//    nreq--;
//}

//static void store_callback(lcb_t instance, int cbtype, const lcb_RESPBASE *rb)
//{
//    if (rb->rc != LCB_SUCCESS) {
//        fprintf(stderr, "Failed to store key: %s\n", lcb_strerror(instance, rb->rc));
//        exit(EXIT_FAILURE);
//    }
//    printf("stored key 'foo'\n");
//    fflush(stdout);
//    {
//        struct event_base *evbase = (struct event_base *)lcb_get_cookie(instance);
//
//        printf("try to get value %d times with %dsec interval\n", nreq, interval);
//        timer = evtimer_new(evbase, timer_callback, instance);
//        schedule_timer();
//    }
//
//    (void)cbtype;
//}

static lcb_t* getInstance(std::string key) {
    if (couchbase_connection_map.find(key) == couchbase_connection_map.end()) {
        return nullptr;
    }
    return &couchbase_connection_map[key];
}

//static lcb_io_opt_t create_libevent_io_ops(struct event_base *evbase)
//{
//    struct lcb_create_io_ops_st ciops;
//    lcb_io_opt_t ioops;
//    lcb_error_t error;
//
//    memset(&ciops, 0, sizeof(ciops));
//    ciops.v.v0.type = LCB_IO_OPS_LIBEVENT;
//    ciops.v.v0.cookie = evbase;
//
//    error = lcb_create_io_ops(&ioops, &ciops);
//    if (error != LCB_SUCCESS) {
//        fprintf(stderr, "Failed to create an IOOPS structure for libevent: %s\n", lcb_strerror(NULL, error));
//        exit(EXIT_FAILURE);
//    }
//
//    return ioops;
//}

//static lcb_t create_libcouchbase_handle(lcb_io_opt_t ioops, int argc, char **argv)
//{
//    lcb_t instance;
//    lcb_error_t error;
//    struct lcb_create_st copts;
//
//    memset(&copts, 0, sizeof(copts));
//
//    /* If NULL, will default to localhost */
//    copts.version = 3;
//    if (argc > 1) {
//        copts.v.v3.connstr = argv[1];
//    }
//    if (argc > 2) {
//        copts.v.v3.passwd = argv[2];
//    }
//    if (argc > 3) {
//        copts.v.v3.username = argv[3];
//    }
//    copts.v.v3.io = ioops;
//    error = lcb_create(&instance, &copts);
//
//    if (error != LCB_SUCCESS) {
//        fprintf(stderr, "Failed to create a libcouchbase instance: %s\n", lcb_strerror(NULL, error));
//        exit(EXIT_FAILURE);
//    }
//
//    /* Set up the callbacks */
//    lcb_set_bootstrap_callback(instance, bootstrap_callback);
//    lcb_install_callback3(instance, LCB_CALLBACK_GET, get_callback);
//    lcb_install_callback3(instance, LCB_CALLBACK_STORE, store_callback);
//
//    if ((error = lcb_connect(instance)) != LCB_SUCCESS) {
//        fprintf(stderr, "Failed to connect libcouchbase instance: %s\n", lcb_strerror(NULL, error));
//        lcb_destroy(instance);
//        exit(EXIT_FAILURE);
//    }
//
//    return instance;
//}

/* This example shows how we can hook ourself into an external event loop.
* You may find more information in the blogpost: http://goo.gl/fCTrX */
// int main(int argc, char **argv)
// {
//     struct event_base *evbase = event_base_new();
//     lcb_io_opt_t ioops = create_libevent_io_ops(evbase);
//     lcb_t instance = create_libcouchbase_handle(ioops, argc, argv);

//     if (argc > 4) {
//         nreq = nresp = atoi(argv[4]);
//     }
//     if (argc > 5) {
//         interval = atoi(argv[5]);
//     }
//     /*Store the event base as the user cookie in our instance so that
//     * we may terminate the program when we're done */
//     lcb_set_cookie(instance, evbase);

//     /* Run the event loop */
//     event_base_loop(evbase, EVLOOP_NO_EXIT_ON_EMPTY);

//     /* Cleanup */
//     lcb_destroy(instance);
//     if (timer) {
//         evtimer_del(timer);
//     }
//     lcb_destroy_io_ops(ioops);
//     event_base_free(evbase);

//     return EXIT_SUCCESS;
// }

// Forward declaration of ResolveName function.
Dart_NativeFunction ResolveName(Dart_Handle name,
	int argc,
	bool* auto_setup_scope);
 


Dart_Handle HandleError(Dart_Handle handle) {
 if (Dart_IsError(handle)) Dart_PropagateError(handle);
 return handle;
}



/**************************
 *  Random Array Test Start
 **************************/
// void SystemRand(Dart_NativeArguments arguments) {
// 	Dart_EnterScope();
// 	Dart_Handle result = HandleError(Dart_NewInteger(rand()));
// 	Dart_SetReturnValue(arguments, result);
// 	Dart_ExitScope();
// }

// void SystemSrand(Dart_NativeArguments arguments) {
// 	Dart_EnterScope();
// 	bool success = false;
// 	Dart_Handle seed_object = HandleError(Dart_GetNativeArgument(arguments, 0));
// 	if (Dart_IsInteger(seed_object)) {
// 		bool fits;
// 		HandleError(Dart_IntegerFitsIntoInt64(seed_object, &fits));
// 		if (fits) {
// 			int64_t seed;
// 			HandleError(Dart_IntegerToInt64(seed_object, &seed));
// 			srand(static_cast<unsigned>(seed));
// 			success = true;
// 		}
// 	}
// 	Dart_SetReturnValue(arguments, HandleError(Dart_NewBoolean(success)));
// 	Dart_ExitScope();
// }

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
	Dart_CObject result;
	result.type = Dart_CObject_kNull;
	Dart_PostCObject(reply_port_id, &result);
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
 *  Init Event Loop
 **************************/

//void initEventLoop() {
//    if (initialised) {
//        return;
//    }
//    /* Run the event loop */
//    evbase = event_base_new();
//    ioops = create_libevent_io_ops(evbase);
//    initialised = true;
//
//    event_base_loop(evbase, EVLOOP_NO_EXIT_ON_EMPTY);
//    lcb_destroy_io_ops(ioops);
//    event_base_free(evbase);
//}
//
//void wrappedInitEventLoop(Dart_Port dest_port_id,
//    Dart_CObject* message) {
//    Dart_Port reply_port_id = ILLEGAL_PORT;
//    if (message->type == Dart_CObject_kArray &&
//        1 == message->value.as_array.length) {
//        // Use .as_array and .as_int32 to access the data in the Dart_CObject.
//        Dart_CObject* param0 = message->value.as_array.values[0];
//        if (param0->type == Dart_CObject_kSendPort) {
//            reply_port_id = param0->value.as_send_port.id;
//
//            Dart_CObject result;
//            result.type = Dart_CObject_kBool;
//            result.value.as_bool = true;
//
//            Dart_PostCObject(reply_port_id, &result);
//            initEventLoop();
//            return;
//        }
//    }
//    Dart_CObject result;
//    result.type = Dart_CObject_kBool;
//    result.value.as_bool = false;
//    Dart_PostCObject(reply_port_id, &result);
//}
//
//void initEventLoopServicePort(Dart_NativeArguments arguments) {
//    Dart_EnterScope();
//    Dart_SetReturnValue(arguments, Dart_Null());
//    Dart_Port service_port =
//        Dart_NewNativePort("InitEventLoop", wrappedInitEventLoop, true);
//    if (service_port != ILLEGAL_PORT) {
//        Dart_Handle send_port = HandleError(Dart_NewSendPort(service_port));
//        Dart_SetReturnValue(arguments, send_port);
//    }
//    Dart_ExitScope();
//}

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
    printf("gsgsgsgsgs\n");
    if (! (resp->rflags & LCB_RESP_F_FINAL)) {
        printf("Row: %.*s\n", (int)resp->nrow, resp->row);
    } else {
        printf("Got metadata: %.*s\n", (int)resp->nrow, resp->row);
    }
    fflush(stdout);
}

void n1qlQuery(char* querystr, lcb_t instance, Dart_Port reply_port_id) {
    lcb_error_t rc;
    lcb_CMDN1QL cmd = { 0 };
    lcb_N1QLPARAMS *params = lcb_n1p_new();
    
    std::string city_str;
    city_str += '"';
    city_str += "singaporeee";
    city_str += '"';
    
    rc = lcb_n1p_setstmtz(params, querystr);
//                          "SELECT * FROM `default` USE KEYS [\"fsfs\", \"ahah\"]");
//    rc = lcb_n1p_posparam(params, "$any", "lala");
//    rc = lcb_n1p_namedparamz(params, "$any", "testvalue");
    
    cmd.callback = rowCallback;
    
    rc = lcb_n1p_mkcmd(params, &cmd);
    rc = lcb_n1ql_query(instance, NULL, &cmd);
    lcb_n1p_free(params);
    lcb_wait(instance);
    
//    rc = lcb_n1p_setstmtz(nparams, querystr);
//
//    lcb_n1p_mkcmd(nparams, &cmd);
//
//    cmd.query = querystr;
//    cmd.nquery = strlen(querystr);
//    printf("Encoded query is: %s\n", lcb_n1p_encode(nparams, &rc));
//    fflush(stdout);
//    cmd.callback = rowCallback;
//
//    rc = lcb_n1ql_query(instance, NULL, &cmd);
//    lcb_n1p_free(nparams);
//    if (rc != LCB_SUCCESS) {
//        fprintf(stderr, "ERROR: %s\n", lcb_strerror(instance, rc));
//        fflush(stderr);
//        return;
//    }
//    lcb_wait(instance);
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
//            printf("wohooo %d\n", lcb_get_num_nodes(*instance));
//            fflush(stdout);
            if (instance == nullptr) {
//                printf("waaaaaatts\n");
//                fflush(stdout);
            }
            printf("gahahahaha %s\n", query);
            fflush(stdout);
            n1qlQuery(query, *instance, reply_port_id);
            return;
        }
    }
    Dart_CObject result;
    result.type = Dart_CObject_kNull;
    Dart_PostCObject(reply_port_id, &result);
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


//FunctionLookup function_list[] = {
//    // { "SystemRand", SystemRand },
//    // { "SystemSrand", SystemSrand },
//    { "RandomArray_ServicePort", randomArrayServicePort },
////    { "InitEventLoop_ServicePort", initEventLoopServicePort },
//    { "ConnectToBucket_ServicePort", connectToBucketServicePort },
//    { NULL, NULL } };
//
//
//FunctionLookup no_scope_function_list[] = {
//    // { "NoScopeSystemRand", SystemRand },
//    { NULL, NULL }
//};

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

//    for (int i = 0; function_list[i].name != NULL; ++i) {
//           printf("[%s] function_list\n", function_list[i].name);
//        fflush(stdout);
//        if (strcmp(function_list[i].name, cname) == 0) {
//            *auto_setup_scope = true;
//            result = function_list[i].function;
//            break;
//        }
//    }
    
    if (function_map.find(nameStr) != function_map.end()) {
        *auto_setup_scope = true;
        result = function_map[nameStr];
    }

	if (result != NULL) {
		Dart_ExitScope();
		return result;
	}

//    for (int i = 0; no_scope_function_list[i].name != NULL; ++i) {
//        if (strcmp(no_scope_function_list[i].name, cname) == 0) {
//            *auto_setup_scope = false;
//            result = no_scope_function_list[i].function;
//            break;
//        }
//    }
    
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
    
//    function_map.insert(std::make_pair("SystemRand", SystemRand));
//    function_map.insert(std::make_pair("SystemSrand", SystemSrand));
    function_map["RandomArray_ServicePort"] = randomArrayServicePort;
//    function_map["InitEventLoop_ServicePort"] = initEventLoopServicePort;
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
