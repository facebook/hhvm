<?hh

function zoned_with()[zoned_with]: void {
  echo "in zoned_with\n";
}

<<__EntryPoint>>
function main() :mixed{
  zoned_with();
  HH\Coeffects\_Private\enter_zoned_with(zoned_with<>);
}
