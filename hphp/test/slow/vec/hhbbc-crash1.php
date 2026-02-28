<?hh

<<__EntryPoint>>
function main() :mixed{
  $v = vec["bar"];
  $v[1] = 1;
  var_dump($v);
}
