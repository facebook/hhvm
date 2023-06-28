<?hh

class Base  {
  public function overriddenMethod() :mixed{
    return "Base::overriddenMethod";
  }
  public function wrapper() :mixed{
    return $this->overriddenMethod();
  }
}

class DerivedOne extends Base {
  public function overriddenMethod() :mixed{
    return "DerivedOne::overriddenMethod";
  }
}

class DerivedTwo extends Base {
}


<<__EntryPoint>>
function main_attr_unique() :mixed{


$inst = new DerivedOne();
printf($inst->wrapper());
}
