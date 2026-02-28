<?hh <<__EntryPoint>> function main(): void {
$a = vec[
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
