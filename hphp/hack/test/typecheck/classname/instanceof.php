<?hh // strict

interface I {}
class C_NotI {}

function classname_instanceof(C_NotI $something, classname<I> $name): ?I {
  if ($something instanceof $name) {
    hh_show($something);
    return $something;
  }
  return null;
}

abstract class Test<T as I> {

  protected abstract function getObjType(): classname<T>;

  public function doInstanceof<Tobj>(Tobj $arg): ?T {
    $type = $this->getObjType();
    if ($arg instanceof $type) {
      hh_show($arg);
      return $arg;
    }
    return null;
  }
}
