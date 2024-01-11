Trait and interface requirements allow you to restrict the use of these constructs by specifying what classes may actually use a trait or
implement an interface. This can simplify long lists of `abstract` method requirements, and provide a hint to the reader as to the
intended use of the trait or interface.

## Syntax

To introduce a trait requirement, you can have one or more of the following in your trait:

```Hack no-extract
require extends <class name>;
require implements <interface name>;
```

*Note:* the experimental `require class <class name>;` trait constraint is discussed [below](#traits__the-require-class-trait-constraints-experimental).

To introduce an interface requirement, you can have one or more of following in your interface:

```Hack no-extract
require extends <class name>;
```

## Traits

Here is an example of a trait that introduces a class and interface requirement, and shows a class that meets the requirement:

```Hack
abstract class Machine {
  public function openDoors(): void {
    return;
  }
  public function closeDoors(): void {
    return;
  }
}
interface Fliers {
  public function fly(): bool;
}

trait Plane {
  require extends Machine;
  require implements Fliers;

  public function takeOff(): bool {
    $this->openDoors();
    $this->closeDoors();
    return $this->fly();
  }
}

class AirBus extends Machine implements Fliers {
  use Plane;

  public function fly(): bool {
    return true;
  }
}

<<__EntryPoint>>
function run(): void {
  $ab = new AirBus();
  \var_dump($ab);
  \var_dump($ab->takeOff());
}
```

Here is an example of a trait that introduces a class and interface requirement, and shows a class that *does not* meet the requirement:

```Hack error
abstract class Machine {
  public function openDoors(): void {
    return;
  }
  public function closeDoors(): void {
    return;
  }
}
interface Fliers {
  public function fly(): bool;
}

trait Plane {
  require extends Machine;
  require implements Fliers;

  public function takeOff(): bool {
    $this->openDoors();
    $this->closeDoors();
    return $this->fly();
  }
}

// Having this will not only cause a typechecker error, but also cause a fatal
// error in HHVM since we did not meet the trait requirement.
class Paper implements Fliers {
  use Plane;

  public function fly(): bool {
    return false;
  }
}

<<__EntryPoint>>
function run(): void {
  // This code will not run in HHVM because of the problem mentioned above.
  $p = new Paper();
  \var_dump($p);
  \var_dump($p->takeOff());
}
```

**NOTE**: `require extends` should be taken literally. The class *must* extend the required class; thus the actual required class
**does not** meet that requirement. This is to avoid some subtle circular dependencies when checking requirements.

#### The `require class` trait constraints (experimental)

A `require class <class name>;` trait constraint in a trait specifies that the trait can only be used by the
_non-generic, _final_, class `<class name>`.  This contrasts with the `require extends t;` constraints that allow the trait to be used by an arbitrary _strict_ subtype of `t`.
By relaxing the strict subtype constraint of `require extends`, `require class` constraints allow splitting the implementation of a class into a
class and one (or multiple) traits, as in the following:

```Hack
<<file:__EnableUnstableFeatures('require_class')>>

trait T {
  require class C;
  public function foo(): void {
    $this->bar();
  }
}

final class C {
  use T;
  public function bar(): void {}
}
```

## Interfaces

Here is an example of an interface that introduces a class requirement, and shows a class that meets the requirement:

```Hack
abstract class Machine {
  public function openDoors(): void {
    return;
  }
  public function closeDoors(): void {
    return;
  }
}
interface Fliers {
  require extends Machine;
  public function fly(): bool;
}

class AirBus extends Machine implements Fliers {
  public function takeOff(): bool {
    $this->openDoors();
    $this->closeDoors();
    return $this->fly();
  }

  public function fly(): bool {
    return true;
  }
}

<<__EntryPoint>>
function run(): void {
  $ab = new AirBus();
  \var_dump($ab);
  \var_dump($ab->takeOff());
}
```

Here is an example of an interface that introduces a class requirement, and shows a class that *does not* meet the requirement:

```Hack error
abstract class Machine {
  public function openDoors(): void {
    return;
  }
  public function closeDoors(): void {
    return;
  }
}
interface Fliers {
  require extends Machine;
  public function fly(): bool;
}

// Having this will not only cause a typechecker error, but also cause a fatal
// error in HHVM since we did not meet the interface requirement (extending
// Machine).
class Paper implements Fliers {
  public function fly(): bool {
    return false;
  }
}

<<__EntryPoint>>
function run(): void {
  // This code will actually not run in HHVM because of the fatal mentioned
  // above.
  $p = new Paper();
  \var_dump($p);
  \var_dump($p->takeOff());
}
```

**NOTE**: trait cannot be used as a type, comparing to some other languages. Only class and interface are types. For example,

```Hack no-extract
trait T {}
class C { use T; }
$a = new C();
$j = ($a is C);
$k = ($a is T); # error!
```
leads to error

```
Hit fatal : "is" and "as" operators cannot be used with a trait
    #0 at [:1]
    #1 include(), called at [:1]
    #2 include(), called at [:0]
Hit fatal : "is" and "as" operators cannot be used with a trait
Failed to evaluate expression
```
