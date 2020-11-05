<?hh // strict

// This file is same as check/while.php except written in for loop syntax.
// The results should be the same.

class C {
  <<__Policied("PRIVATE")>>
  public string $pri = "";

  <<__Policied("PRIVATE")>>
  public bool $prib = true;

  <<__Policied("PUBLIC")>>
  public string $pub = "";

  <<__Policied("PUBLIC")>>
  public arraykey $out = 42;
}

<<__InferFlows>>
function simple(bool $b, C $c): void {
  $x = 42;
  for (;$b;) {
    $x = $c->pri;
    $x = $c->pub;
  }
  // fine because $x can only be public
  $c->out = $x;
}

<<__InferFlows>>
function breaks(bool $b, C $c): void {
  $x = 42;
  for (;$b;) {
    $x = $c->pri;
    if ($b) break;
    $x = $c->pub;
  }
  // this is an error, $x can be either
  // private or public depending on the break
  $c->out = $x;
}

<<__InferFlows>>
function continues(bool $b, C $c): void {
  $x = 42;
  for (;$b;) {
    $x = $c->pri;
    if ($b) continue;
    $x = $c->pub;
  }
  // this is an error, for reasons similar to
  // the breaks() function above
  $c->out = $x;
}

<<__InferFlows>>
function pcleak(C $c): void {
  $n = 0;
  for (;$c->prib;) {
    $n = 1;
    $c->prib = false;
  }
  // this is an error, some information about
  // the boolean $c->prib can be leaked
  $c->out = $n;
}

<<__InferFlows>>
function niftyleak(bool $b, C $c): void {
  $x = 42;
  $y = 24;
  $z = 12;
  // look ma no fixpoint in ifc.ml!
  for (;$b;) {
    $c->out = $z;
    $z = $x;
    $x = $y;
    $y = $c->pri;
  }
}
