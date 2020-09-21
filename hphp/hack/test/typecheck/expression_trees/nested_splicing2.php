<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

final class Code {}

function test(): void {
  Code`__splice__(1 + __splice__(4))`;
}
