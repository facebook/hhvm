<?hh

var_dump(array_map('get_class', array(null)));
var_dump(array_map('get_parent_class', array(null)));
try {
  var_dump(array_map('func_num_args', array(null)));
} catch (Exception $e) {
  var_dump($e->getMessage());
}
