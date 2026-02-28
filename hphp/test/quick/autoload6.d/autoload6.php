<?hh

class C { function __toString() :mixed{ return 'I'; } }

<<__EntryPoint>> function main(): void {
  var_dump(interface_exists(new C));
}
