<?hh
<<__EntryPoint>> function main(): void {
$s = shape(
  // declaring field as optional is possible only in type annotation
  ?'aaaa' => 3,
);
}
