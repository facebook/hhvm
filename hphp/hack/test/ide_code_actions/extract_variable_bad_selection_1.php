<?hh

class Klass {
  public function foo(): void {
    // Bad selection because the selected nodes don't have a common root.
    // In such cases, Kotlin and TypeScript also do not provide "extract into variable"
    100 * /*range-start*/200 + 300/*range-end*/;
  }
}
