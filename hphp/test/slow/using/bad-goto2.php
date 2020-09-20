<?hh

function main() {
  goto foo;
  using ($x);
  foo:
}

<<__EntryPoint>> function main_entry(): void { echo "Done.\n"; }
