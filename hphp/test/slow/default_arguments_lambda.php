<?hh

<<file:__EnableUnstableFeatures(
  'capture_pipe_variables',
  'allow_extended_await_syntax',
  'allow_conditional_await_syntax',
)>>

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
