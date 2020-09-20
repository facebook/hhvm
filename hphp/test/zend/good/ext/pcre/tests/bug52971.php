<?hh
<<__EntryPoint>>
function main_entry(): void {

  $message = 'Der ist ein Süßwasserpool Süsswasserpool ... verschiedene Wassersportmöglichkeiten bei ...';

  $match = null;
  $pattern = '/\bwasser/iu';
  preg_match_all_with_matches($pattern, $message, inout $match, PREG_OFFSET_CAPTURE);
  var_dump($match);

  $pattern = '/[^\w]wasser/iu';
  preg_match_all_with_matches($pattern, $message, inout $match, PREG_OFFSET_CAPTURE);
  var_dump($match);
}
