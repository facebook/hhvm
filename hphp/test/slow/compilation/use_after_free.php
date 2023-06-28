<?hh

trait T {
  public function foo($project = __CLASS__) :mixed{
    return $project;
  }
}

class X {
  use T;
}


<<__EntryPoint>>
function main_use_after_free() :mixed{
var_dump((new X)->foo());
}
