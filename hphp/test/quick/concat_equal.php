<?hh

function main($b) :mixed{
  $a = '';
  $a .= (string)($b);
  var_dump($a);
}
class b { function __toString() :mixed{ return 'b'; }}
<<__EntryPoint>> function main_entry(): void {
main(new b);
}
