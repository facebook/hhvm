<?hh

class GoodExample {
  public function __construct(public int $p) {}
}

class AlsoGoodBecauseNotOverlapping {
  public int $p;
  public function __construct(int $p, public int $q) {
    $this->p = $p;
  }
}

class DuplicateDeclarations {
  public int $p;
  public function __construct(public int $p) {}
}

trait PromoInTrait {
  public function __construct(public int $p) {}
}

interface PromoInInterface {
  public function __construct(public int $p);
}

abstract class PromoInAbstractConstruct {
  public abstract function __construct(public int $p) {}
}

class PromoInMethod {
  public function foo(public int $p): void {}
}
