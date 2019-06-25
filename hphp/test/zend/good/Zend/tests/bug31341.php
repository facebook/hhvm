<?hh <<__EntryPoint>> function main(): void {
$a = array(
    "$     \{    ",
    "      \{   $",
    "      \{$   ",
    "      $\{   ",
    "      \$\{  ",
    "      \{\$  ",
    "\$    \{    ",
    "      \{  \$",
    "%     \{    ");

foreach ($a as $v) {
    echo("'$v'\n");
}
}
