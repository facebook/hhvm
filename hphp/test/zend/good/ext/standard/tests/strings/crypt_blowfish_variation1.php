<?hh
<<__EntryPoint>> function main(): void {
$salts = darray[b'32' => b'$2a$32$CCCCCCCCCCCCCCCCCCCCCC$',
               b'33' => b'$2a$33$CCCCCCCCCCCCCCCCCCCCCC$',
               b'34' => b'$2a$34$CCCCCCCCCCCCCCCCCCCCCC$',
               b'35' => b'$2a$35$CCCCCCCCCCCCCCCCCCCCCC$',
               b'36' => b'$2a$36$CCCCCCCCCCCCCCCCCCCCCC$',
               b'37' => b'$2a$37$CCCCCCCCCCCCCCCCCCCCCC$',
               b'38' => b'$2a$38$CCCCCCCCCCCCCCCCCCCCCC$',];

foreach($salts as $i=>$salt) {
  $crypt = crypt(b'U*U', $salt);
  if ($crypt === b'*0' || $crypt === b'*1') {
    echo "$i. OK\n";
  } else {
    echo "$i. Not OK\n";
  }
}
}
