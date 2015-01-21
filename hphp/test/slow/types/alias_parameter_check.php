<?hh

# Running this test multiple times with profiling enabled (see the .opts file)
# tests whether instance bits are implemented properly in the presence of type
# aliases.

interface Base {}
type Foo = Base;
class Bar implements Base {}

function doit(Foo $x) {
  # instanceof type_alias isn't actually supported by Hack; this should return
  # false despite your intuition.  It's needed to create instance bits, which
  # are also used for the parameter type check.
  var_dump($x instanceof Foo);
}

function main() {
  $x = new Bar;
  doit($x);
}

main();
