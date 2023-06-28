<?hh

trait A {
  function b() :mixed{
    $c = function() {
      return 'd';
    };
    var_dump($c);
    return $c();
  }
}

class E { use A; }
class F { use A; }

<<__EntryPoint>> function main(): void {
  var_dump((new E)->b());
  var_dump((new F)->b());
}
