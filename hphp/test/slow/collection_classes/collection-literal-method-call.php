<?hh

<<__EntryPoint>>
function main_collection_literal_method_call() :mixed{
$c = (Vector {'a', 'b'})->addAll(Vector {'c', 'd'});
foreach ($c as $v) {
  echo $v . "\n";
}
}
