<?hh

class C<T> {}

interface I {
  require extends C<this::TUndefined>;
}
