<?hh // strict

<<__PPL>>
class MyClass {
  public function myMethod(): this {
    return $this;
  }

  public function error(): void {
    $this->myMethod()->myMethod();
  }
}
