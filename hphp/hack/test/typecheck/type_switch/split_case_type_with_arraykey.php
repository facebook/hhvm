<?hh

<<file: __EnableUnstableFeatures('case_types')>>

case type CT = arraykey | float;

function foo(CT $x): void {
  if ($x is arraykey) {
  }
}
