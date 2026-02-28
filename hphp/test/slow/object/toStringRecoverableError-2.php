<?hh
class C {
  public function __toString()[] :mixed{
    return 123;
  }
}
function my_handler($errno, $errmsg) :mixed{
  echo "my_handler called\n";
  var_dump($errno);
  var_dump($errmsg);
}
function main() :mixed{
  set_error_handler(my_handler<>);
  $obj = new C;
  $str = (string)$obj;
  echo "Result: ";
  var_dump($str);
}

<<__EntryPoint>>
function main_to_string_recoverable_error_2() :mixed{
main();
}
