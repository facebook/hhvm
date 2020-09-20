<?hh

class MyException extends Exception {}

class C {
  public function __construct(
    <<Policied("PRIVATE")>>
    public int $private,
    <<Policied("PUBLIC")>>
    public int $public,
  ) { }
}

function leak_via_control(C $c, MyException $e): void {
  if ($c->private > 10) {
    throw $e;
  }
  $c->public = 10;
}

function leak_pc_in_catch(C $c, MyException $e): void {
  try {
    if ($c->private > 0)
      throw $e;
  } catch (Exception $_) {
    // This leaks PRIVATE via the pc
    $c->public = 12;
  }
}
