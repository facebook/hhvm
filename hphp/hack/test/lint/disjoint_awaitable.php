<?hh

class ExpectObj<T> {
  public function __construct(private T $item) { }
  public function toEqual<<<__NonDisjoint>> T1, <<__NonDisjoint>> T2>(T2 $x):bool where T = T1 {
    return $this->item === $x;
  }
}

function expect<T>(T $x):ExpectObj<T> {
  return new ExpectObj($x);
}
final class MyTest {
  private async function genInt(): Awaitable<int> {
    return 3;
  }
  public function testIt(): void {
    $x = $this->genInt();
    expect($x)->toEqual(3);
  }
}
