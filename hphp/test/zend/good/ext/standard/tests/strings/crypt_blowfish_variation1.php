<?hh
<<__EntryPoint>> function main(): void {
$salts = dict['32' => '$2a$32$CCCCCCCCCCCCCCCCCCCCCC$',
               '33' => '$2a$33$CCCCCCCCCCCCCCCCCCCCCC$',
               '34' => '$2a$34$CCCCCCCCCCCCCCCCCCCCCC$',
               '35' => '$2a$35$CCCCCCCCCCCCCCCCCCCCCC$',
               '36' => '$2a$36$CCCCCCCCCCCCCCCCCCCCCC$',
               '37' => '$2a$37$CCCCCCCCCCCCCCCCCCCCCC$',
               '38' => '$2a$38$CCCCCCCCCCCCCCCCCCCCCC$',];

foreach($salts as $i=>$salt) {
  $crypt = crypt('U*U', $salt);
  if ($crypt === '*0' || $crypt === '*1') {
    echo "$i. OK\n";
  } else {
    echo "$i. Not OK\n";
  }
}
}
