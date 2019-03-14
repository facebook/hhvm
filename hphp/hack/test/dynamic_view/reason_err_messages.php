<?hh // partial

function fakeClassGet() : int {
  return BogusClass::bogusprop;
}

function useFakeGlobalConst() : int {
  $y = BogusConst;
  return $y;
}
class Foo {
}
function fakeObjGet(Foo $x) : int {

 /* HH_FIXME[4053] */
 $y =  $x->something;
 return $y;
}

function badCall() : int {
  $y = 5();
  return $y;
}

function testLambdaParam() : void {
  $y = function ($x) {
    return $x;
  };

}
