<?hh

class B<T> {}

class C extends B<this> {}

// Ok, not generic
final class D extends B<this> {}

// Not ok, subtyping possible by variance
final class E<+T> extends B<this> {}

// Not ok, subtyping possible by variance
final class F<-T> extends B<this> {}

// Ok, invariant
final class G<T> extends B<this> {
  public ?T $x
}
