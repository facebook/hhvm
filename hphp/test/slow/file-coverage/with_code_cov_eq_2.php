<?hh

function a() : void {
  echo "a\n";
}

<<__EntryPoint>>
function main() :mixed{
  HH\enable_per_file_coverage(keyset[__FILE__]);
  a();
  var_dump(HH\get_coverage_for_file(__FILE__));
  HH\disable_all_coverage();
}
