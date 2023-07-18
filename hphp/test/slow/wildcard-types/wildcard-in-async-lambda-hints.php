<?hh

<<__EntryPoint>>
async function main():Awaitable<void> {
  $f = async (vec<_> $x) ==> $x[0];
  $g = async (int $x) : Awaitable<vec<_>> ==> vec[$x];
  $h = async (int $x) : Awaitable<_> ==> $x;

  $a = await $f(vec[2]);
  var_dump($a);
  $b = await $g(3);
  var_dump($b);
  $c = await $h(4);
  var_dump($c);
}
