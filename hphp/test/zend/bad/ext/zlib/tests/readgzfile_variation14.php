<?php


$filename = $filename = dirname(__FILE__)."/004.txt.gz";


$heredoc = <<<EOT
hello world
EOT;

$variation_array = array(
  'string DQ' => "string",
  'string SQ' => 'string',
  'mixed case string' => "sTrInG",
  'heredoc' => $heredoc
  );


foreach ( $variation_array as $var ) {
  var_dump(readgzfile( $filename, $var  ) );
}
?>
===DONE===