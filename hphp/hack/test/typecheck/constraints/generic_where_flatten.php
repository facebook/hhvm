<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

abstract class MyList<T> {

  public abstract function Append(MyList<T> $that): MyList<T>;
  public abstract function Flatten<Tu>(): MyList<Tu> where T = MyList<Tu>;
  public abstract function Sum(): num where T as num;
}

class Nil<Ta> extends MyList<Ta> {
  public function Flatten<Tu>(): MyList<Tu> where Ta = MyList<Tu> {
    return new Nil();
  }
  public function Append(MyList<Ta> $that): MyList<Ta> {
    return $that;
  }
  public function Sum(): num {
    return 0;
  }
}

class Cons<Ta> extends MyList<Ta> {
  public function __construct(private Ta $head, private MyList<Ta> $tail) { }
  public function Append(MyList<Ta> $that): MyList<Ta> {
    return new Cons($this->head, $this->tail->Append($that));
  }
  public function Flatten<Tu>(): MyList<Tu> where Ta = MyList<Tu> {
    $h = $this->head;
    $t = $this->tail;
    $ft = $t->Flatten();
    $r = $h->Append($ft);
    return $r;
  }
  public function Sum(): num where Ta as num {
    return $this->head + $this->tail->Sum();
  }
}

function Copies<T>(int $n, T $item): MyList<T> {
  if ($n === 0) return new Nil();
  else return new Cons($item, Copies($n-1, $item));
}


function TestIt():void {
  $l = Copies(5, 0.5);
  $ll = Copies(2, $l);
  $r = $ll->Flatten();
  $n = $r->Sum();
  echo (string) $n;
}

function main(): void {
  TestIt();
}
