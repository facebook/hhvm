<?hh

function generate_next_rel_path() {
	//create the include directory structure
	$pathSep = ":";
	$newIncludePath = "";
	if(substr(PHP_OS, 0, 3) == 'WIN' ) {
	   $pathSep = ";";
	}
	foreach(ZendGoodExtStandardTestsFileFopenIncludePathInc::reldirs() as $newdir) {
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
	foreach(ZendGoodExtStandardTestsFileFopenIncludePathInc::newdirs() as $newdir) {
	   $newIncludePath .= $newdir.$pathSep;
	}
	return ZendGoodExtStandardTestsFileFopenIncludePathInc::dir4().$pathSep . $newIncludePath;
}


function create_include_path() {

	//create the include directory structure
	$pathSep = ":";
	$newIncludePath = "";
	if(substr(PHP_OS, 0, 3) == 'WIN' ) {
	   $pathSep = ";";
	}
	foreach(ZendGoodExtStandardTestsFileFopenIncludePathInc::newdirs() as $newdir) {
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
	foreach(ZendGoodExtStandardTestsFileFopenIncludePathInc::reldirs() as $newdir) {
	   mkdir($newdir);
	   $newIncludePath .= $newdir.$pathSep;
	}
	return $newIncludePath;
}


function teardown_include_path() {

	// remove the directory structure
	foreach(ZendGoodExtStandardTestsFileFopenIncludePathInc::newdirs() as $newdir) {
	   rmdir($newdir);
	}
}

function teardown_relative_path() {

	// remove the directory structure
	foreach(ZendGoodExtStandardTestsFileFopenIncludePathInc::reldirs() as $newdir) {
	   rmdir($newdir);
	}
}

abstract final class ZendGoodExtStandardTestsFileFopenIncludePathInc {
	public static function reldirs() {
		return varray["dir1", "dir2", "dir3"];
	}

	public static function newdirs() {
		return varray[
			self::dir1(),
			self::dir2(),
			self::dir3(),
		];
	}

	public static function dir1() {
		return __SystemLib\hphp_test_tmppath('dir1');
	}

	public static function dir2() {
		return __SystemLib\hphp_test_tmppath('dir2');
	}

	public static function dir3() {
		return __SystemLib\hphp_test_tmppath('dir3');
	}

	//invalid directory
    public static function dir4() {
		return __SystemLib\hphp_test_tmppath('dir4');
	}
}
