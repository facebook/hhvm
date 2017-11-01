<?hh

trait T {
  public function foo($project = __CLASS__) {
    return $project;
  }
}

class X {
  use T;
}

var_dump((new X)->foo());
