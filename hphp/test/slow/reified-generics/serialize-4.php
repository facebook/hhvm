<?hh

class C<reify T> {}

<<__EntryPoint>>
function main() :mixed{
  $c = new C<int>();
  $invalid = 'O:1:"C":1:{s:14:"86reified_prop";a:1:{i:0;a:1:{s:4:"kind";i:77;}}}';
  $c2 = unserialize($invalid);
  var_dump($c2);
}
