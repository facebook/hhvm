<?hh
<<__EntryPoint>> function main(): void {
$test = "foo";
$var = prefix<<<"MYLABEL"
test: $test
MYLABEL;
echo $var;
}
