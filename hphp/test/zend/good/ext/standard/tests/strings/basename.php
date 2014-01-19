<?php
	// simple checks
	var_dump(basename("bar"));
	var_dump(basename("/foo/bar"));
	var_dump(basename("/bar"));

	// simple checks with trailing slashes
	var_dump(basename("bar/"));
	var_dump(basename("/foo/bar/"));
	var_dump(basename("/bar/"));

	// suffix removal checks
	var_dump(basename("bar.gz", ".gz"));
	var_dump(basename("/foo/bar.gz", ".gz"));
	var_dump(basename("/bar.gz", ".gz"));

	// suffix removal checks with trailing slashes
	var_dump(basename("bar.gz/", ".gz"));
	var_dump(basename("/foo/bar.gz/", ".gz"));
	var_dump(basename("/bar.gz/", ".gz"));

	// suffix removal checks
	var_dump(basename("/.gz", ".gz"));
	var_dump(basename("/foo/.gz", ".gz"));
	var_dump(basename("/.gz", ".gz"));

	// binary safe?
	var_dump(basename("foo".chr(0)."bar"));
	var_dump(basename("foo".chr(0)."bar.gz", ".gz"));
?>