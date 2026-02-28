<?hh

<<__EntryPoint>> function main(): void {
echo "*** Test substituting argument 1 with unknown string values ***\n";



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
  var_dump(get_cfg_var( $var  ) );
}
}
