<?hh

class Good implements HH\LambdaAttribute {
  public function __construct(public int $i) {}
}

class Bad implements HH\ClassAttribute {
  public function __construct(public string $s) {}
}

function test(): void {
  <<Good(4), __ProvenanceSkipFrame>>
  () ==> {};

  <<Bad("hi"), Good('but not really')>> () ==> {};
  <<__Invalid>> async $x ==> null;
}
