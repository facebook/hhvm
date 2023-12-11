<?hh

class MyClass {
  private $x = vec[1,2,3];

  public function closure_fun() :mixed{
    $z = () ==> {
      $foo = () ==> {
        return () ==> {
          $this->x = 12;
          return 12;
        };
      };
      $go = $foo();
      $go();
    };
    return $z;
  }

  public function getter() :mixed{ return $this->x; }
}

function main() :mixed{
  $my = new MyClass;
  var_dump($my->getter());
  $fun = $my->closure_fun();
  $x = $fun();
  var_dump($my->getter());
}


<<__EntryPoint>>
function main_closure_context_001() :mixed{
main();
}
