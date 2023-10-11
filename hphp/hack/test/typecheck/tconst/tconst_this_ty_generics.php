<?hh

abstract class Box<T> {
  abstract const type TValue;

  final public function getResult(): T {
    throw new Exception();
  }

}

abstract class Box2 extends Box<this::TValue> {}

abstract class ReadResult extends Box2 {}

abstract class TaoQuery<TGen, TGenw as Box<TGen>> {
  abstract const type TResult as Box2;

  final public function genw(): TGenw {
    throw new Exception();
  }

}

abstract class TaoQuery2
  extends TaoQuery<this::TResult::TValue, this::TResult> {}

abstract class TaoReadQuery extends TaoQuery2 {

  abstract const type TResult as ReadResult;

}

class Test {
  public static function genw<T, Tresult as Box<T>>(
    TaoQuery<T, Tresult> $query,
  ): Tresult {
    return $query->genw();
  }
}

abstract class TaoDataType2 {
  abstract const type TQuery as TaoReadQuery;

  abstract protected function createQuery(): this::TQuery;

  final public async function gen(): Awaitable<mixed> {
    $query = $this->createQuery();
    $result = Test::genw($query);
    return $result;
  }

}
