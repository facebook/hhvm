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
var_dump(compare(varray["a", "b", "c"], varray["a", "a", "b"],
                 varray["a", "b", "c"]), 0);
var_dump(compare(varray["a", "b", "c"], varray["b", "a", "b"],
                 varray["b", "c"]), 1);

var_dump(compare(varray["a", "b", "c"], varray["a", "a", "b"],
                 varray["a", "b", "c"]), 0);
var_dump(compare(varray["a", "b", "c"], varray["b", "a", "b"],
                 varray["b", "c"]), 1);

var_dump(compare(varray["a", "b", "c"], varray["a", "a", "b"],
                 varray["a", "b", "c"]), 0);
var_dump(compare(varray["a", "b", "c"], varray["b", "a", "b"],
                 varray["b", "c"]), 1);

var_dump(compare(varray[], varray[], varray[]), 0);
var_dump(compare(varray[], varray[], varray[]), 1);

var_dump(compare(varray[], varray[], varray[]), 0);
var_dump(compare(varray[], varray[], varray[]), 1);
}
