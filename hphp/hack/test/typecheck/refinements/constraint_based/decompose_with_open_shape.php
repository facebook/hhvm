<?hh
<<file: __EnableUnstableFeatures('case_types')>>

case type CT = string | shape('x' => int, 'y' => string);

function decompose(CT $x): void {
  if ($x is shape(...)) {
    hh_expect<shape('x' => int, 'y' => string)>($x);
  } else {
    hh_expect<string>($x);
  }
}
