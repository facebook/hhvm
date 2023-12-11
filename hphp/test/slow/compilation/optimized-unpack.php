<?hh

function foo($a, $b, $c) :mixed{ var_dump($a, $b, $c); }

class X {
  const FOO=vec[1,2,3];
  static function bar() :mixed{
    foo(...self::FOO);
  }
}

<<__EntryPoint>>
function main_optimized_unpack() :mixed{
X::bar();
}
