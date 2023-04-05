<?hh

class :foo:bar {
  attribute string my-required-attribute @required;
  attribute string my-attribute;
}

function main(): void {
  <foAUTO332 my-attribute="hello" />;
}
