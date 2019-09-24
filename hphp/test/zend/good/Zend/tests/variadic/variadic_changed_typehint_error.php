<?hh

interface DB {
    public function query($query, string ...$params);
}

class MySQL implements DB {
    public function query($query, int ...$params) { }
}

<<__EntryPoint>> function main(): void { echo "Done.\n"; }
