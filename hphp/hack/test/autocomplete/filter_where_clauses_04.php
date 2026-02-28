<?hh

class Thing<T> {
  public function foo<T1 as arraykey>(T1 $_):void {
    where T as T1
}

class C {
  function testit():void {
   (new Thing<?arraykey>())->AUTO332   //    T180432183: foo should not appear
  }
}
