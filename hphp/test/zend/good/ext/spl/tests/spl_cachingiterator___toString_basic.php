<?hh
<<__EntryPoint>> function main(): void {
$ai = new ArrayIterator(varray[new stdClass(), new stdClass()]);
$ci = new CachingIterator($ai);
var_dump(
$ci->__toString() // if conversion to string is done by echo, for example, an exeption is thrown. Invoking __toString explicitly covers different code.
);
}
