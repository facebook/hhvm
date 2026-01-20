# Classname

<FbCaution>

This type is under migration, with the class-specific use cases described below shifting to the new [class type](/hack/built-in-types/class). There is [an internal wiki](https://www.internalfb.com/intern/wiki/Hack_Foundation/classnameC_vs._classC/) describing the type separation.

</FbCaution>

For the most part, we deal with class types directly via their names. For
example:

```hack no-extract
class Employee {
  // ...
}

$emp = new Employee();
```

However, in some applications, it is useful to be able to abstract a class' name
rather than to hard-code it. Consider the following:

```hack file:employee.hack
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

In Hack code, the class names must be specified using "`::class` literals"
(`SomeClassName::class`). The value of an expression of a classname type can be
converted implicitly or explicitly to `string`.
