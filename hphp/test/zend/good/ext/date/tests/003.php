<?hh <<__EntryPoint>> function main(): void {
date_default_timezone_set('UTC');

for ($i = 0; $i < 32; $i++) {
    var_dump(date("jS", mktime(0,0,0, 1, $i, 2006)));
}

echo "Done\n";
}
