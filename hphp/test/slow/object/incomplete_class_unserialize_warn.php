<?hh

<<__EntryPoint>>
function main() :mixed{
  // normal serialization format
  var_dump(unserialize('O:1:"X":1:{s:1:"p";i:0;}'));
  // Serializable format
  var_dump(unserialize('C:1:"Y":3:{lol}'));
}
