<?hh

function reflect($name) :mixed{
  echo '==== ', $name, ' ====', "\n";
  $rf = new ReflectionFunction($name);
  var_dump($rf->getReturnTypeText());
  $params = $rf->getParameters();
  var_dump(count($params));
}

function main() :mixed{
  reflect('hash_algos');
}

<<__EntryPoint>>
function main_hni_return_type() :mixed{
main();
}
