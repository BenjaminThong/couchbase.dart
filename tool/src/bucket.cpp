#include "bucket.h"
#include "dart_ext_helper.h"

/**************************
 *  N1QL Query
 **************************/
void rowCallback(lcb_t instance, int cbtype, const lcb_RESPN1QL *resp) {
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