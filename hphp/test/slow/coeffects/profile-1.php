<?hh

<<__NEVER_INLINE>>
function run(mixed $f)[ctx $f] :mixed{ echo "in f\n"; }

function write_props()[write_props] :mixed{}

<<__EntryPoint>>
function main() :mixed{
  $x = vec[
    tuple(null, 1),
    tuple(write_props<>, 3),
    tuple(()[zoned] ==> 1, 2),
  ];
  foreach ($x as $e) {
    list($v, $count) = $e;
    for ($i = 0; $i < $count; ++$i) {
      run(__hhvm_intrinsics\launder_value($v));
    }
  }
}
