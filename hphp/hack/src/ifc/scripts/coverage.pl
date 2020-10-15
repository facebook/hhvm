#!/bin/perl
use strict;

use Cwd 'abs_path';
use File::Basename;
use File::Temp 'tempdir';
use POSIX;

sub Log {
        my ($msg) = @_;
        my $tm = strftime("%Y-%m-%d %H:%M:%S", localtime);
        print("$tm $msg\n");
}

# strips src/ifc/script
my $hackdir = abs_path(dirname(__FILE__) . "/../../..");
# strips hphp/hack
my $fbcode = abs_path($hackdir . "/../..");

# step 0 - parse command line args
my $debug_flag   = 0;
my $nobuild_flag = 0;
my $limit_flag   = -1;
my $out_flag     = "coverage.json";
while (my $arg = shift) {
        if ($arg eq '--debug') {
                $debug_flag = 1;
        }
        elsif ($arg eq '--nobuild') {
                $nobuild_flag = 1;
        }
        elsif ($arg eq '--limit') {
                $limit_flag = shift
                  or die "invalid arg: missing limit number";
                die "invalid arg: --limit expects a numeric argument"
                  if !($limit_flag =~ /^\d+$/);
        }
        elsif ($arg eq '--out') {
                $out_flag = shift
                  or die "invalid arg: missing output file name";
        }
        else {
                print <DATA>;
                exit;
        }
}
$out_flag = abs_path($out_flag);
Log("results will be written to $out_flag");

# step 1 - build ifc analysis
if ($nobuild_flag) {
        Log("skipping build");
} else {
        Log("building ifc analysis...");
        system("buck build //hphp/hack/src:hh_single_type_check") == 0
          or die "build failure";
        Log("done building");
}

# step 2 - find hh_single_type_check
# we look in various 'mode' directories
my ($hstc) = grep {-e} map
  {"$fbcode/buck-out/$_/bin/hphp/hack/src/" .
   "hh_single_type_check/hh_single_type_check"} qw/dev dbgo opt/;
die "could not find hh_single_type_check" if !defined $hstc;
Log("using $hstc");

# step 3 - find all typecheck tests
# skip tests that rely on flags not supported by
# the ifc analysis (banned_flags); also skip tests
# that are expected to trigger parse errors
Log("indexing tests...");
my %banned_flags = map {$_ => 1} qw/
  --like-type-hints
  --like-types-all
  --enable-higher-kinded-types
/;
sub testok {
        # only look at tests that have no typing errors
        my ($test) = @_;
        if ($test =~ /\.php$/ and open my $fh, "<", "$test.exp") {
                return <$fh> eq "No errors\n"
        }
        return 0;
}
sub finddir {
        # recursively scans the directory passed as
        # argument to find tests matching the testok
        # predicate
        my ($dir) = @_;
        my @tests;
        my $flags = "";
        if (open my $fh, "<", "$dir/HH_FLAGS") {
                chomp(my @lines = <$fh>);
                if (grep {$banned_flags{$_}} @lines) {
                        Log("skipping tests in $dir");
                        return ();
                }
                $flags = join(" ", @lines);
        }
        for (glob "$dir/*") {
                push @tests, finddir($_) if -d;
                push @tests, [$flags, $_] if testok($_);
        }
        return @tests;
}
my $testdir = $hackdir . "/test/typecheck";
chdir $testdir or die "could not chdir to $testdir";
# find all tests and sort them so that tests sharing
# the same HH_FLAGS appear consecutively
my @tests = sort {$a->[0] cmp $b->[0]} finddir(".");
$_->[1] =~ s/^..// for (@tests); # strip './' prefix of paths
@tests = splice @tests, 0, $limit_flag if $limit_flag > 0;
my $ntests = @tests;
Log("found $ntests tests");

# step 4 - run the ifc analysis in parallel
# we take care to use the typechecker flags
# in the HH_FLAGS files
my $ifc_args = "--error-format raw --shallow-class-decl --ifc check ''";
my $nproc = 20; # number of parallel workers
# when computing the number of tests per worker we
# round with ceil to avoid leftovers in case $nproc
# does not divide $ntests
my $workshare = POSIX::ceil($ntests/$nproc);
my $resdir = $debug_flag ? "/tmp/cov" : tempdir(CLEANUP => 1);
mkdir $resdir;
Log("worker results will go in $resdir");
my @todo = @tests;
for my $id (1..$nproc) {
        my $pid = fork;
        die "fork failed" if !defined $pid;
        my @work = splice @todo, 0, $workshare;
        next if $pid != 0;
        # we are in one worker process, run
        # the IFC analysis in batches
        my $batchsz = 20;
        my $out = "$resdir/$id.out";
        system "cat /dev/null > $out";
        while (@work) {
                # make sure all the tests in a single batch
                # share the same HH_FLAGS
                my ($flags, @files) = @{shift @work};
                for (2..$batchsz) {
                        last if !@work or $work[0][0] ne $flags;
                        push @files, ${shift @work}[1];
                }
                # use a timeout of 2s per test
                # add 15s of initialization
                my $secs = 15 + 2*@files;
                # unfortunately, in case a test of the batch
                # times out, the rest of the batch is lost
                my $cmd = "{ timeout $secs $hstc $ifc_args $flags " .
                             join(" ", @files) .
                             " || echo Timeout; } >>$out 2>&1";
                print("running ", $cmd, "\n") if ($debug_flag);
                system $cmd;
        }
        exit;
}
# report progress if the output is a terminal
my $pid = -t STDOUT ? fork : undef;
if (defined $pid and $pid == 0) {
        for (;;) {
                my $done = 0+`grep -r '^=== ' $resdir | wc -l`;
                print "\r\e[2Kprogress: $done/$ntests";
                sleep 1;
        }
}
wait for (1..$nproc); # wait for all workers to be done
if (defined $pid) {
        kill 15, $pid; # kill the monitor process
        print "\r\e[2K"; # clear the progress line
}
Log("all workers are done, processing results");

# step 5 - process the results
my %stats = (
        ok      => {count => 0},
        failed  => {count => 0},
        raised  => {count => 0},
        timeout => {count => 0},
        skipped => {count => 0}
);
my %skipped = map {$_->[1] => 1} @tests;
my ($state, $name, $detail, $result);
sub testdone {
        return unless defined $name;
        delete $skipped{$name};
        my $cnts = $stats{$result};
        $cnts->{count}++;
        push @{$cnts->{tests}}, $name;
        push @{$cnts->{detail}{$detail}}, $name
          if defined $detail;
}
for my $id (1..$nproc) {
        open my $fh, "<", "$resdir/$id.out"
          or die "could not open output file for worker $id";
        $name = undef;
        $state = 'INIT';
        while (<$fh>) {
                if ($state eq 'FAIL') {
                        # grab the rest of the failure message
                        # until we hit an empty line
                        chomp;
                        if (/^$/) {
                                $state = 'INIT';
                                next;
                        }
                        s/^ */ /; # strip all indentation
                        $detail .= $_;
                        next;
                }
                if (/^=== IFC analysis results for (.*)$/) {
                        testdone;
                        $state = 'init';
                        $name = $1;
                        $detail = undef;
                        $result = 'ok';
                }
                if (/^Uncaught exception: (.*)$/) {
                        $detail = $1;
                        $result = 'raised';
                }
                if (/^  Failure: (.*)$/) {
                        $state = 'FAIL';
                        $detail = $1;
                        $result = 'failed';
                }
                if (/Typing\[4385\]/) {
                        $detail = 'unhandled syntax (Typing[4385])';
                        $result = 'failed';
                }
                if (/^Timeout$/) {
                        $result = 'timeout';
                }
        }
        testdone;
}
$stats{skipped}{tests} = [keys %skipped];
$stats{skipped}{count} = @{$stats{skipped}{tests}};
Log("parsed all test results");
# sort details by number of affected tests
for (values %stats) {
        next unless exists $_->{detail};
        my %h = %{$_->{detail}};
        $_->{detail} = [
                map {[$_, $h{$_}]}
                sort {$#{$h{$b}} <=> $#{$h{$a}}}
                keys %h
        ];
}

# step 6 - write the results
sub json {
        # quick and dirty json serializer
        # works for the data at hand
        my ($fh, $val, $indent) = (@_, 0);
        if (ref $val eq 'HASH') {
                print $fh ("{\n");
                my $first = 1;
                while (my ($k, $v) = each %$val) {
                        print $fh (",\n") if !$first;
                        $first = 0;
                        print $fh (" " x ($indent + 4), qq/"$k": /);
                        json($fh, $v, $indent + 4);
                }
                print $fh ("\n", " " x $indent, "}");
        }
        elsif (ref $val eq 'ARRAY') {
                print $fh ("[\n");
                my $first = 1;
                for my $v (@$val) {
                        print $fh (",\n") if !$first;
                        $first = 0;
                        print $fh (" " x ($indent + 4));
                        json($fh, $v, $indent + 4);
                }
                print $fh ("\n", " " x $indent, "]");
        }
        elsif ($val =~ /^\d+$/) {
                print $fh ($val);
        }
        else {
                $val =~ s/\\/\\\\/g; # escape \
                $val =~ s/"/\\"/g; # then "
                print $fh (qq/"$val"/);
        }
}
{
        my $fh;
        if (!open $fh, ">", $out_flag) {
                Log("could not open results file, using stdout");
                # better than failing now
                $fh = \*STDOUT;
        }
        json($fh, \%stats);
        print $fh ("\n");
}

Log("done");

# CLI documentation
__DATA__
usage: perl coverage.pl [--debug] [--nobuild] [--limit N]
                        [--out PATH] [--help]

arguments:
  --debug     log more information and keep intermediate
              results in /tmp/cov
  --nobuild   skip building hh_single_type_check and use
              one in buck-out
  --limit N   limit the number of tests to process
              (useful when debugging)
  --out PATH  output the JSON results in the file PATH
              (defaults to coverage.json)
  --help      display this message
