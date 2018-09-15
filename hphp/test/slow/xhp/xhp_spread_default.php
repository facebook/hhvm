<?hh // strict

class :bar {
  attribute string name;
}

function foo($x = <bar {...<bar name="foo" />} />) { }
