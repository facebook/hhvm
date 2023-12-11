<?hh

<<__EntryPoint>> function main(): void {
$filename = $filename = dirname(__FILE__)."/004.txt.gz";


$heredoc = <<<EOT
hello world
EOT;

$variation_array = dict[
  'string DQ' => "string",
  'string SQ' => 'string',
  'mixed case string' => "sTrInG",
  'heredoc' => $heredoc
  ];


foreach ( $variation_array as $var ) {
  try { var_dump(gzfile( $filename, $var  ) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}
echo "===DONE===\n";
}
