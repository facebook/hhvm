<?hh
<<__EntryPoint>> function main(): void {
$x = 1;

(++$x) as int;
var_dump($x); // 2

try {
  (++$x) as string;
} catch (Exception $_) {}
var_dump($x); // 3
}
