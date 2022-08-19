#! /usr/bin/perl
#
# Copyright (c) 2017, Oracle and/or its affiliates. All rights reserved.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License, version 2.0,
# as published by the Free Software Foundation.
#
# This program is also distributed with certain software (including
# but not limited to OpenSSL) that is licensed under separate terms,
# as designated in a particular file or component or in included license
# documentation.  The authors of MySQL hereby grant you an additional
# permission to link the program and your derivative works with the
# separately licensed software that they have included with MySQL.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License, version 2.0, for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA
#

#
# Randomize each .text and .data name (unless it is already randomized),
# so that we get a different order of all the symbols in the final binary
# (assuming -ffunction-sections -fdata-sections and -Wl,--sort-section=name).
# This makes it possible to see if a performance change is due to
# semirandom address positioning effects or a genuine change that will be
# robust to other, unrelated changes.
#
# Usage: randomize-order.pl SEED [OBJFILES...]

use strict;
use warnings;
use Digest::SHA;

my $seed = shift @ARGV;

for my $objfile (@ARGV) {
	my @objcopy_args = ();

	# Read in the list of sections, and find all that start with .text or .data.
	open my $objdumpfh, "-|", "objdump", "-h", $objfile
		or die "objdump: $!";
	while (<$objdumpfh>) {
		chomp;
		if (/^\s*\d+\s*(\.text|\.data)\.(\S+)\s*/) {
			my ($section_type, $section_name) = ($1, $2);

			# Leave already reordered sections alone.
			next if ($section_name =~ /\.__random_order$/);

			my $new_name = Digest::SHA::sha256_hex($seed . $section_name) . ".__random_order";
			push @objcopy_args, "--rename=$section_type.$section_name=$section_type.$new_name";
		}
	}
	close $objdumpfh;

	while (scalar @objcopy_args > 0) {
		# Some targets are too big for one objcopy run in debug mode
		# (the command line gets too long); split into smaller chunks.
		my @args;
		if (scalar @objcopy_args >= 4096) {
			@args = @objcopy_args[0..4095];
			@objcopy_args = @objcopy_args[4096..$#objcopy_args];
		} else {
			@args = @objcopy_args;
			@objcopy_args = ();
		}

		system("objcopy", @args, $objfile, "$objfile.tmp") == 0
			or die "objcopy: $?";
		rename("$objfile.tmp", $objfile)
			or die "rename($objfile): $!";
	}
}
