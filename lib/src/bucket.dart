import 'dart:async';
import 'dart:isolate';
import 'couchbase_extension.dart';
import 'cluster.dart';

class Bucket {
  final String name;
  final Cluster cluster;
  SendPort _n1qlQueryPort;

  SendPort get _n1qlQueryServicePort {
    if (_n1qlQueryPort == null) {
      _n1qlQueryPort = cbx.newN1QLQueryServicePort();
    }
    return _n1qlQueryPort;
  }

  String get _connectionKey => '${cluster.host}/${name}/${cluster.username}/${cluster.password}';
  
  Bucket({this.name, this.cluster});

  Stream<String> n1qlQuery({String query}) {
    StreamController<String> controller = new StreamController<String>();
    var replyPort = new RawReceivePort();
    var args = new List(3);
    args[0] = query;
    args[1] = _connectionKey;
    args[2] = replyPort.sendPort;
    _n1qlQueryServicePort.send(args);
    replyPort.handler = (result) {
      if (result != null) {
        controller.add(result);
        // if (result == 'end') {
        //   replyPort.close();
        // } else {
        //   // completer.complete(result);
        // }
      } else {
        replyPort.close();
        controller.close();
        // completer.completeError(new Exception("Random array creation failed"));
      }
    };
    return controller.stream;
  }
}