<?hh

class C {}
trait TR<Tr as ?C> {
  abstract protected function get1(): Tr;
  final public function get2(): Tr {
    $x = $this->get1();
    return $x;
  }
}
