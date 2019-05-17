<?hh // strict

class :bar {
  attribute string name;
}

function foo($x = <bar {...<bar name="foo" />} />) { }

<<__EntryPoint>> function main(): void {}
