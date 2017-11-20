import 'dart:async';
import 'dart:isolate';
import 'dart-ext:couchbase_extension';

final CouchbaseExtension cbx = new CouchbaseExtension._private();

// A class caches the native port used to call an asynchronous extension.
class CouchbaseExtension {
  static SendPort _randomArrayPort;
  // static SendPort _initEventLoopPort;

  CouchbaseExtension._private() {}

  Future<List<int>> randomArray(int seed, int length) {
    var completer = new Completer();
    var replyPort = new RawReceivePort();
    var args = new List(3);
    args[0] = seed;
    args[1] = length;
    args[2] = replyPort.sendPort;
    _randomArrayServicePort.send(args);
    replyPort.handler = (result) {
      replyPort.close();
      if (result != null) {
        completer.complete(result);
      } else {
        completer.completeError(new Exception("Random array creation failed"));
      }
    };
    return completer.future;
  }

  // Future<bool> initEventLoop() {
  //   var completer = new Completer();
  //   var replyPort = new RawReceivePort();
  //   var args = new List(1);
  //   args[0] = replyPort.sendPort;
  //   _initEventLoopServicePort.send(args);
  //   replyPort.handler = (result) {
  //     replyPort.close();
  //     if (result != null) {
  //       completer.complete(result);
  //     } else {
  //       completer.completeError(new Exception('Initializing Event Loop failed'));
  //     }
  //   };
  //   return completer.future;
  // }

  SendPort get _randomArrayServicePort {
    if (_randomArrayPort == null) {
      _randomArrayPort = _newRandomArrayServicePort();
    }
    return _randomArrayPort;
  }

  // SendPort get _initEventLoopServicePort {
  //   if (_initEventLoopPort == null) {
  //     _initEventLoopPort = _newInitEventLoopServicePort();
  //   }
  //   return _initEventLoopPort;
  // }

  SendPort _newRandomArrayServicePort() native 'RandomArray_ServicePort';
  // SendPort _newInitEventLoopServicePort() native 'InitEventLoop_ServicePort';
  SendPort newConnectToBucketServicePort() native 'ConnectToBucket_ServicePort';
  SendPort newN1QLQueryServicePort() native 'N1QLQuery_ServicePort';
}