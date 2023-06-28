<?hh
class mom {

  protected $prot = "protected property\n";

  protected function prot() :mixed{
    print "protected method\n";
  }
}

class child extends mom {

  public function callMom() :mixed{
    $this->prot();
  }

  public function viewMom() :mixed{
    print $this->prot;
  }

}
<<__EntryPoint>> function main(): void {
$c = new child();
$c->callMom();
$c->viewMom();
}
