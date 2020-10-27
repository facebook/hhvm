<?hh // strict
interface I1 {
  public function foo(): void;
}
interface I2 {
  public function FOO(): void;
}

// Error, foo defined twice with different casing
abstract class C implements I1, I2 {

}
