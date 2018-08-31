<?hh

trait T {
  public function foo($project = __CLASS__) {
    return $project;
  }
}

class X {
  use T;
}


<<__EntryPoint>>
function main_use_after_free() {
var_dump((new X)->foo());
}
