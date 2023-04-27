The `use` statement permits names defined in one namespace to be introduced into another namespace, so they can be referenced
there by their simple name rather than their (sometimes very long) fully qualified name. The `use` statement can only be
present at the top level.

Consider the following:

```Hack file:use.hack
namespace UseNS {

  const int CON = 100;

  function f(): void {
    echo "In function ".__FUNCTION__."\n";
  }

  class C {
    public function f(): void {
      echo "In method ".__METHOD__."\n";
    }
  }

  class D {}
  class E {}
}

namespace Hack\UserDocumentation\Statements\use\Examples\XXX {

  const int CON2 = 500;

  function f(): void {
    echo "In function ".__FUNCTION__."\n";
  }
}

namespace Hack\UserDocumentation\Statements\use\Examples\test {

  use const UseNS\CON;
  use function UseNS\f;
  //use function Hack\UserDocumentation\Statements\use\Examples\XXX\f;  // Error: name f already declared
  use type UseNS\C;
  use type UseNS\{D, E};
  use namespace Hack\UserDocumentation\Statements\use\Examples\XXX;

  <<__EntryPoint>>
  function main(): void {

    // access const CON by fully qualified and abbreviated names

    echo "CON = ".\UseNS\CON."\n";
    echo "CON = ".CON."\n";

    // access function f by fully qualified and abbreviated names

    \UseNS\f();
    f();

    // access type C by fully qualified and abbreviated names

    $c = new \UseNS\C();
    $c->f();
    $c = new C();
    $c->f();

    // access type D by fully qualified and abbreviated names

    $d = new \UseNS\D();
    $d = new D();

    // access name f by fully qualified and abbreviated names

    \Hack\UserDocumentation\Statements\use\Examples\XXX\f();
    XXX\f();

    // access name CON2 by fully qualified and abbreviated names

    echo "XXX\CON2 = ".
      \Hack\UserDocumentation\Statements\use\Examples\XXX\CON2.
      "\n";
    echo "XXX\\CON2 = ".XXX\CON2."\n";
  }
}
```

Namespace `UseNS` contains  a definition for a constant `CON`. From within namespace `Hack\UserDocumentation\Statements\use\Examples\test`, we
can access that constant by its fully qualified name, `\UseNS\CON`, as shown in `main`. However, if we write `use const UseNS\CON;`, we can
access that constant's name simply as `CON`.

In the same manner, we can have `use type` and `use function` introduce type and function names, respectively. And as we can see
with `use type UseNS\{D, E};`, we can introduce a comma-separated list of names of the same kind in a single statement.

Note that we have two functions called `f`, defined in separate namespaces. If we attempt to introduce the same name from more than
one namespace, references to that name would be ambiguous, so this is disallowed.

In the case of `use namespace`, we can implicitly reference names inside the given namespace by using a prefix that is the right-most
part of the fully qualified name. For example, once

```Hack file:use.hack
namespace {
  use namespace Hack\UserDocumentation\Statements\use\Examples\XXX;
}
```

has been seen, we can access `CON2` via the abbreviated `XXX\CON2`.

Note that names in `use` statements are always fully qualified, they don't need
to be prefixed with `\`.
