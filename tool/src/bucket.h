#ifndef BUCKET_H
#define BUCKET_H


#include <libcouchbase/couchbase.h>
#include <libcouchbase/n1ql.h>
#include "include/dart_api.h"
#include "include/dart_native_api.h"

void rowCallback(lcb_t instance, int cbtype, const lcb_RESPN1QL *resp);
void n1qlQuery(char* querystr, lcb_t instance, Dart_Port reply_port_id);
void wrappedN1QLQuery(Dart_Port dest_port_id,
                            Dart_CObject* message);
void connectToN1QLQueryServicePort(Dart_NativeArguments arguments);

#endif
