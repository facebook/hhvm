<?hh

class C<T as this> {
  private ?T $x;
}

abstract class C<T as this::X> {
  abstract const type X;
}
