The `readonly` keyword can be applied to various positions in Hack.

## Parameters and return values
Parameters and return values of any callable (e.g. a function or method) can be marked `readonly`.

```Hack
class Bar {
  public function __construct(
    public Foo $foo,
  ){}
}
class Foo {
  public function __construct(
    public int $prop,
  ) {}
}

function getFoo(readonly Bar $x): readonly Foo {
  return $x->foo;
}
```

A readonly *parameter* signals that the function/method will not modify that parameter when called. A readonly *return type* signals that the function returns a readonly reference to an object that cannot be modified.

## Static and regular properties
Static and regular properties marked as `readonly` cannot be modified.

```Hack
class Bar {}
class Foo {
  private static readonly ?Bar $static_bar = null;
  public function __construct(
    private readonly Bar $bar,
  ){}
}
```
A readonly property represents a property that holds a readonly reference (specifically, that the nested object within the property cannot be modified).


## Lambdas and function type signatures
`readonly` is allowed on inner parameters and return types on function typehints.

```Hack
class Bar {}
function call(
    (function(readonly Bar) : readonly Bar) $f,
    readonly Bar $arg,
   ) : readonly Bar {
   return readonly $f($arg);
}
```

## Expressions
`readonly` can appear on expressions to convert mutable values to readonly.
```Hack
class Foo {}
function foo(): void {
  $x = new Foo();
  $y = readonly $x;
}
```

## Functions / Methods
`readonly` can appear as a modifier on instance methods, signaling that `$this` is readonly (i.e, that the method promises not to modify the instance).

``` Hack error
class C {
  public function __construct(public int $prop) {}
  public readonly function foo() : void {
    $this->prop = 4; // error, $this is readonly.
  }
}
```
Note that readonly objects can only call readonly methods, since they promise not to modify the object.

``` Hack error
class Data {}
class Box {
  public function __construct(public Data $data) {}
  public readonly function getData(): readonly Data {
    return $this->data;
  }
  public function setData(Data $d) : void {
    $this->data = $d;
  }
}
function readonly_method_example(readonly Box $b): void {
  $y = readonly $b->getData(); // ok, $y is readonly
  $b->setData(new Data()); // error, $b is readonly, it can only call readonly methods
}
```

## Closures and function types
A function type can be marked readonly: `(readonly function(T1): T)`. Denoting a function/closure as readonly adds the restriction that the function/closure captures all values as readonly:

``` Hack error
function readonly_closure_example(): void {
  $x = new Foo();
  $f = readonly () ==> {
    $x->prop = 4; // error, $x is readonly here!
  };
}
```
One way to make sense of this behavior is to think of closures as an object whose properties are the values it captures, which implement a special invocation function that executes the closure. A readonly closure is then defined as a closure whose invocation function is annotated with readonly.

Readonly closures affect Hack’s type system; readonly closures are subtypes of their mutable counterparts. That is, a `(readonly function(T1):T2)` is a strict subtype of a `(function(T1): T2)`.
