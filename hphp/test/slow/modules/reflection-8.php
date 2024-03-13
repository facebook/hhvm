<?hh

<<__EntryPoint>>
function main() :mixed{
  var_dump((new ReflectionFile(__FILE__))->getModule());
}
