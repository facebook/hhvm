<?hh <<__EntryPoint>> function main(): void {
$a = varray[
    "$     \{    ",
    "      \{   $",
    "      \{$   ",
    "      $\{   ",
    "      \$\{  ",
    "      \{\$  ",
    "\$    \{    ",
    "      \{  \$",
    "%     \{    "];

foreach ($a as $v) {
    echo("'$v'\n");
}
}
