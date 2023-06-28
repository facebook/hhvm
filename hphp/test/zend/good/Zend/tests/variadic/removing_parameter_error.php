<?hh

/* Theoretically this should be valid because it weakens the constraint, but
 * PHP does not allow this (for non-variadics), so I'm not allowing it here, too,
 * to stay consistent. */

interface DB {
    public function query($query, ...$params):mixed;
}

class MySQL implements DB {
    public function query(...$params) :mixed{ }
}

<<__EntryPoint>>
function main(): void {
  echo "Done.\n";
}
