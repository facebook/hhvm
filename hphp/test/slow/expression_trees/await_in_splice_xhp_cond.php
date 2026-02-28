<?hh

<<file:__EnableUnstableFeatures(
  'capture_pipe_variables',
  'allow_extended_await_syntax',
  'allow_conditional_await_syntax',
)>>

async function f(): Awaitable<void> {
  true ? <xhp x={BKS2`${await q()}`} /> : false;
}
