<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function test(): void {
  ExampleDsl`{}`;

  $code = ExampleDsl`{
    2 < 3;
    return 5;
  }`;

  ExampleDsl`{
    if(${$code} + 1 < 10) {
      return 'Yes';
    } else {
      return 'No';
    }
  }`;
}
