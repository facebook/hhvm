<?hh // strict
<<file: __EnableUnstableFeatures('readonly')>>

function is_empty<T>(T $container)[]: bool {
  return true;
}

function return_vec(mixed $x): vec<int> {
  return vec[0];
}

<<__EntryPoint>>
function test(): void {

  $x = readonly vec[];
  if (is_empty($x)) {
    echo "Hi";
  }
  while (is_empty($x)) {
  }
  do {

  } while (is_empty($x));
  foreach (return_vec($x) as $y) {
  }


}
