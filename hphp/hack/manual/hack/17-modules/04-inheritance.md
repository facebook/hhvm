## Inheriting methods and properties
In general, override rules for internal class members are consistent with that of private and protected: you can only override a class member with a visibility that's at least as visible as the previous.

```hack no-extract
module foo;
class Foo {
    internal function foo(): void {}

    public function bar(): void {}

    internal function baz(): void {}
}
class Bar extends Foo {
    <<__Override>>
    public function foo(): void {} // ok

    <<__Override>>
    internal function bar(): void {} // error, overriding a public method with a lower visibility

    <<__Override>>
    private function baz(): void {} // ok
}
```

Note that `internal` and `protected` do not encompass each other in terms of visibility. Therefore, you can never override an internal member with a protected one, or vice versa.

The same rule applies when implementing an interface:

```hack no-extract
interface IFoo {
    public function bar(): void;
    internal function baz(): void;
}
class Foo implements IFoo {
    // You must implement a public function in an interface with another public one
    public function bar(): void {}
    // You **can** implement an internal function in an interface with a public one
    public function baz(): void {}
}
```


## Inheriting toplevel symbols
Unlike with methods and properties, within the same module, an internal class can override a public one, and vice versa.

```hack no-extract
public class Bar {}
internal class Foo extends Bar {}
public class Baz extends Foo {}
```
You can think of this behavior as freely choosing which symbols in your module to export to your public API. Since the visibility of a toplevel entity only affects how it is statically referenced, no inheritance rules need to be applied when overriding them.

You cannot, however, extend an internal class, implement an internal interface, or use an internal trait from a different module.

```hack
//// newmodule.hack
new module foo {}

//// foo.hack
module foo;
internal interface IFoo {
    //...
}
class Foo implements IFoo {} // ok
```

```hack no-extract
module bar;
class Bar implements IFoo {} // error, IFoo is internal to module foo
```
