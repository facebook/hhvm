<?hh //strict

function checkExpected((function(mixed):arraykey) $f):void { }

function test(bool $b): void {
  if ($b) {
    $f = $x ==> 3;
  } else {
    $f = $y ==> 'aa';
  }

  checkExpected($f);
}
