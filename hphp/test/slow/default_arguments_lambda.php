<?hh

async function f(
  $x = async () ==> {
    list($a, $b) = await $x;
    yield 3; },
  $y = () ==> {
    yield 3; },
  $z = () ==> {
    yield break; },

  $wrong = await 3,
) :Awaitable<mixed>{}

function g($x) :mixed{}
