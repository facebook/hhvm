<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

function foo(int $x, named int $a, named int $b, named int $c) {
    var_dump(vec[$x, $a, $b, $c]);
    bar($x, a=$a, b=$b);
}


function bar(int $x, named int $a, named int $b) {
    var_dump(vec[$x, $a, $b]);
    baz($x, a=$a);
}


function baz(int $x, named int $a) {
    var_dump(vec[$x, $a]);
}

class C {
  public function dispatch(named int $a, named int $b, named int $c) {
    $this->foo(a=$a, b=$b, c=$c);
  }

  function foo(named int $a, named int $b, named int $c) {
    var_dump(vec[$a, $b, $c]);
    $this->bar(a=$a, b=$b);
  }

  function bar(named int $a, named int $b) {
    var_dump(vec[$a, $b]);
    $this->baz(a=$a);
  }

  function baz(named int $a) {
    var_dump(vec[$a]);
  }
}

class D extends C {
   function foo(named int $a, named int $b, named int $c) {
    $a += 100;
    $b += 100;
    $c += 100;
    var_dump(vec[$a, $b, $c]);
    $this->bar(a=$a, b=$b);
  }

  function bar(named int $a, named int $b) {
    $a += 100;
    $b += 100;
    var_dump(vec[$a, $b]);
    $this->baz(a=$a);
  }

  function baz(named int $a) {
    $a += 100;
    var_dump(vec[$a]);
  }
}

<<__EntryPoint>>
function main() {
    foo(42, a=1, b=2, c=3);
    (new C())->dispatch(a=1, b=2, c=3);
    (new D())->dispatch(a=1, b=2, c=3);
}
