<?hh // $Id$
<<__EntryPoint>> function main(): void {
$x = 2;
$x **= 3;

$tests = <<<TESTS
 -3 ** 2 === -9
 (-3) **2 === 9
 2 ** 3 ** 2 === 512
 (2 ** 3) ** 2 === 64
 $x === 8
TESTS;

 echo "On failure, please mail result to php-dev@lists.php.net\n";
 include(dirname(__FILE__) . '/../../../../tests/quicktester.inc');
 quicktester(() ==> -3 ** 2, () ==> -9);
 quicktester(() ==> (-3) **2, () ==> 9);
 quicktester(() ==> 2 ** 3 ** 2, () ==> 512);
 quicktester(() ==> (2 ** 3) ** 2, () ==> 64);
 quicktester(() ==> $x, () ==> 8);
}
