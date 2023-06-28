<?hh

<<__EntryPoint>>
function main() :mixed{
  var_dump(eregi_replace("\x28\x3f\x27\x24", "", ""));
}
