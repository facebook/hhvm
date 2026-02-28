<?hh

// This class exists so that our tests can take a B_DefWrapper instead of a B_Def.
// Doing the latter would just make the tests get selected immediately.
class B_DefWrapper {

  public function __construct(B_Def $b) {
    $this->b = $b;
  }

  public B_Def $b;

}
