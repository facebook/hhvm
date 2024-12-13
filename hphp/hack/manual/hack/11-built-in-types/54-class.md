```yamlmeta
{
  "fbonly messages": [
    "This type is under migration, and is allowed to flow to the [classname type](/hack/built-in-types/classname) for existing string use cases. There is [an internal wiki](https://www.internalfb.com/intern/wiki/Hack_Foundation/classnameC_vs._classC/) describing the type separation."
  ]
}
```

Hack supports passing references to classes for use in instatiation and static
member access.

```Hack no-extract
<<__ConsistentConstruct>>
class Employee {
  public static function getKind(): int { return 4; };
}

function f(class<Employee> $cls): void {
  $w = new $cls();  // create an object of $cls

  $i = $cls::getKind();
}
```

Function `f` can be called with a reference to the class `Employee` or any of
its subclasses using `::class` literals (`SomeClassName::class`).

```Hack no-extract
class Intern extends Employee {
  // ...
}

function demo(): void {
  f(Employee::class);  // will call: new Employee();
  f(Intern::class);    // will call: new Intern();
  f(Vector::class);    // typechecker error!
}
```
