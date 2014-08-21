<?php
// from: https://github.com/facebook/hhvm/issues/3336
function foo( $a, $b, $c = null, $d = null ) {
  echo "$a\n";
}

// from: https://reviews.facebook.net/D20967#inline-162849
function bar($a, $b = null, $c, $d = null) {
  echo "$a\n";
}
foo();
foo('does');
foo('this', 'not');
foo('really', 'works', 'not');
foo('work', 'this', 'works', 'not');

bar();
bar('does');
bar('this', 'not');
bar('really', 'works', 'not');
bar('work', 'this', 'works', 'not');
