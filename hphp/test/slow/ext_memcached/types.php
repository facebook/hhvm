<?hh
<<__EntryPoint>> function main(): void {
$list = dict[
  'boolean_true' => true,
  'boolean_false' => false,
  'string' => "just a string",
  'string_empty' => '',
  'integer_positive_integer' => 10,
  'integer_negative_integer' => -10,
  'integer_zero_integer' => 0,
  'float_positive1' => 3.912131,
  'float_positive2' => 1.2131E+52,
  'float_negative' => -42.123312,
  'float_zero' => 0.0,
  'null' => null,
  'array_empty' => vec[],
  'array' => vec[1, 2, 3, "foo"],
];

$memc = new Memcached();
$memc->addServer('localhost', '11211');

foreach($list as $key => $value) {
  var_dump($memc->set($key, $value, 60));
  var_dump($memc->get($key));
}

foreach($list as $key => $value) {
  var_dump($memc->delete($key));
}

var_dump($memc->setMulti($list, 60));

$res = $memc->getMulti(array_keys($list));
var_dump(is_array($res));
var_dump(count($res));

foreach($res as $key => $value) {
  var_dump($value == $list[$key]);
}
}
