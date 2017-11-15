import "package:test/test.dart";
import "package:couchbase/couchbase.dart";

void main() {
  group("Random Array", () {
    test("Seeded random generates same values", () async {
      RandomArray r = new RandomArray();
      List<int> list_100 = await r.randomArray(17, 100);
      List<int> list_200 = await r.randomArray(17, 200);
      for (var i = 0; i < 100; ++i) {
        expect(list_100[i], list_200[i]);
      }
    });
  });
}