<?hh
class someClass1 {
  protected static function someMethod() :mixed{
    //do things
  }
}

class someClass2 {
  public function __construct() {
    someClass1::someMethod();
  }
}
<<__EntryPoint>> function main(): void {
$someClass2 = new someClass2;
}
