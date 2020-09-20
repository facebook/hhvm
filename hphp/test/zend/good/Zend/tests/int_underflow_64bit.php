<?hh
<<__EntryPoint>> function main(): void {

foreach (range(0, 4) as $d) {
        $l = (int)(PHP_INT_MIN - $d);
        var_dump($l);
}

echo "Done\n";
}
