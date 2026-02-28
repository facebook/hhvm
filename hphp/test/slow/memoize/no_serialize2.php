<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class C1 {
  <<__Memoize>>
  public function func1() :mixed{
    echo "C1::func1\n";
    return 123;
  }
}

class C2 {
  <<__Memoize>>
  public function func1() :mixed{
    echo "C2::func1\n";
    return 'abc';
  }

  <<__Memoize>>
  public function func2() :mixed{
    echo "C2::func2\n";
    return 'def';
  }
}

function test() :mixed{
  $a1 = new C1;
  var_dump($a1->func1());

  $a2 = unserialize(serialize($a1));
  var_dump($a2->func1());

  $b1 = new C2;
  var_dump($b1->func1());
  var_dump($b1->func2());

  $b2 = unserialize(serialize($b1));
  var_dump($b2->func1());
  var_dump($b2->func2());
}

<<__EntryPoint>>
function main_no_serialize2() :mixed{
test();
}
