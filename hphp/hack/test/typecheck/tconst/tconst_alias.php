<?hh // strict

interface Bar {
  abstract const type TData;
  abstract public function this(): this;
  public function getData(): this::TData;
}

abstract class Test {
  abstract const type TBar as Bar;
  const type TAlias = this::TBar;
  const type TData = this::TAlias::TData;

  abstract public function get(): this::TAlias;

  public function getData(): this::TData {
    $x = $this->get();
    hh_show($x->this()->getData());
    return $x->getData();
  }
}
