<?hh
// from: https://github.com/facebook/hhvm/issues/3336
function foo( $a, $b, $c = null, $d = null ) :mixed{
  echo "$a\n";
}

// from: https://reviews.facebook.net/D20967#inline-162849
function bar($a, $b = null, $c, $d = null) :mixed{
  echo "$a\n";
}

<<__EntryPoint>>
function main_diagnostic_bug_3336() :mixed{
foo('really', 'works', 'not');
foo('work', 'this', 'works', 'not');

bar('really', 'works', 'not');
bar('work', 'this', 'works', 'not');
}
