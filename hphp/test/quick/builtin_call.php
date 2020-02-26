<?hh
function foo($a) {
  $sc = new SoapClient(null,
                       darray['location' =>
                             "http://fizzle/does-not-exist.php",
                             'uri' => "http://does-not-exit-uri/"]);
  try {
    var_dump($sc->NotASoapFunction());
  } catch (Exception $e) {
    echo "Caught exception\n";
  }
  var_dump((string)strtoupper($a));
}
<<__EntryPoint>> function main(): void {
foo("bar");
}
