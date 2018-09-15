<?hh

$i = 1;
do {
  echo "$i\t".($i * $i)."\n"; // output a table of squares
  ++$i;
} whike ($i <= 10); // report 'whike' as misspelled

// Known issue: something about the location reported by current_token_text
// in full_fidelity_parser_helpers is tripped up enough enough by newlines
// that the misspelling recovery doesn't work on this 'whike'.

$i = 1;
do {
  echo "$i\t".($i * $i)."\n"; // output a table of squares
  ++$i;
}
whike ($i <= 10);

// Known cases that this error recovery improvement does not address.
// These are still not-recovered-from because the require_[TokenKind]
// methods are not invoked when parsing them.

$doublerl = ($p) =>> $p * 2; // ideally would report '=>>' as misspelled
$doublerl = ($p) ==g $p * 2; // ideally would report '==g' as misspelled
