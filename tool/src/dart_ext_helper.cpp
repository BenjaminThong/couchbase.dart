#include "dart_ext_helper.h"


std::unordered_map<std::string, lcb_t> couchbase_connection_map;

Dart_Handle HandleError(Dart_Handle handle) {
 if (Dart_IsError(handle)) Dart_PropagateError(handle);
 return handle;
}

void dartPostNull(Dart_Port reply_port_id) {
	Dart_CObject result;
	result.type = Dart_CObject_kNull;
	Dart_PostCObject(reply_port_id, &result);
}

lcb_t* getInstance(std::string key) {
    if (couchbase_connection_map.find(key) == couchbase_connection_map.end()) {
        return nullptr;
    }
    return &couchbase_connection_map[key];
}