<?hh

class C {}

<<__EntryPoint>>
function main() :mixed{
  $array = vec(HH\array_mark_legacy(varray[new C()]));
  var_dump(HH\is_array_marked_legacy($array));
}
