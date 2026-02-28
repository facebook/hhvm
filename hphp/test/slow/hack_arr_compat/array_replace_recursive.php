<?hh

<<__EntryPoint>>
function main() :mixed{
  var_dump(array_replace_recursive(
    dict["1" => dict[ "1" => 42]],
    dict["1" => dict[ "1" => 3]]
  ));
}
