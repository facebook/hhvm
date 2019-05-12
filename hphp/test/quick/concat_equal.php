<?hh

function main($b) {
  $a .= $b;
  var_dump($a);
}
class b { function __toString() { return 'b'; }}
<<__EntryPoint>> function main_entry(): void {
main(new b);
}
