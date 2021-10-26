<?hh

interface MyPlainSprintf {
  public function format_s(mixed $s): ~string;
}

function my_sprintf(HH\FormatString<MyPlainSprintf> $f, mixed ...$_): string {
  return 'hi';
}

function f(): void {
  my_sprintf('%s', 'hi');
}
