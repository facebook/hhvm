<?hh

<<__EntryPoint>>
function main() :mixed{
  var_dump(array_merge_recursive(
    dict[ "1" => Map{ "1" => 42 }],
    dict[ "1" => Map{ "1" => 42 }]
  ));
}
