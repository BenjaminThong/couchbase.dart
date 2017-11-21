#ifndef DART_EXT_HELPER_H
#define DART_EXT_HELPER_H


#include <string>
#include <unordered_map>
#include <libcouchbase/couchbase.h>
#include "include/dart_api.h"
#include "include/dart_native_api.h"

extern std::unordered_map<std::string, lcb_t> couchbase_connection_map;

Dart_Handle HandleError(Dart_Handle handle);
void dartPostNull(Dart_Port reply_port_id);
lcb_t* getInstance(std::string key);


#endif