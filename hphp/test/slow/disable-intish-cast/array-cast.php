<?hh

<<__EntryPoint>>
function main() {
  $a = darray["10" => "string!", 10 => "number!"];
  $b = darray[10 => "string!", "10" => "number!"];

  var_dump((array)$a);
  var_dump((array)$b);
}
