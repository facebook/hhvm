<?hh

<<file:__EnableUnstableFeatures('require_constraint')>>

// `require this as C` and `require extends C` for the same target class are
// compatible (both are subtype constraints) and must not be reported as
// "Conflicting requirements". Here ReqChild inherits `require this as ReqBase`
// from its parent (which uses ReqBaseTrait) while also using ReqChildTrait,
// which declares `require extends ReqBase`.

trait ReqBaseTrait {
  require this as ReqBase;
}

class ReqBase {
  use ReqBaseTrait;
}

trait ReqChildTrait {
  require extends ReqBase;
}

class ReqChild extends ReqBase {
  use ReqChildTrait;

  public function hello(): void { echo "hello\n"; }
}


<<__EntryPoint>>
function main() : void {
  (new ReqChild())->hello();
}
