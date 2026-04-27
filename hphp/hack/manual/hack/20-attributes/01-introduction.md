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

The full set of attribute interfaces is shown below. Each interface
restricts where the attribute may be attached.

| Interface | Applies to |
| --- | --- |
| `HH\ClassAttribute` | a class |
| `HH\ClassConstantAttribute` | a constant of a class |
| `HH\EnumAttribute` | an enum |
| `HH\EnumClassAttribute` | an enum class |
| `HH\TypeAliasAttribute` | a type, newtype or case type |
| `HH\FunctionAttribute` | a function |
| `HH\MethodAttribute` | a method |
| `HH\InstancePropertyAttribute` | an instance property |
| `HH\StaticPropertyAttribute` | a static property |
| `HH\ParameterAttribute` | a parameter |
| `HH\TypeParameterAttribute` | a type parameter |
| `HH\TypeConstantAttribute` | a type constant |
| `HH\FileAttribute` | a file |
| `HH\LambdaAttribute` | a lambda expression |
| `HH\ModuleAttribute` | a module |

See [the Hack interface reference](/apis/Interfaces/) for the full
signature of each interface.


## Accessing attribute arguments

You need to use reflection to access attribute arguments.

Given the `MyClass` example defined above:

```hack file:contributors.hack
$rc = new ReflectionClass('MyClass');
$my_class_contributors = $rc->getAttributeClass(Contributors::class);
$my_class_contributors?->getAuthor(); // "John Doe"
$my_class_contributors?->getMaintainers(); // keyset["ORM Team", "Core Library Team"]
```
