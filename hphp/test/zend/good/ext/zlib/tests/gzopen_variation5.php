<?hh

function runtest() {
	$tmpfile = 'gzopen_variation5.tmp';
	$h = gzopen($tmpfile, "w", 1);
	fwrite($h, "This is the test file");
	fclose($h);


	$h = @gzopen($tmpfile, "r");
	if ($h === false) {
	   echo "Not created in working dir\n";
	}
	else {
	   echo "created in working dir\n";
	   gzclose($h);
	   unlink($tmpfile);
	}

	$h = @gzopen('dir1/'.$tmpfile, "r");
	if ($h === false) {
	   echo "Not created in dir1\n";
	}
	else {
	   echo "created in dir1\n";
	   gzclose($h);
	   unlink('dir1/'.$tmpfile);
	}
}
<<__EntryPoint>>
function main_entry(): void {
  /* Prototype  : resource gzopen(string filename, string mode [, int use_include_path])
   * Description: Open a .gz-file and return a .gz-file pointer
   * Source code: ext/zlib/zlib.c
   * Alias to functions:
   */

  require_once('gzopen_include_path.inc');

  echo "*** Testing gzopen() : variation ***\n";
  $thisTestDir = "gzopenVariation5.dir";
  mkdir($thisTestDir);
  chdir($thisTestDir);

  $newpath = relative_include_path();
  set_include_path($newpath);
  runtest();
  $newpath = generate_next_rel_path();
  set_include_path($newpath);
  runtest();

  teardown_relative_path();
  restore_include_path();
  chdir("..");
  rmdir($thisTestDir);
  echo "===DONE===\n";
}
