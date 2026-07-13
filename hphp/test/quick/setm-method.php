<?hh


function main($o) :mixed{
  $o->circle = $o;
  $o = $o->circle;
  $o->foo();
}
<<__EntryPoint>> function main_entry(): void {
main(new stdClass);
}
