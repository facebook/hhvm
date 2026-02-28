<?hh

interface IParent<T> {
  abstract const type TKey;
  public function getParent(): T;
}

interface IChild<T> extends IParent<T> {
  public function getChild(): T;
}

function test(IParent<int> with { type TKey = string } $x): int {
  if ($x is IChild<_>) {
    return $x->getChild();
  }
  return 0;
}
