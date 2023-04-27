A method is a function defined in a class.

```Hack file:person.hack
class Person {
  public string $name = "anonymous";

  public function greeting(): string {
    return "Hi, my name is ".$this->name;
  }
}
```

To call an instance method, use `->`.

```Hack file:person.hack
$p = new Person();
echo $p->greeting();
```

You can access the current instance with `$this` inside a method.

## Static Methods

A static method is a function in a class that is called without an
instance. Since there's no instance, `$this` is not available.

```Hack file:person2.hack
class Person {
  public static function typicalGreeting(): string {
    return "Hello";
  }
}
```

To call a static method, use `::`.

```Hack file:person2.hack
echo Person::typicalGreeting();
```
