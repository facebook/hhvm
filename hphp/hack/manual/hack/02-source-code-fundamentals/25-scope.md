The same name can designate different things at different places in a program. For each different thing that a name
designates, that name is visible only within a part of the program called that name's *scope*.  The following distinct scopes exist:
-   Script, which means from the point of declaration/first initialization through to the end of that script,
including any [included scripts](script-inclusion.md).
-   Function, which means from the point of declaration/first initialization through to the end of that
[function](../functions/defining-a-function).
-   Class, which means the body of that class and any classes derived from it ([defining a class](../classes/defining-a-basic-class.md)).
-   Interface, which means the body of that interface, any interfaces derived from it, and any classes that implement it
([implementing an interface](../classes/implementing-an-interface.md)).
-   Namespace, which means from the point of declaration/first initialization through to the end of that
[namespace](../source-code-fundamentals/namespaces.md).

A variable declared or first initialized inside a function has function scope.

Each function has its own function scope. An [anonymous function](../functions/anonymous-functions.md) has its own scope
separate from that of any function inside which that anonymous function is defined.

The scope of a parameter is the body of the function in which the parameter is declared. For the purposes of scope, a
[catch-block](../statements/try.md) is treated like a function body.

The scope of a class member ([defining a class](../classes/defining-a-basic-class.md)) declared in, or inherited by, a class type `C` is
the body of `C`.

The scope of an interface member ([implementing an interface](../classes/implementing-an-interface.md)) declared in, or inherited by, an interface
type `I` is the body of `I`.

When a [trait](../classes/using-a-trait.md) is used by a class or an interface, the trait's members take on the scope of a
member of that class or interface.
