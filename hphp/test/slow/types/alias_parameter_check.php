<?hh

# Running this test multiple times with profiling enabled (see the .opts file)
# tests whether instance bits are implemented properly in the presence of type
# aliases.

interface Base {}
type Foo = Base;
class Bar implements Base {}

function doit(Foo $x) {
  var_dump($x is Foo);
}

function main() {
  $x = new Bar;
  doit($x);
}


<<__EntryPoint>>
function main_alias_parameter_check() {
main();
}
