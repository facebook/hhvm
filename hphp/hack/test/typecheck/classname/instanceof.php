<?hh // strict

interface I {}
class C_NotI {}

function classname_instanceof(C_NotI $something, classname<I> $name): ?I {
  if ($something instanceof $name) {
    return $something;
  }
  return null;
}

abstract class Test<T as I> {

  protected abstract function getObjType(): classname<T>;

  public function doInstanceof<Tobj>(Tobj $arg): ?T {
    $type = $this->getObjType();
    if ($arg instanceof $type) {
      return $arg;
    }
    return null;
  }
}
