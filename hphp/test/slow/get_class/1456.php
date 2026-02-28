<?hh

abstract class bar {
  public function __construct()  {
    var_dump(get_class($this));
  }
}
class foo extends bar {
}

<<__EntryPoint>>
function main_1456() :mixed{
new foo;
}
