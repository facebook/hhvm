#!/usr/bin/perl

# Process the TCSpace instrumentation in an hphp.log to produce
# human-readable summaries. Usage:
#    $ env TRACE=tcspace:1 ./hhvm/hhvm -v Eval.Jit=true \
#           -f my-script.php
#    $ tcspace.pl ./hphp.log

%cnts = {};
%occs = {};

while(<>) {
  if (/^TCSpace/) {
    my ($unused, $category, $num) = split();
    $cnts{$category} += $num;
    $occs{$category} += 1;
  }
}

foreach $value (sort {int($cnts{$b}) <=> int($cnts{$a}) }
                keys %cnts) {
  printf("%24s %6dB / %6d\n", $value, $cnts{$value},
         $occs{$value});
}

