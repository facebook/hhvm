<?hh

trait NotActuallyReusedTrait {
  final public function foo(): void {}
}

trait TraitWithRequirement {
  require extends MyParent;
}

class MyParent {
  use NotActuallyReusedTrait;
}


class MyChild extends MyParent {
  use TraitWithRequirement;
}

<<__EntryPoint>>
function my_main(): void {
  new MyChild();
}
