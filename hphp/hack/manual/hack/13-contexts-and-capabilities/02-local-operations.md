**Note:** Context and capabilities are enabled by default since
[HHVM 4.93](https://hhvm.com/blog/2021/01/19/hhvm-4.93.html).

The existence of a capability (or lack thereof) within the contexts of a function plays a direct role in the operations allowed within that function.

Consider the following potential (although not necessarily planned) contexts (with implied matching capabilities):
* `throws<T>`, representing the permission to throw an exception type `Te <: T`
* `io`, representing the permission to do io
* `statics`, representing the permission to access static members and global variables
* `write_prop`, representing the permission to mutate properties of objects
* `dynamic`, representing the permission to cast a value to the `dynamic` type

In all of the below cases, the relevant local operations successfully typecheck due to the matching capabilities within the context of the function.

```hack no-extract
function io_good()[io]: void {
  echo "good"; // ok
  print "also ok"; // also ok
}
```

```hack no-extract
class FooException extends Exception {}
class FooChildException extends FooException {}
class BarException extends Exception {}

function throws_foo_exception()[throws<FooException>]: void {
  throw new FooException(); // ok: FooException <: FooException
  throw new FooChildException(); // ok: FooChildException <: FooException
}

function throws_bar_exception()[throws<BarException>]: void {
  throw new BarException(); // ok: BarException <: BarException
}

function throws_foo_and_bar_exceptions()[throws<FooException>, throws<BarException>]: void {
  throw new FooException(); // ok: FooException <: FooException
  throw new FooChildException(); // ok: FooChildException <: FooException
  throw new BarException(); // ok: BarException <: BarException
}
```

```hack no-extract
class HasAStatic {
  public static int $i = 0;
}

function reads_and_writes_static()[statics]: void {
  HasAStatic::$i++;
}
```

```hack
class SomeClass {
  public int $i = 0;
}

function reads_and_writes_prop(SomeClass $sc)[write_props]: void {
  $sc->i++;
}
```

```hack no-extract
function casts_to_dynamic(int $in)[dynamic]: void {
  invokes_off_dynamic($in as dynamic);
}

function invokes_off_dynamic(dynamic $in)[]: void {
  $in->thisIsbananasAndDefinitelyThrowsButTypechecks();
}
```
