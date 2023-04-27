<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function test(): void {
  ExampleDsl`() ==> {
    2 < 3;
    2 <= 3;
    2 > 3;
    2 >= 3;
    2 === 3;
    2 !== 3;
  }`;
}
