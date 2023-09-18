Modules are an experimental feature for organizing code and separating your internal and external APIs. Modules are collections of Hack files that share an identity or utility.

## Module definitions
You can define a new module with the `new module` keywords.

```hack file:foomodule.hack
//// module.hack
new module foo {}
```
Modules do not share a namespace with other symbols. Module names can contain periods `.` to help delineate between directories or logical units. Periods in module names do not currently have any semantic meaning.

```hack
//// module_foo.hack
new module foo {}
//// module_foobar.hack
new module foo.bar {}
//// module_foobartest.hack
new module foo.bar.test {}
```

## Module membership
Any Hack file can be placed in a defined module by writing `module <module name>` at the top of the file.

```hack no-extract
module foo;
class Foo {
  // ...
}
```

## Module level visibility: `internal`
By placing your code in modules, you can use a new visibility keyword: `internal`. An `internal` symbol can only be accessed from within the module.

```hack file:foomodule.hack
//// foo_class.hack
module foo;

public class Foo {}
internal class FooInternal {
  public function foo(): void {}
  internal function bar(): void {}
}
internal function foo_fun(): void {}
```

An internal symbol cannot be accessed outside of the module it's defined in.

```hack no-extract
module bar;

public function bar_test(): void {
  $x = new FooInternal(); // error! FooInternal is internal to module foo.
}
```
