<?hh

<<__EntryPoint>>
function main() :mixed{
  $c = HH\Shapes::idx<>;
  $v = vec[$c];
  apc_add('whoops', $v);
  echo "done.\n";
}
