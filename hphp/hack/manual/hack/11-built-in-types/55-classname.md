For the most part, we deal with class types directly via their names.  For example:

```Hack no-extract
class Employee {
  // ...
}

$emp = new Employee();
```

However, in some applications, it is useful to be able to abstract a class' name rather than to hard-code it.  Consider the following:

```Hack file:employee.hack
<<__ConsistentConstruct>>
class Employee {
  // ...
}

function f(classname<Employee> $clsname): void {
  $w = new $clsname();  // create an object whose type is passed in
}
```

This function can be called with the name of the class `Employee` or any of its
subclasses.

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

In Hack code, the class names must be specified using "`::class` literals"
(`SomeClassName::class`). At runtime, these are regular strings (`SomeClassName::class === 'SomeClassName'`).

The value of an expression of a classname type can be converted implicitly or explicitly to `string`.
