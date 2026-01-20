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
## Importing from other Namespaces
With the `use` keyword, a namespace can import one or more member names into a scope, optionally giving them each an alias.

When importing many names, use `{ ... }`.

```hack no-extract
use namespace NS1\{C, I, T}; // instead of `NS1\C, NS1\I, NS1\T`
```

Imported names can designate a namespace, a sub-namespace, a class or interface or trait, a function, or any built-in type.

If an imported name introduces ambiguity, you can refer to name `foo` with `namespace\foo`—using the the actual word `namespace`. For example, if you're importing a function `bar()`, but also want to call the `bar()` function from within your own namespace, refer to the one native to your namespace with `namespace\bar()`.


```hack
namespace NS1 {
  const int CON1 = 100;
  function f(): void {
    echo "In ".__FUNCTION__."\n";
  }

  class C {
    const int C_CON = 200;
    public function f(): void {
      echo "In ".__NAMESPACE__."...".__METHOD__."\n";
    }
  }

  interface I {
    const int I_CON = 300;
  }

  trait T {
    public function f(): void {
      echo "In ".__TRAIT__."...".__NAMESPACE__."...".__METHOD__."\n";
    }
  }
}

namespace NS2 {
  use type NS1\{C, I, T};

  class D extends C implements I {
    use T;
  }

  function f(): void {
    $d = new D();
    echo "CON1 = ".\NS1\CON1."\n";
    \NS1\f();
  }
}
```
