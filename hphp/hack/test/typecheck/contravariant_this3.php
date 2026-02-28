<?hh

final class C<-T> {
  public function __construct(private T $v) {}
  // We'll raise an error on the use of 'this' in the argument
  // position, but for now the contravariant reference to 'this'
  // as the return type is accepted.
  public function id(this $p): this {
    return $p;
  }
}
