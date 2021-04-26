<?hh

class :foo:bar {
  attribute enum {"firefly", 1} my-attribute;
}

function main(): void {
  <foo:bar my-attribute={AUTO332}
}
