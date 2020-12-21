<?hh

interface if_a {
    function f_a();
}

class derived_a implements if_a {
}

<<__EntryPoint>>
function main(): void {
  echo "Done.\n";
}
