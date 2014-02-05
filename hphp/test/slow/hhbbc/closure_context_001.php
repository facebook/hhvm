<?hh

class MyClass {
  private $x = array(1,2,3);

  public function closure_fun() {
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

  public function getter() { return $this->x; }
}

function main() {
  $my = new MyClass;
  var_dump($my->getter());
  $fun = $my->closure_fun();
  $x = $fun();
  var_dump($my->getter());
}

main();
