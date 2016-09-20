<?hh // strict

abstract class MyList<T> implements Traversable<T> {
  private bool $isSet = false;

  public abstract function cons(T $x): MyList<T>;
  public abstract function head(): T;
  public abstract function tail(): MyList<T>;

  public function compact<Tinner>(): MyList<Tinner> where T = ?Tinner {
    throw new Exception('UNIMPLEMENTED!');
  }

  public static function compactStatic<Tinner>(
    MyList<?Tinner> $xs,
  ): MyList<Tinner> {
    return $xs->compact();
  }

  public static function fromTraversable(Traversable<T> $t): MyList<T> {
    $ret = new MyNil();
    foreach ($t as $x) {
      $ret->cons($x);
    }
    return $ret;
  }

}

class MyNil<T> extends MyList<T> {
  public function cons(T $x): MyList<T> {
    return new MyCons($x, $this);
  }

  public function head(): T {
    throw new Exception('Can not consume from an empty list');
  }

  public function tail(): MyList<T> {
    throw new Exception('Can not consume from an empty list');
  }

  public function compact<Tinner>(): MyList<Tinner> where T = ?Tinner {
    return new MyNil();
  }

}

class MyCons<T> extends MyList<T> {
  public function __construct(
    private T $head,
    private MyList<T> $tail,
  ) {}

  public function cons(T $x): MyList<T> {
    return new MyCons($x, $this);
  }

  public function head(): T {
    return $this->head;
  }

  public function tail(): MyList<T> {
    return $this->tail;
  }

  public function compact<Tinner>(): MyList<Tinner> where T = ?Tinner {
    $compact_tail = $this->tail->compact();
    return null === $this->head
      ? $compact_tail
      : $compact_tail->cons($this->head);
  }
}

class Tester {
  public static function testCall(): Vector<int> {
    $l = MyList::fromTraversable(array(42,null));
    $cle = $l->compact()->head();
    $csle = MyList::compactStatic($l)->head();
    return Vector {$cle, $csle};
  }
}
