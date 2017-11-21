#ifndef CLUSTER_H
#define CLUSTER_H

#include <stdlib.h>
#include "include/dart_api.h"
#include "include/dart_native_api.h"

bool connectToBucket(char* connstr, char* username, char* password);
void wrappedConnectToBucket(Dart_Port dest_port_id,
	Dart_CObject* message);
void connectToBucketServicePort(Dart_NativeArguments arguments);

#endif