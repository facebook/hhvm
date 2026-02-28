<?hh
<<file: __EnableUnstableFeatures('case_types')>>

function foo(mixed $x): shape('x' => int, 'y' => string) {
  if ($x is shape('x' => int, 'y' => string)) {
    return $x;
  }
  return shape('x' => 0, 'y' => '');
}

case type CT = string | shape('x' => int, 'y' => string);

function bar<T as CT>(T $x): void {
  if ($x is shape('x' => int, 'y' => string)) {
    hh_expect<shape('x' => int, 'y' => string)>($x);
    hh_expect<T>($x);
  } else {
    hh_expect<string>($x);
    hh_expect<T>($x);
  }
}

function bar_reverse_field_order<T as CT>(T $x): void {
  if ($x is shape('x' => int, 'y' => string)) {
    hh_expect<shape('y' => string, 'x' => int)>($x);
    hh_expect<T>($x);
  } else {
    hh_expect<string>($x);
    hh_expect<T>($x);
  }
}

function baz<T as CT>(T $x): void {
  if ($x is string) {
    hh_expect<string>($x);
    hh_expect<T>($x);
  } else {
    hh_expect<shape('x' => int, 'y' => string)>($x);
    hh_expect<T>($x);
  }
}
