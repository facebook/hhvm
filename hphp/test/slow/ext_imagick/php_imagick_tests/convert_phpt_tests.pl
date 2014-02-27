#!/usr/bin/perl

use strict;
use warnings;

my %extension = (
  SKIPIF  => 'skipif',
  FILE    => 'disabled',
  EXPECTF => 'expectf',
);

for my $file (<*.phpt>) {
  my %content = ();
  my $section = 'TEST';
  open FH, '<', $file;
  while (<FH>) {
    if (/^--(TEST|SKIPIF|FILE|EXPECTF)--$/) {
      $section = $1;
    } else {
      $content{$section} ||= [];
      push $content{$section}, $_;
    }
  }
  close FH;

  $file =~ s/\.phpt$//;
  $file =~ s/-/_/g;
  for my $i (keys %extension) {
    next unless exists $content{$i};
    open FH, '>', "$file.php.$extension{$i}";
    print FH join('', @{$content{$i}});
    close FH;
  }
}
