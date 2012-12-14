<?php

/*
 * Generates tables of HhbcExtFuncInfo and HhbcExtClassInfo from
 * extension idl definitions.
 */

global $scriptPath, $outputPath, $extension_src_path;
global $extension_lib_path, $extensions, $lib_dir;
global $extra_idl;
$scriptPath = dirname(__FILE__);
if (getenv("PROJECT_ROOT")) {
  $idl_path = getenv("PROJECT_ROOT") . "/src/idl/";
} else {
  $idl_path = $scriptPath . "../../idl/";
}
$output_file = $argv[1];
$ext_hhvm_path = $argv[2];
$lib_dir = $argv[3]; ## XXX UNUSED
$extension_src_path = $argv[4];
$extension_lib_path = $argv[5];
$extensions = array();
$extra_idl = array();
foreach (array_slice($argv, 7) as $a) {
  if (preg_match('/\.idl.php$/', $a)) {
    $extra_idl[] = $a;
  } else {
    $extensions[] = $a;
  }
}

require_once "gen_lib.php";

function main() {
  global $scriptPath, $output_file, $extension_src_path;
  global $extension_lib_path, $extensions;
  global $idl_path;

  $ext_func_info = array();
  $ext_class_info = array();
  $mangleMap = array();
  parseIDLForFunctions($ext_func_info, $mangleMap, $idl_path);
  parseIDLForMethods($ext_class_info, $mangleMap, $idl_path);
  $sepExtDirs = getSepExtDirs($extension_src_path, $extensions);
  $sepExtHeaders = getSepExtHeaders($extension_src_path, $extensions);
  foreach ($sepExtDirs as $dir) {
    parseIDLForFunctions($ext_func_info, $mangleMap, $dir);
    parseIDLForMethods($ext_class_info, $mangleMap, $dir);
  }

  try {
    $outfile_tempnam = tempnam('/tmp', 'ext_hhvm.cpp.tmp');
    $outfile = fopen($outfile_tempnam, 'w');

    emit_include($outfile, "runtime/ext_hhvm/ext_hhvm.h");
    emit_include($outfile, "runtime/ext/ext.h");
    fwrite($outfile, "#include \"ext_hhvm_infotabs.h\"\n");
    fwrite($outfile,
      "namespace HPHP {\n" .
      "  struct TypedValue;\n" .
      "  namespace VM { struct ActRec; struct Class; }\n" .
      "}\n\n"
    );

    fwrite($outfile, "namespace HPHP {\n\n");

    // First declare all the stubs we need to be able to register.
    foreach ($ext_func_info as $obj) {
      fwrite($outfile, "TypedValue* fg_" . $obj->name .
        "(VM::ActRec *ar);\n");
    }
    foreach ($ext_class_info as $cname => $method_info) {
      fwrite($outfile,
        "VM::Instance* new_" . $cname . "_Instance(" .
        "VM::Class*);\n");
      foreach ($method_info as $obj) {
        fwrite($outfile, "TypedValue* tg_" . getUniqueFuncName($obj) .
          "(VM::ActRec *ar);\n");
      }
    }
    fwrite($outfile, "\n");

    fwrite($outfile, "const long long hhbc_ext_funcs_count = " .
           count($ext_func_info) . ";\n");
    fwrite($outfile, "const HhbcExtFuncInfo hhbc_ext_funcs[] = {\n  ");
    $firstParam = true;
    foreach ($ext_func_info as $obj) {
      if (!$firstParam) {
        fwrite($outfile, ",\n  ");
      }
      $firstParam = false;
      fwrite($outfile, '{ "' . $obj->name . '", fg_' . $obj->name);
      fwrite($outfile, ', (void *)&fh_' . $obj->name . ' }');
    }
    fwrite($outfile, "\n};\n\n");

    foreach ($ext_class_info as $cname => $method_info) {
      fwrite($outfile, "static const long long hhbc_ext_method_count_" .
             $cname . " = " . count($method_info) . ";\n");
      fwrite($outfile, "static const HhbcExtMethodInfo hhbc_ext_methods_" .
             $cname . "[] = {\n  ");
      $firstParam = true;
      foreach ($method_info as $obj) {
        if (!$firstParam) {
          fwrite($outfile, ",\n  ");
        }
        $firstParam = false;
        fwrite($outfile, '{ "' . $obj->name . '", tg_' .
               getUniqueFuncName($obj) . ' }');
      }
      fwrite($outfile, "\n};\n\n");
    }

    fwrite($outfile, "const long long hhbc_ext_class_count = " .
           count($ext_class_info) . ";\n");
    fwrite($outfile, "const HhbcExtClassInfo hhbc_ext_classes[] = {\n  ");
    $firstParam = true;
    foreach ($ext_class_info as $cname => $method_info) {
      if (!$firstParam) {
        fwrite($outfile, ",\n  ");
      }
      $firstParam = false;
      fwrite($outfile, '{ "' . $cname . '", new_' . $cname . '_Instance'.
             ', sizeof(c_' . $cname . ')' .
             ', hhbc_ext_method_count_' . $cname .
             ', hhbc_ext_methods_' . $cname . ' }');
    }
    fwrite($outfile, "\n};\n\n");
    fwrite($outfile, "\n} // !HPHP\n\n");

    fclose($outfile);
    $outfile = null;
    install_file($outfile_tempnam, $output_file);
  } catch (Exception $e) {
    if ($outfile) fclose($outfile);
    if ($outfile_tempnam) `rm -rf $outfile_tempnam`;
  }
}
main();
