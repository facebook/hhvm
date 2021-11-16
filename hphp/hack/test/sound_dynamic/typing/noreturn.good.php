<?hh

<<file:__EnableUnstableFeatures('upcast_expression')>>

function f((function (): noreturn) $f): void {
  $f upcast (function (): dynamic);
}
