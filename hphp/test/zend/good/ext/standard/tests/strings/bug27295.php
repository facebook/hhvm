<?hh <<__EntryPoint>> function main(): void {
$strings = varray["foo = bar", "bar = foo"];
foreach( $strings as $string )
{
    list($var, $val) = sscanf( $string, "%s = %[^[]]");
    echo "$var = $val\n";
}
}
