<?hh

class MyException extends Exception {}

class C {
  <<__InferFlows>>
  public function __construct(
    <<__Policied("PRIVATE")>>
    public int $private,
    <<__Policied("PUBLIC")>>
    public int $public,
  ) { }
}

<<__InferFlows>>
function leak_via_control(C $c, MyException $e): void {
  if ($c->private > 10) {
    throw $e;
  }
  $c->public = 10;
}

<<__InferFlows>>
function leak_pc_in_catch(C $c, MyException $e): void {
  try {
    if ($c->private > 0)
      throw $e;
  } catch (Exception $_) {
    // This leaks PRIVATE via the pc
    $c->public = 12;
  }
}
