import 'dart:async';
import 'package:test/test.dart';
import 'package:couchbase/couchbase.dart';

void main() {
  group('Bucket', () {
    test('Can upsert to and get from default', () async {
      var c = new Cluster(
        host: 'couchbase://127.0.0.1',
        password: 'password',
        username: 'root',
        );
      var b = await c.openBucket(bucket: 'default');
      expect(b, isNot(null));
      var res = await b.n1qlQuery(query: 'SELECT * FROM `default` USE KEYS ["fsfs", "ahah"]');
    });
  });
}