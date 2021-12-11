<?hh

function foo(): void {
  $x = ExampleDsl`<foo:bar x="1"><baz /> stuff </foo:bar>`;
}
