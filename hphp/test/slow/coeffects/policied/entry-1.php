<?hh

function policied_of()[policied_of]: void {
  echo "in policied_of\n";
}

<<__EntryPoint>>
function main() {
  policied_of();
  HH\Coeffects\enter_policied_of(policied_of<>);
}
