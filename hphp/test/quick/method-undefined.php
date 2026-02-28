<?hh

class c{

}
function main($o) :mixed{
  if (false) {}
  $o->foo();
}
<<__EntryPoint>> function main_entry(): void {
main(new c());
}
