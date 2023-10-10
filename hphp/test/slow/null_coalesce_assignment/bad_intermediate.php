<?hh

class Foo {
  const bar = 'I am a constant';
}

function f(): void {
  try {
    $arr = darray[
      4 => darray[
        2 => darray[],
        (string)$z => darray[],
      ],
    ];
    $arreq = $arr;
    $x = 4;

    var_dump(
      ($arr[$x][(string)$z][Foo::bar] ??= 'Yay!') === 'Yay!',
    ); // $z warns
    var_dump($z === NULL); // $z doesn't get set
    $arreq[$x][(string)$z][Foo::bar] = 'Yay!'; // Note the `=`
    var_dump($z === NULL); // $z doesn't get set
    var_dump($arr === $arreq); // $arr and $arreq get set consistently

  } catch (UndefinedVariableException $e) {
    var_dump($e->getMessage());
  }
  $obj = new Foo;
  var_dump(($obj->foo ??= 42) === 42); // Warns but sets intermediates
  var_dump($obj);
  $objeq = new Foo;
  $objeq->foo = 42; // Consistent with equals
  var_dump($obj == $objeq);
}


<<__EntryPoint>>
function main_bad_intermediate() :mixed{
  f();
}
