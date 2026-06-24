<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

class A {
  public function f(named int $x = 0, named int $y = 1)[write_props] {
    $z = vec[$x, $y];
    return $z;
  }
}

final class ParamCoeffect {
  public function render(
    named ?string $placeholder = null,
    named bool $write_unions = false,
    named ?(function(string)[_]: string) $post_process = null,
  )[ctx $post_process]: string {
    return $placeholder ?? 'null';
  }
}

// Cartesian product of (named vs unnamed $a) x (named vs unnamed $b), where the
// coeffect context parameter $a is a closure/null and the other parameter $b is
// a bool. Each case checks that Coeffect::from_ast picks the right parameter
// index after named-parameter reordering (named params, sorted lexicographically
// by name, precede positional params). If the index were computed from source
// order, the `[ctx $a]` rule would resolve to the bool $b and the call would
// throw "Coeffect rule requires parameter ... but bool given". The returned
// 'T'/'F' also confirms $b is still bound to the right value after reordering.

// 1. $a unnamed, $b unnamed
function both_positional(
  ?(function()[_]: void) $a,
  bool $b,
)[ctx $a]: string {
  return $b ? 'T' : 'F';
}

// 2. $a unnamed, $b named
function a_positional_b_named(
  ?(function()[_]: void) $a,
  named bool $b = false,
)[ctx $a]: string {
  return $b ? 'T' : 'F';
}

// 3. $a named, $b unnamed
function a_named_b_positional(
  bool $b,
  named ?(function()[_]: void) $a = null,
)[ctx $a]: string {
  return $b ? 'T' : 'F';
}

// 4. $a named, $b named
function both_named(
  named ?(function()[_]: void) $a = null,
  named bool $b = false,
)[ctx $a]: string {
  return $b ? 'T' : 'F';
}

// 5. $a named, $b named, but $b is declared before $a
function both_named_b_first(
  named bool $b = false,
  named ?(function()[_]: void) $a = null,
)[ctx $a]: string {
  return $b ? 'T' : 'F';
}

<<__EntryPoint>>
function main() :mixed{
  $a = new A();
  var_dump($a->f());
  var_dump($a->f(x=100));
  var_dump($a->f(y=200));
  var_dump($a->f(x=100, y=200));

  // post_process is omitted, so it should default to null (pure context):
  var_dump((new ParamCoeffect())->render(placeholder='x', write_unions=true));

  // Cartesian product. Pass null/closure for the coeffect param $a and a real
  // bool for $b.
  $closure = ()[] ==> {};
  var_dump(both_positional(null, true));
  var_dump(both_positional($closure, false));
  var_dump(a_positional_b_named(null, b=true));
  var_dump(a_positional_b_named($closure, b=false));
  var_dump(a_named_b_positional(true, a=null));
  var_dump(a_named_b_positional(false, a=$closure));
  var_dump(both_named(a=null, b=true));
  var_dump(both_named(a=$closure, b=false));
  var_dump(both_named_b_first(a=null, b=true));
  var_dump(both_named_b_first(a=$closure, b=false));
}
