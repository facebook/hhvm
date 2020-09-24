<?hh // strict

class C {
  <<Policied("PRIVATE")>>
  public string $pri = "";

  <<Policied("PRIVATE")>>
  public bool $prib = true;

  <<Policied("PUBLIC")>>
  public string $pub = "";

  <<Policied("PUBLIC")>>
  public arraykey $out = 42;
}

function simple(bool $b, C $c): void {
  $x = 42;
  while ($b) {
    $x = $c->pri;
    $x = $c->pub;
  }
  // fine because $x can only be public
  $c->out = $x;
}

function breaks(bool $b, C $c): void {
  $x = 42;
  while ($b) {
    $x = $c->pri;
    if ($b) break;
    $x = $c->pub;
  }
  // this is an error, $x can be either
  // private or public depending on the break
  $c->out = $x;
}

function continues(bool $b, C $c): void {
  $x = 42;
  while ($b) {
    $x = $c->pri;
    if ($b) continue;
    $x = $c->pub;
  }
  // this is an error, for reasons similar to
  // the breaks() function above
  $c->out = $x;
}

function pcleak(C $c): void {
  $n = 0;
  while ($c->prib) {
    $n = 1;
    $c->prib = false;
  }
  // this is an error, some information about
  // the boolean $c->prib can be leaked
  $c->out = $n;
}

function niftyleak(bool $b, C $c): void {
  $x = 42;
  $y = 24;
  $z = 12;
  // look ma no fixpoint in ifc.ml!
  while ($b) {
    $c->out = $z;
    $z = $x;
    $x = $y;
    $y = $c->pri;
  }
}
