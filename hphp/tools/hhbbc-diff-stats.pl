#!/usr/bin/perl -w
#
# This is a utility for diffing stats output from hhbbc.
#
use strict;

die "usage: hhbbc-diff-stats.pl stats-file-a stats-file-b" if $#ARGV != 1;

my $a=$ARGV[0];
my $b=$ARGV[1];

my %vals;
my %pcts;

open A, "<$a" or die "$!";
END { close A; };
while (<A>) {
    if (/\s*([a-zA-Z<=_]+)\s*\:\s*(\d+)$/) {
        $vals{$1} = $2;
    }
}
open B, "<$b" or die "$!";
END { close B; }
while (<B>) {
    if (/\s*([a-zA-Z<=_]+)\s*\:\s*(\d+)$/) {
        my $diff = $2 - $vals{$1};
        my $denom = $vals{$1} != 0 ? $vals{$1} : 1;
        my $pct = $diff / $denom;
        $vals{$1} = $diff;
        $pcts{$1} = $pct;
    }
}

foreach my $x (keys %vals) {
    next if $vals{$x} == 0;
    printf "%30s %20d  %7.02f%%\n", $x, $vals{$x}, $pcts{$x} * 100;
}
