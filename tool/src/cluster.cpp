#include <string>
#include "cluster.h"
#include "dart_ext_helper.h"


lcb_io_opt_t ioops;

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
