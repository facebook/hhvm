<?hh

class :bar {
  attribute string name;
}

function foo($x = <bar {...<bar name="foo" />} />) :mixed{ }

<<__EntryPoint>> function main(): void { echo "Done.\n"; }
