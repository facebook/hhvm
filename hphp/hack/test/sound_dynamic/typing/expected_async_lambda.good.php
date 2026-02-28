<?hh

// Accepted
function test1():void {
  $d = makeDynamic();
  $f = async function($x) { return $x; };
  $d->foo($f);
}

function makeDynamic():dynamic {
  return null;
}

// Rejected
function test2():void {
  $d = makeDynamic();
  $d->foo(async function($x) { return $x; });
  $d->foo(function($x) { return $x; });
  $d->foo(async function($x) { return 3; });
  $d->foo(function($x) { return 3; });
}
