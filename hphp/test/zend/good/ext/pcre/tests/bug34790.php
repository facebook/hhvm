<?hh
function func1():mixed{
        $string = 'what the word and the other word the';
  $matches = null;
  preg_match_all_with_matches('/(?P<word>the)/', $string, inout $matches);
  return $matches['word'];
}
<<__EntryPoint>> function main(): void {
$words = func1();
var_dump($words);
}
