<?hh

class C {
  public $val;
  public function __debugInfo() {
    return $this->val;
  }
  public function __construct($val) {
    $this->val = $val;
  }
}
<<__EntryPoint>> function main(): void {
$c = new C(fopen("data:text/plain,Foo", 'r'));
var_dump($c);
}
