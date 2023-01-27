**Note:** Context and capabilities are enabled by default since
[HHVM 4.93](https://hhvm.com/blog/2021/01/19/hhvm-4.93.html).

The following contexts and capabilities are implemented at present.

## Capabilities

### IO

This gates the ability to use the `echo` and `print` intrinsics within function bodies.
Additionally, built-in functions that perform output operations such as file writes and DB reads will require this capablity.

```Hack
function does_echo_and_print(): void {
  echo 'like this';
  print 'or like this';
}
```

### WriteProperty

This gates the ability to modify objects within function bodies.
Built-in functions that modify their inputs or methods that modify `$this` will require this capability.

At present, all constructors have the ability to modify `$this`. Note that this does *not* imply that constructors can call functions requiring the WriteProperty capability.

```Hack
// Valid example

class SomeClass {
  public string $s = '';
  public function modifyThis()[write_props]: void {
    $this->s = 'this applies as well';
  }
}

function can_write_props(SomeClass $sc)[write_props]: void {
  $sc->s = 'like this';
  $sc2 = new SomeClass();
  $sc2->s = 'or like this';
}
```

```Hack error
// Invalid example

class SomeClass {
  public string $s = '';
  public function modifyThis()[]: void {  // pure (empty context list)
    $this->s = 'this applies as well';
  }
}

function pure_function(SomeClass $sc)[]: void {
  $sc->s = 'like this';
}
```

Hack Collections, being objects, require this capability to use the array access operator in a write context.

```Hack
function modify_collection()[write_props]: void {
  $v = Vector {};
  $v[] = 'like this';
  $m = Map {};
  $m['or'] = 'like this';
}
```

### AccessGlobals

This gates the ability to access static variables and globals.
Built-in functions that make use of mutable global state or expose the php-style superglobals will require this capability.

```Hack
// Valid example

class SomeClass {
  public static string $s = '';
  public function accessStatic()[globals]: void {
    self::$s; // like this
  }
}

function access_static()[globals]: void {
  SomeClass::$s; // or like this
}
```

```Hack error
// Invalid example

class SomeClass {
  public static string $s = '';
  public function pureMethod()[]: void {
    self::$s; // like this
  }
}

function pure_function()[]: void {
  SomeClass::$s; // or like this
}
```


## Contexts

- `defaults` represents the capability set {IO, WriteProperty, AccessGlobals}.
- `write_props` represents the capability set {WriteProperty}.
- `globals` represents the capability set {AccessGlobals}.

### The Empty List

The empty context list, `[]`, has no capabilities. A function with no capabilities is the closest thing Hack has to 'pure' functions. As additional capabilities are added to Hack in the future, the restriction on these functions will increase.

As such, this is sometimes referred to as the 'pure context'.
