<?hh

final class C<T1> {
  public function __construct(private classname<T1> $kls1) {}

  public function main(): int {
    $kls = $this->kls1;
    return new $kls();
  }
}
