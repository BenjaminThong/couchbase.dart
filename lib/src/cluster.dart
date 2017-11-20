import 'dart:async';
import 'dart:isolate';
import 'couchbase_extension.dart';
import 'bucket.dart';


// A class caches the native port used to call an asynchronous extension.
class Cluster {
  static SendPort _connectToBucketPort;
  final host;
  final username;
  final password;

  const Cluster({
    this.host = 'couchbase://localhost',
    this.username = 'root',
    this.password = 'password',
    });

  SendPort get _connectToBucketServicePort {
    if (_connectToBucketPort == null) {
      _connectToBucketPort = cbx.newConnectToBucketServicePort();
    }
    return _connectToBucketPort;
  }

  String get key => '${host}${username}${password}';

  Future<Bucket> openBucket({String bucket}) async {
    var completer = new Completer();
    var replyPort = new RawReceivePort();
    var args = new List(4);
    args[0] = '${host}/${bucket}';
    args[1] = username;
    args[2] = password;
    args[3] = replyPort.sendPort;
    _connectToBucketServicePort.send(args);
    replyPort.handler = (result) {
      replyPort.close();
      if (result) {
        completer.complete(new Bucket(
          name: bucket,
          cluster: this,
          ));
      } else {
        completer.completeError(new Exception('Connect to ${host}${bucket} failed'));
      }
    };
    return completer.future;
  }
}