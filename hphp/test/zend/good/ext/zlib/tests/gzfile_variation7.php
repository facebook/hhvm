<?php


$use_include_path = 0;


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
  var_dump(gzfile( $var ,  $use_include_path ) );
}
echo "===DONE===\n";
