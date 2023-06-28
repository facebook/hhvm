<?hh

<<__EntryPoint>>
function main() :mixed{
  var_dump(file_get_contents(__FILE__, false, null, 2147483647, 32));
  var_dump(file_get_contents(__FILE__, false, null, 2147483647*2, 32));
}
