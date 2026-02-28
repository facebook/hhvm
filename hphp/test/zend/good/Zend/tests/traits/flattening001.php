<?hh

trait T1 {
  public function getText() :mixed{
    return $this->text;
  }
}

trait T2 {
  public function setTextT2($val) :mixed{
    $this->text = $val;
  }
}

class TraitsTest {
  use T1;
  use T2;
  private $text = 'test';
  public function setText($val) :mixed{
    $this->text = $val;
  }
}
<<__EntryPoint>> function main(): void {
error_reporting(E_ALL);

$o = new TraitsTest();
var_dump($o->getText());

$o->setText('foo');

var_dump($o->getText());

$o->setText('bar');

var_dump($o->getText());
}
