<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

class Code {}

function foo(): void {
  $e = Code`__splice__(1)`;
}

<<__EntryPoint>>
function main(): void {}
