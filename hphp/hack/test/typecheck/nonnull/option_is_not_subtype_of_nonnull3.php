<?hh // strict

class C<T> {
  private ?T $value;

  // This passes the type-checker if nonnull is replaced with mixed.
  public function get(): nonnull {
    return $this->value;
  }
}
