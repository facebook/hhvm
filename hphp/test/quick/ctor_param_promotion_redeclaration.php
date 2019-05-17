<?hh

//
// field redeclaration, error
//
class A {
  public $c;
  public function __construct(protected $c) {}
}

<<__EntryPoint>> function main(): void {}
