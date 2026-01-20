# Introduction

Attributes attach metadata to Hack definitions.

Hack provides built-in attributes that can change runtime or
typechecker behavior.

```hack
<<__Memoize>>
function add_one(int $x): int {
  return $x + 1;
}
```

You can attach multiple attributes to a definition, and attributes can
have arguments.

``` Hack
<<__ConsistentConstruct>>
class OtherClass {
  <<__Memoize, __Deprecated("Use FooClass methods instead")>>
  public function addOne(int $x): int {
    return $x + 1;
  }
}
```

## Defining an attribute

You can define your own attribute by implementing an attribute
interface in the HH namespace.

```hack file:contributors.hack
class Contributors implements HH\ClassAttribute {
  public function __construct(private string $author, private ?keyset<string> $maintainers = null) {}
  public function getAuthor(): string {
    return $this->author;
  }
  public function getMaintainers(): keyset<string> {
    return $this->maintainers ?? keyset[$this->author];
  }
}

<<Contributors("John Doe", keyset["ORM Team", "Core Library Team"])>>
class MyClass {}

<<Contributors("You")>>
class YourClass {}
```

Other common attribute interfaces are `HH\FunctionAttribute`,
`HH\MethodAttribute` and `HH\PropertyAttribute`, but see [the Hack
interface reference](/apis/Interfaces/) for the full list.


## Accessing attribute arguments

You need to use reflection to access attribute arguments.

Given the `MyClass` example defined above:

```hack file:contributors.hack
$rc = new ReflectionClass('MyClass');
$my_class_contributors = $rc->getAttributeClass(Contributors::class);
$my_class_contributors?->getAuthor(); // "John Doe"
$my_class_contributors?->getMaintainers(); // keyset["ORM Team", "Core Library Team"]
```
