<?hh // $Id$

<<__EntryPoint>>
function entrypoint_with_strings(): void {
error_reporting(E_ALL);

// This FAILS
$blaat = <<<HEREDOC
This is some random data
HEREDOC;

// This Works: (so, is heredoc related)
//$blaat= 'This is some random data';

$blaat2 = bzdecompress(bzcompress($blaat));

include(dirname(__FILE__) . '/../../../tests/quicktester.inc');
quicktester(() ==> $blaat, () ==> $blaat2);
}
