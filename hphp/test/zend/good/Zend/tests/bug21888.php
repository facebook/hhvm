<?hh
class mom {

  protected $prot = "protected property\n";

  protected function prot() {
    print "protected method\n";
  }
}

class child extends mom {

  public function callMom() {
    $this->prot();
  }

  public function viewMom() {
    print $this->prot;
  }

}
<<__EntryPoint>> function main(): void {
$c = new child();
$c->callMom();
$c->viewMom();
}
