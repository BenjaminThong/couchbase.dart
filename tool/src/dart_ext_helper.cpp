#include "dart_ext_helper.h"


Dart_Handle HandleError(Dart_Handle handle) {
 if (Dart_IsError(handle)) Dart_PropagateError(handle);
 return handle;
}

void dartPostNull(Dart_Port reply_port_id) {
	Dart_CObject result;
	result.type = Dart_CObject_kNull;
	Dart_PostCObject(reply_port_id, &result);
}