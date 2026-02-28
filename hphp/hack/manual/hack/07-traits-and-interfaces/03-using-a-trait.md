# Using a Trait

Traits are a mechanism for code reuse that overcomes some limitations of Hack single inheritance model.

In its simplest form a trait defines properties and method declarations.  A trait cannot be instantiated with `new`, but it can be _used_ inside one or more classes, via the `use` clause.  Informally, whenever a trait is used by a class, the property and method definitions of the trait are inlined (copy/pasted) inside the class itself.  The example below shows a simple trait defining a method that returns even numbers.  The trait is used by two, unrelated, classes.

```hack
trait T {
  public int $x = 0;

  public function return_even() : int {
    invariant($this->x % 2 == 0, 'error, not even\n');
    $this->x = $this->x + 2;
    return $this->x;
  }
}

class C1 {
  use T;

  public function foo() : void {
    echo "C1: " . $this->return_even() . "\n";
  }
}

class C2 {
  use T;

  public function bar() : void {
    echo "C2: " . $this->return_even() . "\n";
  }
}

<<__EntryPoint>>
function main() : void {
  (new C1()) -> foo();
  (new C2()) -> bar();
}
```

A class can use multiple traits, and traits themselves can use one or more traits.  The example below uses three traits, to generate even numbers, to generate odd numbers given a generator of even numbers, and to test if a number is odd:

```hack
trait T1 {
  public int $x = 0;

  public function return_even() : int {
    invariant($this->x % 2 == 0, 'error, not even\n');
    $this->x = $this->x + 2;
    return $this->x;
  }
}

trait T2 {
  use T1;

  public function return_odd() : int {
    return $this->return_even() + 1;
  }
}

trait T3 {
  public static function is_odd(int $x) : bool {
    if ($x % 2 == 1) {
      return true;
    } else {
      return false;
    }
  }
}

class C {
  use T2;
  use T3;

  public function foo() : void {
    echo (string)static::is_odd($this->return_odd()) . "\n";
  }
}

<<__EntryPoint>>
function main() : void {
  (new C()) -> foo();
}
```

Traits can contain both instance and static members. If a trait defines a static property, then each class using the trait has its own instance of the static property.

A trait can access methods and properties of the class that uses it, but these dependencies must be declared using [trait requirements](/hack/traits-and-interfaces/trait-and-interface-requirements).

### Resolution of naming conflicts

A trait can insert arbitrary properties, constants, and methods inside a class, and naming conflicts may arise.  Conflict resolution rules are different according to whether the conflict concerns a property, constant, or method.

If a class uses multiple traits that define the same property, say `$x`, then every trait must define the property `$x` with the same type, visibility modifier, and initialization value.  The class itself may, or not, define again the property `$x`, subject to the same conditions as above.

Beware that at runtime all the instances of the multiply defined property `$x` are _aliased_. This might be source of unexpected interference between traits implementing unrelated services: in the example below the trait `T2` breaks the invariant of trait `T1` whenever both are used by the same class.

```hack
trait T1 {
  public static int $x = 0;

  public static function even() : void {
    invariant(static::$x % 2 == 0, 'error, not even\n');
    static::$x = static::$x + 2;
  }
}

trait T2 {
  public static int $x = 0;

  public static function inc() : void {
    static::$x = static::$x + 1;
  }
}

class C {
  use T1;
  use T2;

  public static function foo() : void {
    static::inc();
    static::even();
  }
}

<<__EntryPoint>>
function main() : void {
  try {
    C::foo();
  } catch (\Exception $ex) {
    echo "Caught an exception\n";
  }
}
```

For methods, a rule of thumb is "traits provide a method implementation if the class itself does not".  If the class implements a method `m`, then traits used by the class can define methods named `m` provided that their interfaces are compatible (more precisely _super types_ of the type of the method defined in the class.  At runtime methods inserted by traits are ignored, and dispatch selects the method defined in the class.

If multiple traits used by a class define the same method `m`, and a method named `m` is not defined by the class itself, then the code is rejected altogether, independently of the method interfaces.

Traits inherited along multiple paths (aka. "diamond traits") are rejected by Hack and HHVM whenever they define methods.  However the experimental `<<__EnableMethodTraitDiamond>>` attribute can be specified on the base class (or trait) to enable support for diamond traits that define methods, provided that method resolution remains unambiguous.  For instance, in the example below the invocation of `(new C())->foo()` unambiguously  resolves to the method `foo` defined in trait `T`:


```hack
<<file:__EnableUnstableFeatures('method_trait_diamond')>>

trait T {
  public function foo(): void { echo "I am T"; }
}

trait T1 { use T; }
trait T2 { use T; }

<<__EnableMethodTraitDiamond>>
class C {
  use T1, T2;
}

<<__EntryPoint>>
function main() : void {
  (new C())->foo();
}
```

```.ini
hhvm.diamond_trait_methods=1
```

Since the `<<__EnableMethodTraitDiamond>>` attribute is specified on the class `C`, the example is accepted by Hack and HHVM.

_Remark_: a diamond trait cannot define properties if it is used by a class via the `<<__EnableMethodTraitDiamond>>` attribute.


For constants, constants inherited from the parent class take precedence over constants inherited from traits.

```hack
trait T {
  const FOO = 'trait';
}

class B {
  const FOO = 'parent';
}

class A extends B { use T; }

<<__EntryPoint>>
function main() : void {
  \var_dump(A::FOO);
}
```

If multiple used traits declare the same constant, the constant inherited from the first trait is used.

```hack
trait T1 {
  const FOO = 'one';
}

trait T2 {
  const FOO = 'two';
}

class A { use T1, T2; }

<<__EntryPoint>>
function main() : void {
  \var_dump(A::FOO);
}
```

Finally, constants inherited from interfaces declared on the class conflict with other inherited constants, including constants declared on traits.

```hack error
trait T {
  const FOO = 'trait';
}

interface I {
  const FOO = 'interface';
}

class A implements I { use T; }

<<__EntryPoint>>
function main() : void {
  \var_dump(A::FOO);
}
```

The single exception to this rule are constants inherited from traits via interfaces, as these will lose silently upon conflict.

```hack
interface I1 {
  const FOO = 'one';
}

trait T implements I1 {}

interface I {
  const FOO = 'two';
}

class A implements I { use T; }

<<__EntryPoint>>
function main() : void {
  \var_dump(A::FOO);
}
```
