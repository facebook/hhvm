<?hh

function foo((function(): int) $f): void {}

class A {
  public function main(): void {
    foo(() ==>
      // bug: We SHOULD offer "extract into method" for the next line
      /*range-start*/3 + 3/*range-end*/
    );
  }
}
