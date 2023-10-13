<?hh

class Test<T> {}

function test() : void {
  new Test<Test>(); //bad, missing type arg on inner Test
}
