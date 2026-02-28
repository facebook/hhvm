<?hh

<<__EntryPoint>>
function main() :mixed{
  $config = simplexml_load_string('<config><prepare /></config>');
  $config2 = simplexml_load_string('<config><prepare2 /></config>');

  // bool, int, double cast
  var_dump((bool)$config);
  var_dump((int)$config);
  var_dump((float)$config);

  // iterator
  foreach ($config as $c) {
    var_dump($c);
  }

  // equality
  var_dump($config == $config2);
}
