#ifndef RANDOM_ARRAY_H
#define RANDOM_ARRAY_H

#include <stdlib.h>
#include "include/dart_api.h"
#include "include/dart_native_api.h"

uint8_t* randomArray(int seed, int length);
void wrappedRandomArray(Dart_Port dest_port_id,
	Dart_CObject* message);
void randomArrayServicePort(Dart_NativeArguments arguments);

#endif
