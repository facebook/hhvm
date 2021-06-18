<?hh


function main($o) {
  $o = $o->circle = $o;
  $o->foo();
}
<<__EntryPoint>> function main_entry(): void {
main(new stdClass);
}
