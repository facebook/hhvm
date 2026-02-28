<?hh
<<file: __EnableUnstableFeatures('case_types')>>

function foo(mixed $x): (int, string) {
  if ($x is (int, string)) {
    return $x;
  }
  return tuple(0, '');
}

case type CT = string | (int, string);

function bar<T as CT>(T $x): void {
  if ($x is (int, string)) {
    hh_expect<(int, string)>($x);
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
    hh_expect<(int, string)>($x);
    hh_expect<T>($x);
  }
}
