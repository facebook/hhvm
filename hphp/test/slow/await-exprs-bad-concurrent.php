<?hh

<<file:__EnableUnstableFeatures(
  'capture_pipe_variables',
  'allow_extended_await_syntax',
  'allow_conditional_await_syntax',
)>>

<<__EntryPoint>>
async function main() {
  concurrent {
    $x = await g(await g(10)) + await g(await g(20));
    $y = (await g(1) === await g(2)) || (await f(1) === await g(2));

    $bad ??= await f(0);

    $z = await g(100) === 100 ? await f(await id(null) ?? await g(500)) : await g(600);
    $t = await g(1000);
  }

  var_dump($x, $y, $z, $t);
}

