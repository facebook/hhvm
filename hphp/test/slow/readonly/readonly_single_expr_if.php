<?hh
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
}
