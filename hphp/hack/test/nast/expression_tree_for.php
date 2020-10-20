<?hh

function test(): void {
  Code`() ==> { for($i = 0; true; $i = $i + 1) { foo(); } }`;
}
