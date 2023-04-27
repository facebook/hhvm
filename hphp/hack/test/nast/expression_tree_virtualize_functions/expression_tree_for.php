<?hh

function test(): void {
  ExampleDsl`() ==> {
    for ($i = 0; true; $i = $i + 1) {
      foo();
    }
  }`;
}
