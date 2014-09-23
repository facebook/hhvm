<?hh

function compare($args, $args_manual, $var_args, $var_arg_start = 0) {
  foreach ($var_args as $i => $val) {
    $n = $i + $var_arg_start;
    $args_val = $args_manual[$n];
    $args_val = $args[$n];
  }
}

var_dump(compare(array("a", "b", "c"), array("a", "a", "b"),
                 array("a", "b", "c")), 0);
var_dump(compare(array("a", "b", "c"), array("b", "a", "b"),
                 array("b", "c")), 1);

var_dump(compare(array("a", "b", "c"), array("a", "a", "b"),
                 array("a", "b", "c")), 0);
var_dump(compare(array("a", "b", "c"), array("b", "a", "b"),
                 array("b", "c")), 1);

var_dump(compare(array("a", "b", "c"), array("a", "a", "b"),
                 array("a", "b", "c")), 0);
var_dump(compare(array("a", "b", "c"), array("b", "a", "b"),
                 array("b", "c")), 1);

var_dump(compare(array(), array(), array()), 0);
var_dump(compare(array(), array(), array()), 1);

var_dump(compare(array(), array(), array()), 0);
var_dump(compare(array(), array(), array()), 1);
