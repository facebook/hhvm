<?hh
<<__SimpliHack(deriveGetters())>>
 //             ^ at-caret
class SomeClass {
  public int $one;
  public string $word;
  public function foo(): void {}
}

function deriveGetters(): string {
  return "generate getters for all fields";
}
