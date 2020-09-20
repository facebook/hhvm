<?hh <<__EntryPoint>> function main(): void {
$foo = 'bla bla bla';

  $m = null;
  var_dump(preg_match_with_matches(
    '/(?<!\w)(0x[\p{N}]+[lL]?|[\p{Nd}]+(e[\p{Nd}]*)?[lLdDfF]?)(?!\w)/',
    $foo,
    inout $m,
  ));
  var_dump($m);
}
