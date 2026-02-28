<?hh

class :foo {
  public function __toString()[] :mixed{
    return "it works";
  }
}


<<__EntryPoint>>
function main_enable_xhp() :mixed{
var_dump(ini_get('hhvm.enable_xhp'));
var_dump((string) <foo />);
}
