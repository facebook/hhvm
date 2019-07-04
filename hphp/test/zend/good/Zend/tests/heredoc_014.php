<?hh
<<__EntryPoint>> function main(): void {
$test = "foo";
$var = <<<"MYLABEL
test: $test
MYLABEL;
echo $var;
}
