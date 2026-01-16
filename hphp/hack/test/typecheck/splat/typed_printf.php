<?hh


interface I {
  public function format_s(string $s): string;
  public function format_d(int $d): string;
}

function myprintf<Ta as (mixed...)>(
  HH\TypedFormatString<I, Ta> $format_string,
  ... Ta $format_args,
): void {}

<<__NoAutoLikes>>
function make_my_formatstring<Ta as (mixed...)>(
  HH\TypedFormatString<I, Ta> $fs,
): HH\TypedFormatString<I, Ta> {
  return $fs;
}

<<__NoAutoLikes>>
function make_formatstring<Ta as (mixed...)>(
  HH\TypedFormatString<PlainSprintf, Ta> $fs,
): HH\TypedFormatString<PlainSprintf, Ta> {
  return $fs;
}

function test1(): void {
  myprintf("Test %s with %d", "ABC", 23);
  myprintf("Test %s with %d", 2, false);
  myprintf("Test %s with %d", 'a');
  $fs = make_my_formatstring("Test %s with %d");
}
function test2(): void {
  $fs = make_formatstring("String %s integer %d float %g");
  printf($fs, 23, "A", 23);
  printf($fs, 42);
}

function test_concat(): void {
  $fs = make_my_formatstring("String %s"." integer %d");
  myprintf($fs, "A", 23);
  myprintf($fs, false, 2.3);
}

function bad_concat(): void {
  myprintf("Test %"."s", "ABC");
}
