<?hh
$pwd = getcwd();
$included_files = get_included_files(); $f = basename(current($included_files), ".php");
ZendGoodExtStandardTestsFileFopenIncludePathInc::$dir1 = $pwd."/".$f.".dir1";
ZendGoodExtStandardTestsFileFopenIncludePathInc::$dir2 = $pwd."/".$f.".dir2";
ZendGoodExtStandardTestsFileFopenIncludePathInc::$dir3 = $pwd."/".$f.".dir3";
//invalid directory
ZendGoodExtStandardTestsFileFopenIncludePathInc::$dir4 = $pwd."/".$f.".dir4";
ZendGoodExtStandardTestsFileFopenIncludePathInc::$newdirs = varray[
  ZendGoodExtStandardTestsFileFopenIncludePathInc::$dir1,
  ZendGoodExtStandardTestsFileFopenIncludePathInc::$dir2,
  ZendGoodExtStandardTestsFileFopenIncludePathInc::$dir3,
];



function generate_next_rel_path() {

	//create the include directory structure
	$pathSep = ":";
	$newIncludePath = "";
	if(substr(PHP_OS, 0, 3) == 'WIN' ) {
	   $pathSep = ";";
	}
	foreach(ZendGoodExtStandardTestsFileFopenIncludePathInc::$reldirs as $newdir) {
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
	foreach(ZendGoodExtStandardTestsFileFopenIncludePathInc::$newdirs as $newdir) {
	   $newIncludePath .= $newdir.$pathSep;
	}
	return ZendGoodExtStandardTestsFileFopenIncludePathInc::$dir4.$pathSep . $newIncludePath;
}


function create_include_path() {

	//create the include directory structure
	$pathSep = ":";
	$newIncludePath = "";
	if(substr(PHP_OS, 0, 3) == 'WIN' ) {
	   $pathSep = ";";
	}
	foreach(ZendGoodExtStandardTestsFileFopenIncludePathInc::$newdirs as $newdir) {
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
	foreach(ZendGoodExtStandardTestsFileFopenIncludePathInc::$reldirs as $newdir) {
	   mkdir($newdir);
	   $newIncludePath .= $newdir.$pathSep;
	}
	return $newIncludePath;
}


function teardown_include_path() {

	// remove the directory structure
	foreach(ZendGoodExtStandardTestsFileFopenIncludePathInc::$newdirs as $newdir) {
	   rmdir($newdir);
	}
}

function teardown_relative_path() {

	// remove the directory structure
	foreach(ZendGoodExtStandardTestsFileFopenIncludePathInc::$reldirs as $newdir) {
	   rmdir($newdir);
	}
}

abstract final class ZendGoodExtStandardTestsFileFopenIncludePathInc {
  public static $reldirs = varray["dir1", "dir2", "dir3"];
  public static $newdirs;
  public static $dir1;
  public static $dir2;
  public static $dir3;
  public static $dir4;
}
