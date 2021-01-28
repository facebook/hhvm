<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function test(): void {
  Code`() ==> {
    2 < 3;
    2 <= 3;
    2 > 3;
    2 >= 3;
    2 === 3;
    2 !== 3;
  }`;
}
