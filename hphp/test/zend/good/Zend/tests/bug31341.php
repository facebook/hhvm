<?hh <<__EntryPoint>> function main() {
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
