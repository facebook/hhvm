<?hh

enum MyEnum: string as string {}

function test(arraykey $x): void {
  if ($x is MyEnum) {
    //
  } else {
    hh_show($x); // arraykey & not MyEnum
    if ($x is int) {
      hh_show($x); // int & not MyEnum
    } else {
      hh_show($x); // string & not MyEnum
    }
  }
}
