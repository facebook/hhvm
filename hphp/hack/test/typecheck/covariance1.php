<?hh // partial

class Ref<T> implements ReadRef<T>, WriteRef<T> {
  public function __construct(private T $x) {}

  public function set(T $x): void { $this->x = $x; }
  public function get(): T { return $this->x; }
}

interface ReadRef<+T> {
  public function get(): T;
}

interface WriteRef<-T> {
  public function set(T $x): void;
}

class A {}
class B extends A {}

function readA(ReadRef<A> $x): void {}
function readB(ReadRef<B> $x): void {}
function writeA(WriteRef<A> $x): void {}
function writeB(WriteRef<B> $x): void {}

function test(
  Ref<A> $refA,
  Ref<B> $refB,
  ReadRef<A> $readA,
  ReadRef<B> $readB,
  WriteRef<A> $writeA,
  WriteRef<B> $writeB,
): void {
  readA($readA);
  readA($readB);
  readA($refA);
  readA($refB);
  writeB($writeA);
  writeB($writeB);
  writeB($refA);
  writeB($refB);
}
