`readonly` is a keyword used to create immutable references to [Objects](/hack/classes/introduction) and their properties.

### How does it work?
Expressions in Hack can be annotated with the `readonly` keyword. When an object or reference is readonly, there are two main constraints on it:
* **Readonlyness:** Object properties cannot be modified (i.e. mutated).
* **Deepness:** All nested properties of a readonly value are readonly.


### Readonlyness
Object properties of `readonly` values cannot be modified (i.e. mutated).

```Hack error
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

function test(readonly Foo $x) : void {
  $x->prop = 4; // error, $x is readonly, its properties cannot be modified
}
```

### Deepness
All nested properties of `readonly` objects are readonly.

``` Hack error
function test(readonly Bar $x) : void {
  $foo = $x->foo;
  $foo->prop = 3; // error, $foo is readonly
}
```

### How is `readonly` different from contexts and capabilities that control property mutation (such as `write_props`)?
[Contexts](/hack/contexts-and-capabilities/available-contexts-and-capabilities) such as `write_props` affect an entire function (and all of its callees), whereas readonly affects specific values / expressions.


### Topics covered in this section
* [Syntax](syntax.md): Basic syntax for readonly keyword
* [Subtyping](subtyping.md): Rules and semantics for interacting with readonly and mutable values
* [Explicit Readonly Keywords](explicit-readonly-keywords.md): Positions where the readonly keyword is explicitly required
* [Containers and Collections](containers-and-collections.md): Interactions between collections of readonly values
* [Advanced Features and Semantics](advanced-semantics.md): More complex features and interactions
