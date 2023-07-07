<?hh

function foo((function(): int) $f): void {}

class A {
  public function main(): void {
    foo(() ==>
      // We offer a refactor even though the selection
      // corresponds to a `return` statement in the tast.
      /*range-start*/3 + 3/*range-end*/
    );
  }
}
