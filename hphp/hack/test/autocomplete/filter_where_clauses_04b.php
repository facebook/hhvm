<?hh

class Thing<T> {
  public function foo<T1 as arraykey>(T1 $_):void {
    where T as T1
}

class C {
  public function testit(): void {
    (new Thing<int>())->AUTO332   //    foo should appear
  }
}
