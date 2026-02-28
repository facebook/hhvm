<?hh
class someClass1 {
  private function someMethod() :mixed{
    //do things
  }
}

class someClass2 extends someClass1 {
  public function __construct() {
    $this->someMethod();
  }
}
<<__EntryPoint>> function main(): void {
$someClass2 = new someClass2;
}
