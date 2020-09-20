<?hh // partial

function fakeClassGet() : (int, int) {
  return BogusClass::bogusprop;
}

function useFakeGlobalConst() : (int, int) {
  $y = BogusConst;
  return $y;
}
class Foo {
}
function fakeObjGet(Foo $x) : (int, int) {

 /* HH_FIXME[4053] */
 $y =  $x->something;
 return $y;
}

function badCall() : (int, int) {
  $y = 5();
  return $y;
}

function testLambdaParam() : void {
  $y = function ($x) {
    return $x;
  };

}
