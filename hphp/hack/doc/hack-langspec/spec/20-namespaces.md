# Namespaces

## General

A problem encountered when managing large projects is that of avoiding
the use of the same name in the same scope for different purposes. This
is especially problematic in a language that supports modular design and
component libraries.

A *namespace* is a container for a set of (typically related)
definitions of classes, interfaces, traits, functions, and constants.
Namespaces serve two purposes:

* They help avoid name collisions.
* They allow certain long names to be accessed via shorter, more convenient and readable, names.

A namespace may have *sub-namespaces*, where a sub-namespace name shares
a common prefix with another namespace. For example, the namespace
`Graphics` might have sub-namespaces `Graphics\D2` and `Graphics\D3`, for
two- and three-dimensional facilities, respectively. Apart from their
common prefix, a namespace and its sub-namespaces have no special
relationship. The namespace whose prefix is part of a sub-namespace need
not actually exist for the sub-namespace to exist. That is, `NS1\Sub` can
exist without `NS1`.

In the absence of any namespace definition, the names of subsequent
classes, interfaces, traits, functions, and constants are in the
*default namespace*, which has no name, per se.

The namespaces HH, PHP, php, and sub-namespaces beginning with those
prefixes are reserved for use by Hack.

## Name Lookup

When an existing name is used in source code, the Engine must decide
how that name is found with respect to namespace lookup. For this
purpose, names can have one of the three following forms:

* Unqualified name: Such names are just simple names without any prefix, as in the class name `Point` in the following expression: `$p = new Point(3,5)`. If the current namespace is `NS1`, the name `Point` resolves to `NS1\Point`. If the current namespace is the [default namespace](20-namespaces.md#general), the name `Point` resolves to `Point`. In the case of an unqualified function or constant name, if that name does not exist in the current namespace, a global function or constant by that name is used.
* Qualified name: Such names have a prefix consisting of a namespace name and/or one or more levels of sub-namespace names, and, possibly, a class, interface, trait, function, or constant name. Such names are relative. For example, `D2\Point` could be used to refer to the class Point in the sub-namespace `D2` of the current namespace. One special case of this is when the first component of the name is the keyword `namespace`. This means "the current namespace".
* Fully qualified name: Such names begin with a backslash (`\`) and are followed optionally by a namespace name and one or more levels of sub-namespace names, and, finally, a class, interface, trait, function, or constant name. Such names are absolute. For example, `\Graphics\D2\Point` could be used to refer unambiguously to the class `Point` in namespace `Graphics`, sub-namespace `D2`.
   
The names of the standard types that come with PHP (such as `Exception`), constants (such as
`PHP_INT_MAX`), and library functions (such as `is_null`) are defined outside
any namespace. To refer unambiguously to such names, one can prefix them
with a backslash (`\`), as in `\Exception`, `\PHP_INT_MAX`, and `\is_null`. The names of the standard types that are introduced with Hack (such as `Vector` and `Map`), are treated implicitly as belonging to namespace HH. Such names are resolved through a process known as *auto importation*.

## Defining Namespaces

**Syntax**

<pre>
  <i>namespace-definition:</i>
    namespace  <i>namespace-name</i>  ;
    namespace  <i>namespace-name<sub>opt</sub></i>  { <i>declaration-list<sub>opt</sub></i> }

  <i>namespace-name:</i>
    <i>name </i>
    <i>namespace-name</i>   \   </i>name</i>

  <i>namespace-name-as-a-prefix:</i>
    \
    \<sub>opt</sub>   <i>namespace-name</i>   \
    namespace   \
    namespace   \   <i>namespace-name</i>   \

  <i>qualified-name:</i>
    <i>namespace-name-as-a-prefix<sub>opt</sub>   name</i>
</pre>

**Defined elsewhere**

* [*declaration-list*](04-basic-concepts.md)
* [*name*](09-lexical-structure.md#names)

**Constraints**

Except for white space, the
first occurrence of a *namespace-definition* in a script must be the
first thing in that script.

All occurrence of a *namespace-definition* in a script must have the
*declaration-list* form or must have the "semicolon" form; the two forms
cannot be mixed.

When a script contains source code that is not inside a namespace, and
source code that is inside one or namespaces, the namespaced code must
use the *declaration-list* form of *namespace-definition*.

The *declaration-list* must not contain a *namespace-definition*.

**Semantics**

Namespace and sub-namespace names are [case-preserved](03-terms-and-definitions.md).

The pre-defined constant [`__NAMESPACE__`](06-constants.md#context-dependent-constants) contains the name of
the current namespace.

When the same namespace is defined in multiple scripts, and those
scripts are combined into the same program, the namespace is considered
the merger of its individual contributions.

In the "semicolon" form of *namespace-definition* the given 
namespace extends until the end of the script, or until the lexically next
*namespace-definition*, whichever comes first. In the *declaration-list*
form the namespace extends from the opening brace to the closing brace.

**Examples**

Script1.php:
```Hack
namespace NS1;
...				// __NAMESPACE__ is "NS1"
namespace NS3\Sub1;
...				// __NAMESPACE__ is "NS3\Sub1"
```

Script2.php:
```Hack
namespace NS1
{
...				// __NAMESPACE__ is "NS1"
}
namespace
{
...				// __NAMESPACE__ is ""
}
namespace NS3\Sub1;
{
...				// __NAMESPACE__ is "NS3\Sub1"
}
```

## Namespace Use Declarations**

**Syntax**

<pre>
  <i>namespace-use-declaration:</i>
    use <i>namespace-use-kind<sub>opt</sub></i>  <i>namespace-use-clauses</i>  ;
    use <i>namespace-use-kind</i>  <i>namespace-name-as-a-prefix</i>  { <i>namespace-use-clauses</i>  }  ;
    use <i>namespace-name-as-a-prefix</i>  { <i>namespace-use-kind-clauses</i>  }  ;

  <i>namespace-use-clauses:</i>
    <i>namespace-use-clause</i>
    <i>namespace-use-clauses</i>  ,  <i>namespace-use-clause</i>

  <i>namespace-use-clause:</i>
    <i>qualified-name  namespace-aliasing-clause<sub>opt</sub></i>

  <i>namespace-use-kind-clauses:</i>
    <i>namespace-use-kind-clause</i>
    <i>namespace-use-kind-clauses</i>  ,  <i>namespace-use-kind-clause</i>

  <i>namespace-use-kind-clause:</i>
    <i>namespace-use-kind<sub>opt</sub></i>  <i>qualified-name  namespace-aliasing-clause<sub>opt</sub></i>

  <i>namespace-aliasing-clause:</i>
    as  <i>name</i>

  <i>namespace-use-kind</i>:
    function
    const
</pre>

**Defined elsewhere**

* [*name*](09-lexical-structure.md#names)
* [*qualified-name*](#defining-namespaces)

**Constraints**

A *namespace-use-declaration* must not occur except at the pseudomain
level.

If the same *qualified-name* is imported multiple times in the same
scope, each occurrence must have a different alias.

**Semantics**

If a *namespace-use-kind* is specified before the clauses or group prefix, 
then all subsequent clauses must name constants or functions, as appropriate.

Otherwise, if a *namespace-use-kind* is specified in a *namespace-use-kind-clause*
then the clause must name a constant or function, as appropriate.

Otherwise, the clause must name a namespace, class, interface or trait.

A *namespace-use-declaration* *imports*—that is, makes available—one or
more names into a scope, optionally giving them each an alias. Each of
those names may designate a namespace, a sub-namespace, a class, an
interface, or a trait. If a namespace-alias-clause is present, its
*name* is the alias for *qualified-name*. Otherwise, the right-most name
in *qualified-name* is the implied alias for *qualified-name*.

The "group" form of a *namespace-use-declaration* is a convenient syntax when
importing many members of a given namespace. The "group" form logically concatenates
the prefix onto the *qualified-name* in each clause. See the following section for 
an example.

**Examples**

```Hack
namespace NS1 {
  function f(): void { … }
  class C { … }
  interface I { … }
  trait T { … }
}

namespace NS2 {
  use \NS1\C, \NS1\I, \NS1\T;
  class D extends C implements I {
    use T;
  }
  \NS1\f();     // explicit namespace

  use \NS1\C as C2; // C2 is an alias for the class name \NS1\C
  $c2 = new C2;
}
```
The *namespace-use-declaration* in the example above could also be written in 
"group" form as:

```
  use \NS1\ { C, I, T };
```
