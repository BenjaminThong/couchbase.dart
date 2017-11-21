#ifndef DART_EXT_HELPER_H
#define DART_EXT_HELPER_H

#include "include/dart_api.h"
#include "include/dart_native_api.h"

Dart_Handle HandleError(Dart_Handle handle);
void dartPostNull(Dart_Port reply_port_id);

#endif