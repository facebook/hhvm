#!/usr/bin/perl -w
use strict;

my $buffer = '';
my $output_buffer = '';

sub process_buffer {
    $buffer =~ s/^\s*//g;
    $buffer =~ s/\s+/ /g;
    $buffer =~ s/^\s*([a-zA-Z0-9]+)<[^>]+>/$1/g;
    $output_buffer .= 'O(';
    $output_buffer .= $buffer.") \\\n";
    $buffer = '';
}

while (<>) {
    if ($buffer && !/^\|(.*)$/) {
        process_buffer;
        next;
    }
    if (/^\|(.*)/) {
        $buffer .= $1;
    }
}

######################################################################

# Print but insert some space so opcode names show up in a clear
# column.
sub print_pretty {
    my @lines = split /\n/, $output_buffer;
    my $max = 0;
    foreach (@lines) {
        if (/^(O[^,]*)/) {
            $max = length $1 if length $1 > $max;
        }
    }
    foreach (sort @lines) {
        if (/^(O[^,]*)/) {
            my $op = $1;
            print $op . ", ";
            s/^(O[^,]*),//;
            print ' ' x ($max - length $op);
        }
        print;
        print "\n";
    }
}

print "// \@".
    "generated\n";
print "#define IR_OPCODES \\\n";
print "\\\n";
print_pretty;
print "/**/\n";
