<?hh // partial

interface IParent {
  const CBAR = 'bar';
}

abstract class CParent implements IParent {
  protected function foo(): string {
    return __CLASS__;
  }
}

abstract class Kid extends CParent {
  protected function bar() {}
}

trait KidTrait {
  require extends Kid;
  require extends CParent;

  protected function foo(): string {
    takes_parent($this);
    takes_iparent($this);
    takes_kid($this);
    takes_kt($this);
    parent::bar();
    return 'wrapped('.parent::foo().')';
  }
}

function takes_kt(Vector<KidTrait> $kt): void {}
