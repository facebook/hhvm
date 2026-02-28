<?hh
class C {}

class D extends C implements Iterator {

  private $counter = 2;

  public function valid() :mixed{
    echo __METHOD__ . "($this->counter)\n";
    return $this->counter;
  }

  public function next() :mixed{
    $this->counter--;
    echo __METHOD__ . "($this->counter)\n";
  }

  public function rewind() :mixed{
    echo __METHOD__ . "($this->counter)\n";
  }

  public function current() :mixed{
    echo __METHOD__ . "($this->counter)\n";
  }

  public function key() :mixed{
    echo __METHOD__ . "($this->counter)\n";
  }

}
<<__EntryPoint>> function main(): void {
foreach (new D as $x) {}
}
