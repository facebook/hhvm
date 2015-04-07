<?hh

// --------------------------------
// Arrays
// --------------------------------

$a = array('x' => 10, 'y' => 20);

array_filter($a, ($v, $k) ==> {
  var_dump($k);
  var_dump($v);
}, ARRAY_FILTER_USE_BOTH);

array_filter($a, $k ==> {
  var_dump($k);
}, ARRAY_FILTER_USE_KEY);

array_filter($a, $v ==> {
  var_dump($v);
}); // default to value

array_filter($a, $v ==> {
  var_dump($v);
}, 3); // unknown use is defaulted to value

array_filter($a, $k ==> {}, ARRAY_FILTER_USE_KEY, 5); // too many params

// --------------------------------
// KeyedTraversable
// --------------------------------

$m = Map {'x' => 10, 'y' => 20};
var_dump($m instanceof KeyedTraversable);

array_filter($m, ($v, $k) ==> {
  var_dump($k);
  var_dump($v);
}, ARRAY_FILTER_USE_BOTH);

array_filter($m, $k ==> {
  var_dump($k);
}, ARRAY_FILTER_USE_KEY);

array_filter($m, $v ==> {
  var_dump($v);
}); // default to value

array_filter($m, $v ==> {
  var_dump($v);
}, 3); // unknown use is defaulted to value
