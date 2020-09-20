<?hh

class Base  {
  public function overriddenMethod() {
    return "Base::overriddenMethod";
  }
  public function wrapper() {
    return $this->overriddenMethod();
  }
}

class DerivedOne extends Base {
  public function overriddenMethod() {
    return "DerivedOne::overriddenMethod";
  }
}

class DerivedTwo extends Base {
}


<<__EntryPoint>>
function main_attr_unique() {


$inst = new DerivedOne();
printf($inst->wrapper());
}
