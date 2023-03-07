<?hh

class C { function __toString() { return 'I'; } }

<<__EntryPoint>> function main(): void {
  var_dump(interface_exists(new C));
}
