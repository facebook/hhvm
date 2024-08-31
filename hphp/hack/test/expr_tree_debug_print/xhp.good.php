<?hh
<<file:__EnableUnstableFeatures('expression_trees')>>
class :abc {
  public function __construct(
  ) {
  }
}

class :def {
  public function __construct(
  ) {
  }
}

function f(): void {
  ExampleDsl`<abc> <def /> </abc>`;
}
