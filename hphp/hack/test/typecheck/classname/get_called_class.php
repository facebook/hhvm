<?hh // strict

abstract class C {

  public function foo(): classname<C> {
    $cls = get_called_class();
    hh_show($cls);
    return $cls;
  }
}
