# Class

<FbCaution>

This type is under migration, with the class-specific use cases described below shifting to the new [class type](/hack/built-in-types/class). There is [an internal wiki](https://www.internalfb.com/intern/wiki/Hack_Foundation/classnameC_vs._classC/) describing the type separation.

</FbCaution>

Hack supports passing references to classes for use in instatiation and static
member access.

```hack no-extract
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

```hack no-extract
class Intern extends Employee {
  // ...
}

function demo(): void {
  f(Employee::class);  // will call: new Employee();
  f(Intern::class);    // will call: new Intern();
  f(Vector::class);    // typechecker error!
}
```
