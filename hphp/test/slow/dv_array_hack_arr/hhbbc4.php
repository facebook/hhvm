<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class A {
  private static function get(): varray<string> {
    return varray['SP', 'PP', 'SP2', 'PP2', 'I'];
  }

  public async function gen(): Awaitable<darray> {
    $x = darray[];
    foreach (self::get() as $t) $x[$t] = 'N/A';
    return $x;
  }
}


<<__EntryPoint>>
function main_hhbbc4() :mixed{
$a = new A;
var_dump(\HH\Asio\join($a->gen()));
}
