<?hh

<<__EntryPoint>>
function main() {
  $config = simplexml_load_string('<config><prepare /></config>');
  $config2 = simplexml_load_string('<config><prepare2 /></config>');

  // bool, array, int, double cast
  var_dump((bool)$config);
  var_dump((array)$config);
  var_dump((int)$config);
  var_dump((double)$config);

  // empty()
  var_dump(empty($config->prepare));
  var_dump(empty($config->does_not_exist));

  // iterator
  foreach ($config as $c) {
    var_dump($c);
  }

  // comparisons
  var_dump($config < $config2);
  var_dump($config > $config2);
  var_dump($config <= $config2);
  var_dump($config >= $config2);

  // equality
  var_dump($config == $config2);
}
