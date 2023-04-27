## Defining modules
You can define a module with the `new module` syntax. A module, just like any other toplevel entity (classes, functions, etc), can have at most one definition.

```hack
new module foo {}
```
Module names live in their own namespace, and do not conflict with classes, functions, etc. Module names are **not** affected by namespaces.

```Hack no-extract
namespace Bar;
module foo;
function foo(): void {}
```

Currently, module bodies are empty. This will change in future versions of Hack as we build more support for modules and organizing their relationships.

# Module membership
A module can be thought of as a logical structure of code organized into a list of files. To add a file to a module, use the `module foo;` syntax at the top of the file.

```Hack no-extract
// This file is now a member of module foo
module foo;
class MyFoo {
    // ...
}
```
A file can have at most one module membership statement, and the statement must be before any symbol in the file (File attributes and namespace declarations can appear before module membership statements, as they are not referenceable symbols). For clarity, module definitions (i.e. `new module`) cannot be placed within any module.

```hack error
module foo; // ok
module bar; // not okay: duplicate module membership statement
```

```hack error
module bar;
// not okay: module definitions must live outside of files already in a module
new module foo {}
```
