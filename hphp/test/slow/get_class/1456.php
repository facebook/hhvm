<?hh

abstract class bar {
  public function __construct()  {
    var_dump(get_class($this));
    var_dump(get_class());
  }
}
class foo extends bar {
}

<<__EntryPoint>>
function main_1456() :mixed{
new foo;
}
