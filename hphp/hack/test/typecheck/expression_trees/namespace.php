<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

namespace Bar {
  class CodeChild extends \Code {}
}

namespace {

function test(): void {
    Bar\CodeChild`1`;
}





}
