<?hh
<<file: __EnableUnstableFeatures("readonly")>>
class Foo {
  public function __construct(public int $prop = 4) {}
}


function test_while(bool $b): void {
  $x = readonly new Foo();
  while ($b) {
    $x = new Foo();
  }
  $x->prop = 5; // error, $x could be readonly
}
function test_for(bool $b): void {
  $y = readonly new Foo();
  for ($i = 0; $i < 5; $i++) {
    $y = new Foo();
  }
  $y->prop = 5; // error, $y could be readonly
}

function test_foreach(bool $b, vec<int> $vec): void {
  $z = readonly new Foo();
  foreach ($vec as $_) {
    $z = new Foo();
  }
  $z->prop = 5; // error, $z could be readonly
}

function test_do_while(bool $b): void {
  $w = readonly new Foo();
  do {
    $w = new Foo();
  } while ($b);
  $w->prop = 5; // ok, $w is now mutable
}

function test_try(bool $b): void {
  $l = readonly new Foo();
  try {
    $l = new Foo();
  } catch (Exception $e) {
    $l = new Foo();
  }
  $l->prop = 4; // ok, $l is mutable in all cases

  $v = readonly new Foo();
  try {
    $v = new Foo();
  } catch (Exception $e) {
  }
  $v->prop = 2; // error, $v could be readonly

  $v = readonly new Foo();
  try {
  } catch (Exception $e) {
    $v = new Foo();
  }
  $v->prop = 2; // error, $v could be readonly
}

function test_finally(bool $b): void {
  $l = readonly new Foo();
  try {
    $l = new Foo();
  } catch (Exception $e) {
  } finally {
    // error, $l could be readonly here if constructor throws an exception
    $l->prop = 4;
    $l = new Foo();
  }
  $l->prop = 4; // ok, $l is no longer readonly here

}

function test_flow(bool $b): void {
  if ($b) {
    $x = readonly new Foo();
  } else {
    return;
  }
  // error, $x is readonly here
  $x->prop = 6;

}

function test_advanced(bool $b): void {
  $x = new Foo();
  $y = new Foo();
  $z = new Foo();
  while ($b) {
    $w = $x;
    $w->prop = 4; // error if loop runs 3 times
    $x = $y;
    $y = $z;
    $z = readonly new Foo();
  }
  $z->prop = 4; // obviously error
  $y->prop = 4; // error if loop runs once
  $x->prop = 4; // error if loop runs twice
}
