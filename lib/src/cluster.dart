import 'dart:async';
import 'couchbase_extension.dart';
import 'bucket.dart';


// A class caches the native port used to call an asynchronous extension.
class Cluster {
  final host;
  final username;
  final password;

  const Cluster({this.host, this.username, this.password});

  Future<Bucket> openBucket({String name}) async {
    bool success = await cbx.connectToBucket(
      bucket: name,
      host: host,
      username: username,
      password: password,
    );
    // print('openBucket ${success}');
    if (success) {
      Bucket b = new Bucket(name: name);
      return b;
    }

    return null;
    
  }
}