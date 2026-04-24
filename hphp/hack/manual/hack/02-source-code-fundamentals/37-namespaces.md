# Namespaces

A ***namespace*** is a container for a set of (typically related) definitions of classes, interfaces, traits, functions, and constants.

When the same namespace is declared across multiple scripts, and those scripts are combined into the same program, the resulting namespace
is the union of all of the namespaces' individual components.

## The Root Namespace
In the absence of any namespace definition, the names of subsequent classes, interfaces, traits, functions, and constants are in
the ***root namespace***, which is not named.

Some types, such as `Exception`, and constants, and library functions (such as `sqrt`) are inherited from PHP and, as such, are also defined outside a namespace. Prefix a backslash (`\`) to refer to these types or functions; for example: `\Exception`, `\sqrt`.

## Sub-Namespaces
A namespace can have ***sub-namespaces***, where a sub-namespace name shares a common prefix with another namespace.

For example, the namespace `Graphics` can have sub-namespaces `Graphics\TwoD` and `Graphics\ThreeD`, for two- and three-dimensional facilities,
respectively.

Apart from their common prefix, a namespace and its sub-namespaces have no special relationship. For example, `NS1\Sub` can exist without `NS1`. A named top-level namespace does not need to exist for a sub-namespace to exist.

## Reserved Namespaces
The namespaces `HH`, `PHP`, `php`, and sub-namespaces beginning with those prefixes are reserved for use by Hack.

## XHP and the `HTML` Namespace
As of HHVM 4.73 and XHP-Lib v4, standard XHP elements like `<p>` are defined in `Facebook\XHP\HTML` (for this example, specifically `Facebook\XHP\HTML\p`).

For more information, see [XHP Namespace Syntax](/hack/XHP/basic-usage#namespace-syntax).

## Special Constants
When debugging, use the predefined constant [`__NAMESPACE__`](/hack/source-code-fundamentals/constants#context-dependent-constants) to access the name of the current namespace.

## Declaring a Namespace
Namespace declarations can be file-scoped with `namespace MyNS;`, or block-scoped with `namespace MyNS { ... }`.

With semicolons, a namespace extends until the end of the script, or until the next namespace declaration, whichever is first.

```hack
namespace NS1;
// ...              // __NAMESPACE__ is "NS1"
namespace NS3\Sub1;
// ...              // __NAMESPACE__ is "NS3\Sub1"
```

With brace delimiters, a namespace extends from the opening brace to the closing brace.

```hack
namespace NS1 {
  // __NAMESPACE__ is "NS1"
}
namespace {
  // __NAMESPACE__ is ""
}
namespace NS3\Sub1 {
  // __NAMESPACE__ is "NS3\Sub1"
}
```
## The `use` Statement

The `use` statement introduces names defined in one namespace into another, so they can be referenced by their simple name rather than their fully qualified name. The `use` statement can only be present at the top level.

There are four forms: `use const`, `use function`, `use type`, and `use namespace`.

When importing many names of the same kind, use `{ ... }`:

```hack no-extract
use type NS1\{C, I, T}; // instead of three separate use statements
```

Names in `use` statements are always fully qualified — they don't need to be prefixed with `\`.

If an imported name introduces ambiguity, you can refer to name `foo` with `namespace\foo`—using the actual word `namespace`. For example, if you're importing a function `bar()`, but also want to call the `bar()` function from within your own namespace, refer to the one native to your namespace with `namespace\bar()`.

```hack file:use.hack
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

In the case of `use namespace`, we can reference names inside the given namespace by using a prefix that is the right-most part of the fully qualified name:

```hack file:use.hack
namespace {
  use namespace Hack\UserDocumentation\Statements\use\Examples\XXX;
}
```

Once this is seen, we can access `CON2` via the abbreviated `XXX\CON2`.
