<?hh

<<__EntryPoint>> function main(): void {
echo "*** Test substituting argument 1 with string values ***\n";



$heredoc = <<<EOT
hello world
EOT;

$variation_array = darray[
  'string DQ' => "string",
  'string SQ' => 'string',
  'mixed case string' => "sTrInG",
  'heredoc' => $heredoc,
  ];


foreach ( $variation_array as $var ) {
  try { var_dump(posix_seteuid( $var  ) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}
}
