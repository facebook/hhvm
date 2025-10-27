# Scope

The same name can designate different things at different places in a program. For each different thing that a name
designates, that name is visible only within a part of the program called that name's *scope*.  The following distinct scopes exist:
-   Script, which means from the point of declaration/first initialization through to the end of that script,
including any [included scripts](/hack/source-code-fundamentals/script-inclusion).
-   Function, which means from the point of declaration/first initialization through to the end of that
[function](/hack/functions/introduction).
-   Class, which means the body of that class and any classes derived from it ([defining a class](/hack/classes/introduction)).
-   Interface, which means the body of that interface, any interfaces derived from it, and any classes that implement it
([implementing an interface](/hack/traits-and-interfaces/implementing-an-interface)).
-   Namespace, which means from the point of declaration/first initialization through to the end of that
[namespace](/hack/source-code-fundamentals/namespaces).

A variable declared or first initialized inside a function has function scope.

Each function has its own function scope. An [anonymous function](/hack/functions/anonymous-functions) has its own scope
separate from that of any function inside which that anonymous function is defined.

The scope of a parameter is the body of the function in which the parameter is declared. For the purposes of scope, a
[catch-block](/hack/statements/try) is treated like a function body.

The scope of a class member ([defining a class](/hack/classes/introduction)) declared in, or inherited by, a class type `C` is
the body of `C`.

The scope of an interface member ([implementing an interface](/hack/traits-and-interfaces/implementing-an-interface)) declared in, or inherited by, an interface
type `I` is the body of `I`.

When a [trait](/hack/traits-and-interfaces/using-a-trait) is used by a class or an interface, the trait's members take on the scope of a
member of that class or interface.
