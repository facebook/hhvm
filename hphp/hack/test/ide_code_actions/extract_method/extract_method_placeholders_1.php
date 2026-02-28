<?hh

class Klass {
  public function method0(): void {}
  public static async function method1(): Promise<void> {}
  public function method2(): void {
    // extracted method should be called "method3" because 0-2 inclusive are taken
    /*range-start*/
    $x = 1;
    /*range-end*/
  }
}
