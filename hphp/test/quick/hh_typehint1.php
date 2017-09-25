<?hh

class Bar {}
class Foo {}
function foo(?Bar $x) {}
foo(new Foo);
