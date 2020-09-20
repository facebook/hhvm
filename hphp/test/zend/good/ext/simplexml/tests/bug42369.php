<?hh
<<__EntryPoint>> function main(): void {
    $xml = '<?xml version="1.0" encoding="utf-8"?>';
    $x = simplexml_load_string($xml . "<q><x>foo</x></q>");

    echo 'explicit conversion' . PHP_EOL;
    for ($i = 0; $i < 100000; $i++) {
        md5(strval($x->x));
    }

    echo 'no conversion' . PHP_EOL;
    for ($i = 0; $i < 100000; $i++) {
        md5((string)$x->x);
    }
    echo "===DONE===\n";
}
