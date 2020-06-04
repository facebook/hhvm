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

function generate_next_path() {

	//create the include directory structure
	$pathSep = ":";
	$newIncludePath = "";
	if(substr(PHP_OS, 0, 3) == 'WIN' ) {
	   $pathSep = ";";
	}
	foreach(ZendGoodExtZlibTestsGzopenIncludePathInc::newdirs() as $newdir) {
	   $newIncludePath .= $newdir.$pathSep;
	}
	return ZendGoodExtZlibTestsGzopenIncludePathInc::dir4().$pathSep . $newIncludePath;
}


function create_include_path() {

	//create the include directory structure
	$pathSep = ":";
	$newIncludePath = "";
	if(substr(PHP_OS, 0, 3) == 'WIN' ) {
	   $pathSep = ";";
	}
	foreach(ZendGoodExtZlibTestsGzopenIncludePathInc::newdirs() as $newdir) {
	   mkdir($newdir);
	   $newIncludePath .= $newdir.$pathSep;
	}
	return $newIncludePath;
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


function teardown_include_path() {

	// remove the directory structure
	foreach(ZendGoodExtZlibTestsGzopenIncludePathInc::newdirs() as $newdir) {
	   rmdir($newdir);
	}
}

function teardown_relative_path() {

	// remove the directory structure
	foreach(ZendGoodExtZlibTestsGzopenIncludePathInc::reldirs() as $newdir) {
	   rmdir($newdir);
	}
}

abstract final class ZendGoodExtZlibTestsGzopenIncludePathInc {
  public static function reldirs() {
		return varray["dir1", "dir2", "dir3"];
	}

  public static function newdirs() {
		$base = self::base();
		return varray[
			$base.".dir1",$base.".dir2",$base.".dir3",
		];
	}

	//invalid directory
  public static function dir4() {
		return self::base().".dir4";
	}

	private static function base(): string {
		$pwd = getcwd();
		$f = basename(__FILE__);
		return $pwd."/".$f;
	}
}
