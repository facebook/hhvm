<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

final class Code {}

function test(): void {
  Code`__splice__(4) + __splice__(5)`;
}
