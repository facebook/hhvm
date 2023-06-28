<?hh

class C {
  public $val;
  public function __debugInfo() :mixed{
    return $this->val;
  }
  public function __construct($val) {
    $this->val = $val;
  }
}
<<__EntryPoint>> function main(): void {
$c = new C(1.0);
var_dump($c);
}
