<?hh // strict

async function f(
  $x = async () ==> {
    list($a, $b) = await $x;
    yield 3; },
  $y = () ==> {
    yield from 3; },
  $z = () ==> {
    yield break; },

  $wrong = await 3,
) {}

function g($x) {}
