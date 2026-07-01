<?hh

final class C<TBar> {
  public function __construct(private class<TBar> $kls) {}
  public function main(): void {
    $kls = $this->kls;
    new $kls($undefined); // no type error
  }
}
