<?hh
class Rep {
  public function __invoke() :mixed{
    return "d";
  }
}
class Foo {
  public static function rep($rep) :mixed{
    return "ok";
  }
}
function b() :mixed{
  return "b";
}

<<__EntryPoint>>
function main_preg_replace_callback_array_basic() :mixed{
$count = -1;
var_dump(preg_replace_callback_array(
  dict[
    "/a/" => 'b',
    "/b/" => function () { return "c"; },
    "/c/" => new Rep,
    '/d/' => vec["Foo", "rep"]], 'a', -1, inout $count));
var_dump(preg_replace_callback_array(
  dict[
    "/a/" => 'b',
    "/c/" => new Rep,
    "/b/" => function () { return "ok"; },
    '/d/' => vec["Foo", "rep"]], 'a', -1, inout $count));
var_dump(preg_replace_callback_array(
  dict[
    '/d/' => vec["Foo", "rep"],
    "/c/" => new Rep,
    "/a/" => 'b',
    "/b/" => $_ ==> 'ok',
  ], 'a', -1, inout $count));
var_dump($count);
}
