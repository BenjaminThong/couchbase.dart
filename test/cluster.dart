import 'package:test/test.dart';
import 'package:couchbase/couchbase.dart';

void main() {
  group('Cluster', () {
    test('Can connect to "couchbase://127.0.0.1/default" with "root" and "password"', () async {
      var c = new Cluster(
        host: 'couchbase://127.0.0.1',
        password: 'password',
        username: 'root',
        );
      var b = await c.openBucket(bucket: 'default');
      expect(b, isNot(null));
    });
  });
}