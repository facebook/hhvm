<?hh
function generate_next_rel_path() {

	//create the include directory structure
	$pathSep = ":";
	$newIncludePath = "";
	if(substr(PHP_OS, 0, 3) == 'WIN' ) {
	   $pathSep = ";";
	}
	foreach(ZendGoodExtZlibTestsGzopenIncludePathInc::reldirs() as $newdir) {
	   $newIncludePath .= $newdir.$pathSep;
	}
	return "dir4".$pathSep . $newIncludePath;
}

function relative_include_path() {

	//create the include directory structure
	$pathSep = ":";
	$newIncludePath = "";
	if(substr(PHP_OS, 0, 3) == 'WIN' ) {
	   $pathSep = ";";
	}
	foreach(ZendGoodExtZlibTestsGzopenIncludePathInc::reldirs() as $newdir) {
	   mkdir($newdir);
	   $newIncludePath .= $newdir.$pathSep;
	}
	return $newIncludePath;
}


function teardown_relative_path() {

	// remove the directory structure
	foreach(ZendGoodExtZlibTestsGzopenIncludePathInc::reldirs() as $newdir) {
	   rmdir($newdir);
	}
}

abstract final class ZendGoodExtZlibTestsGzopenIncludePathInc {
  public static function reldirs() {
		return vec["dir1", "dir2", "dir3"];
	}
}
