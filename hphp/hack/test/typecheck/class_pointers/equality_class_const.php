<?hh
<<file:__EnableUnstableFeatures('case_types')>>

class C1 {}
interface I {}
enum ES: string {
  A = 'a';
}
enum EI: int as int {
  A = 1;
}
case type CT = class<C1> | int;

function compared_with_strings(
  string $s,
  classname<C1> $cn,
  mixed $m,
  vec<string> $v,
  shape('c' => string) $sh,
  (string, int) $t,
  dict<string, string> $d,
): void {
  if ($s === C1::class) {
  }
  if ($cn !== C1::class) {
  }
  if (C1::class == $m) {
  }
  if ($s != C1::class) {
  }
  if ($v === vec[C1::class]) {
  }
  if ($sh === shape('c' => C1::class)) {
  }
  if ($t === tuple(C1::class, 1)) {
  }
  if ($d === dict['k' => C1::class]) {
  }
  switch ($cn) {
    case C1::class:
      break;
    default:
      break;
  }
}

function compared_with_types_that_may_hold_strings(
  ?string $ns,
  arraykey $ak,
  Stringish $sish,
  ~string $ls,
  ES $es,
  vec<vec<string>> $vv,
  (class<C1>, string) $cs,
): void {
  if ($ns === C1::class) {
  }
  if ($ak === C1::class) {
  }
  if ($sish === C1::class) {
  }
  if ($ls === C1::class) {
  }
  if ($es === C1::class) {
  }
  if ($vv === vec[vec[C1::class]]) {
  }
  // Literal elements are checked against the operand as a whole
  if ($cs === tuple(C1::class, 'x')) {
  }
}

// Abstract types are checked through their bounds; a case type without an
// `as` bound is bounded by mixed
function compared_with_abstract_types<TS as string, TU, TR as vec<TR>>(
  TS $ts,
  TU $tu,
  TR $tr,
  CT $ct,
): void {
  if ($ts === C1::class) {
  }
  if ($tu === C1::class) {
  }
  if ($tr === vec[C1::class]) {
  }
  if ($ct === C1::class) {
  }
}

function compared_with_class_pointers(
  string $s,
  class<C1> $c,
  ~class<C1> $lc,
  vec<class<C1>> $vc,
  dict<string, class<C1>> $dc,
  shape('c' => class<C1>) $sc,
): void {
  if ($c === C1::class) {
  }
  if ($lc === C1::class) {
  }
  if ($vc === vec[C1::class]) {
  }
  if ($dc === dict['k' => C1::class]) {
  }
  if ($sc === shape('c' => C1::class)) {
  }
  if ($s === nameof C1) {
  }
  switch ($c) {
    case C1::class:
      break;
    default:
      break;
  }
}

function compared_with_non_strings<TC as class<C1>, TB as Stringish as I>(
  I $i,
  EI $ei,
  TC $tc,
  TB $tb,
): void {
  if ($i === C1::class) {
  }
  if ($ei === C1::class) {
  }
  if ($tc === C1::class) {
  }
  if ($tb === C1::class) {
  }
}

function reified<reify T as C1>(string $s): bool {
  return $s === T::class;
}

class K {
  public static function is_me(string $s): bool {
    return $s === static::class;
  }
}
