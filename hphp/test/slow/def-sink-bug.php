<?hh

<<__NEVER_INLINE>>
function foo($v) :mixed{
  $c = count($v);
  $v[] = __hhvm_intrinsics\launder_value(4);
  if ($c != 1) {
    var_dump('$c', $c);
  }
}

<<__EntryPoint>>
function main() :mixed{
  // Bias the profile towards COWing the array
  for ($i = 0; $i < 30; $i++) {
    $s_vec = vec[3];
    foo($s_vec);
  }

  // Actual test.
  $vec = vec[rand(1,2)]; // Non static vec.
  foo($vec);

  echo "Done!";
}
