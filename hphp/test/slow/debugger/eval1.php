<?hh

class cls {
  public $pub = 0;
  private $pri = 0;
  public function __construct() {
    $this->pub = 11;
    $this->pri = 12;
  }
  public function meth($x) :mixed{
    $a = $this->pub.':'.$this->pri;
    $b = $this->pub.':'.$this->pri;
    $c = $this->pub.':'.$this->pri;
    return $a.'-'.$b.'-'.$c;
  }
}

<<__EntryPoint>> function main() :mixed{
  error_log('eval1.php loaded');
}
