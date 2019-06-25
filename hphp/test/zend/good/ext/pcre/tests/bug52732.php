<?hh <<__EntryPoint>> function main(): void {
$ret = preg_match('/(?:\D+|<\d+>)*[!?]/', 'foobar foobar foobar');

var_dump($ret);
}
