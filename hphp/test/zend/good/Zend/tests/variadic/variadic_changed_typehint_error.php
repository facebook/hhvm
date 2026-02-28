<?hh

interface DB {
    public function query($query, string ...$params):mixed;
}

class MySQL implements DB {
    public function query($query, int ...$params) :mixed{ }
}

<<__EntryPoint>> function main(): void { echo "Done.\n"; }
