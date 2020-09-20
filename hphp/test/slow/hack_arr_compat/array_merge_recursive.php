<?hh

<<__EntryPoint>>
function main() {
  var_dump(array_merge_recursive(
    dict[ "1" => Map{ "1" => 42 }],
    dict[ "1" => Map{ "1" => 42 }]
  ));
}
