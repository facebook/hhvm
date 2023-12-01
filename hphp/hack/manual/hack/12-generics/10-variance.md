Hack supports both generic *covariance* and *contravariance* on a type parameter.

Each generic parameter can optionally be marked separately with a variance indicator:
 * `+` for covariance
 * `-` for contravariance

If no variance is indicated, the parameter is invariant.

## Covariance

If `Foo<int>` is a subtype of `Foo<num>`, then `Foo` is covariant on `T`. 'co' means 'with'; and the subtype relationship of the generic
type goes with the subtype relationship of arguments to a covariant type parameter.  Here is an example:

```Hack
// This class is readonly. Had we put in a setter for $this->t, we could not
// use covariance. e.g., if we had function setMe(T $x), you would get this
// cov.php:9:25,25: Illegal usage of a covariant type parameter (Typing[4120])
//   cov.php:7:10,10: This is where the parameter was declared as covariant (+)
//   cov.php:9:25,25: Function parameters are contravariant
class C<+T> {
  public function __construct(private T $t) {}
}

class Animal {}
class Cat extends Animal {}

function f(C<Animal> $p1): void {
  \var_dump($p1);
}

function g(varray<Animal> $p1): void {
  \var_dump($p1);
}

<<__EntryPoint>>
function run(): void {
  f(new C(new Animal()));
  f(new C(new Cat())); // accepted

  g(vec[new Animal(), new Animal()]);
  g(vec[new Cat(), new Cat(), new Animal()]); // arrays are covariant
}
```

A covariant type parameter is for read-only types. Thus, if the type can somehow be set, then you cannot use covariance.

Covariance cannot be used as the type of a parameter on any method, or as the type of any mutable property, in that class.

## Contravariance

If `Foo<num>` is a subtype of `Foo<int>`, then `Foo` is contravariant on `T`. 'contra' means 'against'; and the subtype relationship
of the generic type goes against the subtype relationship of arguments to a contravariant type parameter.  Here is an example:

```Hack
// This class is write only. Had we put in a getter for $this->t, we could not
// use contravariance. e.g., if we had function getMe(T $x): T, you would get
// con.php:10:28,28: Illegal usage of a contravariant type
//                   parameter (Typing[4121])
//  con.php:5:10,10: This is where the parameter was declared as
//                   contravariant (-)
//  con.php:10:28,28: Function return types are covariant
class C<-T> {
  public function __construct(private T $t) {}
  public function set_me(T $val): void {
    $this->t = $val;
  }
}

class Animal {}
class Cat extends Animal {}

<<__EntryPoint>>
function main(): void {
  $animal = new Animal();
  $cat = new Cat();
  $c = new C($cat);
  // calling set_me with Animal on an instance of C that was initialized with Cat
  $c->set_me($animal);
  \var_dump($c);
}
```

A contravariant type parameter is for write-only types. Thus, if the type can somehow be read, then you cannot use
contravariant. (e.g., serialization functions are a good use case).

A contravariant type parameter cannot be used as the return type of any method in that class.
