<?hh

class Super {
  public function foo(): int {
    if ($this instanceof IMarked) {
      return $this->methodOfMarked();
    }
    return 0;
  }

  public function methodOfSuper(): string {
    return 'foo';
  }
}

interface IMarked {
  // require extends Super;

  public function methodOfMarked(): int;
}

function f_marked(IMarked $inst) {
  $inst->methodOfSuper();
  f_super($inst);
  $inst->methodOfMarked();
  f_marked2($inst);
}

function f_super(Super $super): void {}
function f_marked2(IMarked $inst): void {}
