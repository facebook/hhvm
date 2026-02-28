# Type Constants Revisited

Imagine that you have a class, and some various `extends` to that class.

```hack
abstract class User {
  public function __construct(private int $id) {}
  public function getID(): int {
    return $this->id;
  }
}

trait UserTrait {
  require extends User;
}

interface IUser {
  require extends User;
}

class AppUser extends User implements IUser {
  use UserTrait;
}

<<__EntryPoint>>
function run(): void {
  $au = new AppUser(-1);
  \var_dump($au->getID());
}
```

Now imagine that you realize that sometimes the ID of a user could be a `string` as well as an `int`. But you know that the concrete classes
of `User` will know exactly what type will be returned.

While this situation could be handled by using generics, an alternate approach is to use type constants. Instead of types being declared
as parameters directly on the class itself, type constants allow the type to be declared as class member constants instead.

```hack
abstract class User {
  abstract const type T as arraykey;
  public function __construct(private this::T $id) {}
  public function getID(): this::T {
    return $this->id;
  }
}

trait UserTrait {
  require extends User;
}

interface IUser {
  require extends User;
}

// We know that AppUser will only have int ids
class AppUser extends User implements IUser {
  const type T = int;
  use UserTrait;
}

class WebUser extends User implements IUser {
  const type T = string;
  use UserTrait;
}

class OtherUser extends User implements IUser {
  const type T = arraykey;
  use UserTrait;
}

<<__EntryPoint>>
function run(): void {
  $au = new AppUser(-1);
  \var_dump($au->getID());
  $wu = new WebUser('-1');
  \var_dump($wu->getID());
  $ou1 = new OtherUser(-1);
  \var_dump($ou1->getID());
  $ou2 = new OtherUser('-1');
  \var_dump($ou2->getID());
}
```

Notice the syntax `abstract const type <name> [ as <constraint> ];`. All type constants are `const` and use the keyword `type`. You
specify a name for the constant, along with any possible [constraints](/hack/generics/type-constraints) that
must be adhered to.

## Using Type Constants

Given that the type constant is a first-class constant of the class, you can reference it using `this`. As
a type annotation, you annotate a type constant like:

```
this::<name>
```

e.g.,

```
this::T
```

You can think of `this::` in a similar manner as the [`this` return type](/hack/built-in-types/this).

This example shows the real benefit of type constants. The property is defined in `Base`, but can have different types depending
on the context of where it is being used.

```hack
abstract class Base {
  abstract const type T;
  protected this::T $value;
}

class Stringy extends Base {
  const type T = string;
  public function __construct() {
    // inherits $value in Base which is now setting T as a string
    $this->value = "Hi";
  }
  public function getString(): string {
    return $this->value; // property of type string
  }
}

class Inty extends Base {
  const type T = int;
  public function __construct() {
    // inherits $value in Base which is now setting T as an int
    $this->value = 4;
  }
  public function getInt(): int {
    return $this->value; // property of type int
  }
}

<<__EntryPoint>>
function run(): void {
  $s = new Stringy();
  $i = new Inty();
  \var_dump($s->getString());
  \var_dump($i->getInt());
}
```

## Examples

Here are some examples of where type constants may be useful:

### Referencing Type Constants

Referencing type constants is as easy as referencing a static class constant.

```hack
abstract class UserTC {
  abstract const type Ttc as arraykey;
  public function __construct(private this::Ttc $id) {}
  public function getID(): this::Ttc {
    return $this->id;
  }
}

class AppUserTC extends UserTC {
  const type Ttc = int;
}

function get_id_from_userTC(AppUserTC $uc): AppUserTC::Ttc {
  return $uc->getID();
}

<<__EntryPoint>>
function run(): void {
  $autc = new AppUserTC(10);
  \var_dump(get_id_from_userTC($autc));
}
```

### Type Constants and Instance Methods

You can use type constants as inputs to class instance methods.

```hack
abstract class Box {
  abstract const type T;
  public function __construct(private this::T $value) {}
  public function get(): this::T {
    return $this->value;
  }
  public function set(this::T $val): this {
    $this->value = $val;
    return $this;
  }
}

class IntBox extends Box {
  const type T = int;
}

<<__EntryPoint>>
function run(): void {
  $ibox = new IntBox(10);
  \var_dump($ibox);
  $ibox->set(123);
  \var_dump($ibox);
}
```
