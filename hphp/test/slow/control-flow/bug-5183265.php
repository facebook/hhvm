<?hh

function compare($args, $args_manual, $var_args, $var_arg_start = 0) :mixed{
  foreach ($var_args as $i => $val) {
    $n = $i + $var_arg_start;
    $args_val = $args_manual[$n];
    $args_val = $args[$n];
  }
}


<<__EntryPoint>>
function main_bug_5183265() :mixed{
var_dump(compare(vec["a", "b", "c"], vec["a", "a", "b"],
                 vec["a", "b", "c"]), 0);
var_dump(compare(vec["a", "b", "c"], vec["b", "a", "b"],
                 vec["b", "c"]), 1);

var_dump(compare(vec["a", "b", "c"], vec["a", "a", "b"],
                 vec["a", "b", "c"]), 0);
var_dump(compare(vec["a", "b", "c"], vec["b", "a", "b"],
                 vec["b", "c"]), 1);

var_dump(compare(vec["a", "b", "c"], vec["a", "a", "b"],
                 vec["a", "b", "c"]), 0);
var_dump(compare(vec["a", "b", "c"], vec["b", "a", "b"],
                 vec["b", "c"]), 1);

var_dump(compare(vec[], vec[], vec[]), 0);
var_dump(compare(vec[], vec[], vec[]), 1);

var_dump(compare(vec[], vec[], vec[]), 0);
var_dump(compare(vec[], vec[], vec[]), 1);
}
