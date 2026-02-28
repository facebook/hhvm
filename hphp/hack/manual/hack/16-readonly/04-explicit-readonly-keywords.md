# Explicit Readonly Keywords

There are a few places where an explicit readonly keyword is required when using readonly values.

## Calling a readonly function
Calling a function or method that returns readonly requires wrapping the result in a readonly expression.

```hack

class Foo {}
function returns_readonly(): readonly Foo {
  return readonly new Foo();
}

function test(): void {
  $x = readonly returns_readonly(); // this is required to call returns_readonly()
}

```
## Accessing readonly properties
Accessing a readonly property (i.e. a property annotated readonly at the declaration, not accessing a property off of a readonly object) requires readonly annotation.

```hack

class Bar {}
class Foo {
  public function __construct(
    public readonly Bar $bar,
  ) {}
}

function test(Foo $f): void {
  $bar = readonly $f->bar; // this is required
}
```

## Interactions with [Coeffects](/hack/contexts-and-capabilities/available-contexts-and-capabilities)
If your function has the `ReadGlobals` capability but not the `AccessGlobals` capability (i.e. is marked `read_globals` or `leak_safe`), it can only access class static variables if they are wrapped in a readonly expression:

```hack
<<file:__EnableUnstableFeatures("readonly")>>
class Bar {}
class Foo {
  public static readonly ?Bar $bar = null;
}

function read_static()[read_globals]: void {
  $y = readonly Foo::$bar; // keyword required
}
function read_static2()[leak_safe]: void {
  $y = readonly Foo::$bar; // keyword required
}
```
