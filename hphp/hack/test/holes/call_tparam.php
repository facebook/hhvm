<?hh

class Foo {
public function a((int, int) $a): void {}
public function b(Pair<int, int> $b): void {}
public function c(varray<int> $c): void {}
public function d(varray_or_darray<int> $d): void {}
public function e(vec<int> $e): void {}
public function f(keyset<int> $f): void {}
public function g(Vector<int> $g): void {}
public function h(Set<int> $h): void {}
public function i(Traversable<int> $i): void {}
public function j(ImmSet<int> $j): void {}
public function k(ImmVector<int> $k): void {}
public function l(Collection<int> $l): void {}
public function m(Iterable<int> $m): void {}
public function n(dict<int, int> $n): void {}
public function o(KeyedTraversable<int, int> $o): void {}
}

function a((int, int) $a): void {}
function b(Pair<int, int> $b): void {}
function c(varray<int> $c): void {}
function d(varray_or_darray<int> $d): void {}
function e(vec<int> $e): void {}
function f(keyset<int> $f): void {}
function g(Vector<int> $g): void {}
function h(Set<int> $h): void {}
function i(Traversable<int> $i): void {}
function j(ImmSet<int> $j): void {}
function k(ImmVector<int> $k): void {}
function l(Collection<int> $l): void {}
function m(Iterable<int> $m): void {}
function n(dict<int, int> $n): void {}
function o(KeyedTraversable<int, int> $o): void {}

function call_tparam(
  (int, bool) $a1,
  (bool, int) $a2,
  Pair<bool, int> $b1,
  Pair<int, bool> $b2,
  varray<float> $c,
  varray_or_darray<float> $d,
  vec<float> $e,
  keyset<string> $f,
  Vector<float> $g,
  Set<string> $h,
  Traversable<float> $i,
  ImmSet<string> $j,
  ImmVector<float> $k,
  Collection<string> $l,
  Iterable<float> $m,
  dict<string, int> $n1,
  dict<int, string> $n2,
  KeyedTraversable<string, int> $o1,
  KeyedTraversable<int, string> $o2,
): void {
/* HH_FIXME[4110] */
a($a1);
/* HH_FIXME[4110] */
a($a2);
/* HH_FIXME[4110] */
b($b1);
/* HH_FIXME[4110] */
b($b2);
/* HH_FIXME[4110] */
c($c);
/* HH_FIXME[4110] */
d($d);
/* HH_FIXME[4110] */
e($e);
/* HH_FIXME[4110] */
f($f);
/* HH_FIXME[4110] */
g($g);
/* HH_FIXME[4110] */
h($h);
/* HH_FIXME[4110] */
i($i);
/* HH_FIXME[4110] */
j($j);
/* HH_FIXME[4110] */
k($k);
/* HH_FIXME[4110] */
l($l);
/* HH_FIXME[4110] */
m($m);
/* HH_FIXME[4110] */
n($n1);
/* HH_FIXME[4110] */
n($n2);
/* HH_FIXME[4110] */
o($o1);
/* HH_FIXME[4110] */
o($o2);

$foo = new Foo();

/* HH_FIXME[4110] */
$foo->a($a1);
/* HH_FIXME[4110] */
$foo->a($a2);
/* HH_FIXME[4110] */
$foo->b($b1);
/* HH_FIXME[4110] */
$foo->b($b2);
/* HH_FIXME[4110] */
$foo->c($c);
/* HH_FIXME[4110] */
$foo->d($d);
/* HH_FIXME[4110] */
$foo->e($e);
/* HH_FIXME[4110] */
$foo->f($f);
/* HH_FIXME[4110] */
$foo->g($g);
/* HH_FIXME[4110] */
$foo->h($h);
/* HH_FIXME[4110] */
$foo->i($i);
/* HH_FIXME[4110] */
$foo->j($j);
/* HH_FIXME[4110] */
$foo->k($k);
/* HH_FIXME[4110] */
$foo->l($l);
/* HH_FIXME[4110] */
$foo->m($m);
/* HH_FIXME[4110] */
$foo->n($n1);
/* HH_FIXME[4110] */
$foo->n($n2);
/* HH_FIXME[4110] */
$foo->o($o1);
/* HH_FIXME[4110] */
$foo->o($o2);
}
