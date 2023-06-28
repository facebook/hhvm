<?hh

function myfunc($a, $b) :mixed{
  error_log($a.$b);
}

class MyClass {
  public $pub;
  protected $pro;
  private $pri;
}
