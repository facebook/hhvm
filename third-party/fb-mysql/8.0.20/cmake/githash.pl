#!/usr/bin/perl -w

#
# Copyright (c) 2016 Facebook, Inc.
# All rights reserved
#

#
# Git hash/date header file generator
#

use strict;

use Cwd;
use File::Copy qw(copy);
use Getopt::Long qw(GetOptions);
use Pod::Usage qw(pod2usage);

sub check_all_or_none {
  my $bitmask_a = 0;
  $bitmask_a = $bitmask_a | 1 if shift ne "";
  $bitmask_a = $bitmask_a | 2 if shift ne "";

  my $bitmask_b = 0;
  $bitmask_b = $bitmask_b | 1 if shift ne "";
  $bitmask_b = $bitmask_b | 2 if shift ne "";

  pod2usage("You must specify both or none of the mysql git hash and date")
      unless ($bitmask_a == 0 || $bitmask_a == 3);

  pod2usage("You must specify both or none of the rocksdb git hash and date")
      unless ($bitmask_b == 0 || $bitmask_b == 3);
}

# Function to retrieve the git hash and date from a repository
sub git_hash_and_date {
  my $subdir = shift;
  my $orgdir = Cwd::getcwd;

  # Switch directories to the specified one
  chdir($subdir) or die "Can't change directory to '$subdir'";

  # Get the hash and date from the most recent revision in the repository
  my $git_cmd = "git log -1 --format=\"%H;%cI\"";
  open (my $log, "$git_cmd |") or die "Can't run $git_cmd";

  my $githash = "";
  my $gitdate = "";

  # Loop through all the lines - we should only have one line
  while (<$log>) {
    # Search for a line that has a hash, a semicolon and the date
    if (/^([0-9a-f]{7,40});(.*)\n/) {
      die "Unexpected multiple matching log lines" unless !length($githash);
      $githash = $1;
      $gitdate = $2;
    }
  }

  # Make sure we got something
  die "No matching log lines" unless length($githash);

  # Close the input and switch back to the original subdirectory
  close($log);
  chdir($orgdir);

  # Return the found hash and date
  return ($githash, $gitdate);
}

# main function
sub main {
  my $root = "";
  my $infile = "";
  my $mysql_git_hash = "";
  my $mysql_git_date = "";
  my $rocksdb_git_hash = "";
  my $rocksdb_git_date = "";
  my $help = 0;

  # Get the parameters
  GetOptions(
    'help|?' => \$help,
    'git_root=s' => \$root,
    'file=s' => \$infile,
    'mysql_githash=s' => \$mysql_git_hash,
    'mysql_gitdate=s' => \$mysql_git_date,
    'rocksdb_githash=s' => \$rocksdb_git_hash,
    'rocksdb_gitdate=s' => \$rocksdb_git_date
  ) or pod2usage();

  # Validate the parameters
  pod2usage(-verbose => 1) if $help;
  pod2usage("missing required parameter --git_root") if $root eq "";
  pos2usage("missing required parameter --file") if $infile eq "";
  check_all_or_none($mysql_git_hash, $mysql_git_date,
                    $rocksdb_git_hash, $rocksdb_git_date);

  my $rocksdb = "$root/rocksdb";
  my $outfile = "$infile.tmp";

  if ($mysql_git_hash eq "") {
    # retrieve the git hash and date for the main repository
    ($mysql_git_hash, $mysql_git_date) = git_hash_and_date $root;
  }
  if ($rocksdb_git_hash eq "") {
    if (-d $rocksdb) {
      # retrieve the git hash and date for the rocksdb submodule
      ($rocksdb_git_hash, $rocksdb_git_date) = git_hash_and_date $rocksdb;
    } else {
      $rocksdb_git_hash="none";
      $rocksdb_git_date="none";
    }
  }

  # Open the user's file for reading and a temporary file for writing
  open(my $in, "<", $infile) or die "Could not open '$infile'";
  open(my $out, ">", $outfile) or die "Could not create '$outfile'";

  # For each line, see if we can replace anything
  while (<$in>) {
    s/\@MYSQL_GIT_HASH\@/$mysql_git_hash/g;
    s/\@MYSQL_GIT_DATE\@/$mysql_git_date/g;
    s/\@ROCKSDB_GIT_HASH\@/$rocksdb_git_hash/g;
    s/\@ROCKSDB_GIT_DATE\@/$rocksdb_git_date/g;
    print $out $_;
  }

  # Close both files
  close $in;
  close $out;

  # Copy the temporary file to the original and then delete it
  copy $outfile, $infile or
      die "Unable to copy temp file ($outfile) on top of original ($infile)";
  unlink $outfile;
}

main(@ARGV);

=head1 NAME

githash.pl - Generate the mysql_githash.h file for MySQL

=head1 SYNOPSIS

githash.pl [options]

  Options:
    --help                        Brief help message
    --git_root=<path>             Path to the root of the MySQL git repository
    --file=<file>                 File to update with the git hashes and dates
    --mysql_githash=<hash_str>    Optional git hash to use for MySQL
    --mysql_gitdate=<date_str>    Optional git date to use for MySQL
    --rocksdb_githash=<hash_str>  Optional git hash to use for RocksDB
    --rocksdb_gitdate=<date_str>  Optional git date to use for RocksDB

=head1 OPTIONS

=over 4

=item B<--help>

Print a brief help message list all the options and exits

=item B<--git_root>

Supplies the path to the root git repository.  If the optional hashes and
dates are not supplied this script will call 'git log' in this location to get
the hashes and dates.

=item B<--file>

Specifies the file to update.  This file will be scanned for the following
markers, which will be replaced by the corresponding hash or date:

  @MYSQL_GIT_HASH@
  @MYSQL_GIT_DATE@
  @ROCKSDB_GIT_HASH@
  @ROCKSDB_GIT_DATE@

=item B<--mysql_githash>

Optional git hash for the MySQL repository.  If this is not specified the
script will use "git log" to query the repository.  If one of the hashes or
dates is supplied, all of them must be supplied.

=item B<--mysql_gitdate>

Optional git date for the MySQL repository.  If this is not specified the
script will use "git log" to query the repository.  If one of the hashes or
dates is supplied, all of them must be supplied.

=item B<--rocksdb_githash>

Optional git hash for the RocksDB repository.  If this is not specified the
script will use "git log" to query the repository.  If one of the hashes or
dates is supplied, all of them must be supplied.

=item B<--rocksdb_gitdate>

Optional git date for the RocksDB repository.  If this is not specified the
script will use "git log" to query the repository.  If one of the hashes or
dates is supplied, all of them must be supplied.

=back

=head1 DESCRIPTION

B<This program> will take the git hashes and dates (either supplied via
parameters or retrieved from 'git log') and update the specified file with the
values.

=cut
