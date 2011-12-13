<?php

global $produceHeaderFile;
$produceHeaderFile = false;

global $scriptPath, $outputPath, $extension_src_path;
global $extension_lib_path, $extensions, $lib_dir;
$scriptPath = dirname(__FILE__);
$outputPath = $argv[1];
$lib_dir = $argv[2];
$extension_src_path = $argv[3];
$extension_lib_path = $argv[4];
$extensions = array_slice($argv, 5);

require_once "gen_lib.php";

function phase1() {
  global $scriptPath, $outputPath, $produceHeaderFile, $extension_src_path;
  global $extension_lib_path, $extensions;

  $ext_func_info = array();
  $mangleMap = generateMangleMap(false);
  parseIDLForFunctions($ext_func_info, $mangleMap,
                       $scriptPath . '/../../idl/', 1);
  $sepExtDirs = getSepExtDirs($extension_src_path, $extensions);
  $sepExtHeaders = getSepExtHeaders($extension_src_path, $extensions);
  foreach ($sepExtDirs as $dir) {
    parseIDLForFunctions($ext_func_info, $mangleMap, $dir, 1);
  }

  $ni_cpp_tempnam = null;
  $ni_cpp = null;
  $ni_h_tempnam = null;
  $ni_h = null;

  try {
    $ni_cpp_tempnam = tempnam('/tmp', 'ext_noinline.cpp.tmp');
    $ni_cpp = fopen($ni_cpp_tempnam, 'w');
    if ($produceHeaderFile) {
      $ni_h_tempnam = tempnam('/tmp', 'ext_noinline.h.tmp');
      $ni_h = fopen($ni_h_tempnam, 'w');
    }

    if ($produceHeaderFile) {
      fwrite($ni_h, "#include <runtime/ext/ext.h>\n\n");
      foreach($sepExtHeaders as $header) {
        fwrite($ni_h, "#include \"$header\"\n");
      }
      fwrite($ni_h, "\n");
      fwrite($ni_h, "#ifndef __HHBC_EXT_NOINLINE_H__\n");
      fwrite($ni_h, "#define __HHBC_EXT_NOINLINE_H__\n\n");
      fwrite($ni_h, "namespace HPHP {\n");
      fwrite($ni_cpp, "#include <runtime/ext_hhvm/ext_noinline.h>\n\n");
    } else {
      fwrite($ni_cpp, "#include <runtime/ext/ext.h>\n\n");
      foreach($sepExtHeaders as $header) {
        fwrite($ni_cpp, "#include \"$header\"\n");
      }
      fwrite($ni_cpp, "\n");
      fwrite($ni_cpp, "namespace HPHP {\n");
      foreach ($ext_func_info as $obj) {
        if ($obj->mangledName !== null) {
          continue;
        }
        $nihelper_sig = $obj->getHphpSig(true, true, 'fni_');
        fwrite($ni_cpp, '  ' . preg_replace('/HPHP::/', '', $nihelper_sig) .
               ";\n");
      }
      fwrite($ni_cpp, "}\n\n");
    }

    foreach ($ext_func_info as $obj) {
      if ($obj->mangledName !== null) {
        continue;
      }
      $nihelper_sig = $obj->getHphpSig(true, true, 'fni_');
      if ($produceHeaderFile) {
        fwrite($ni_h, '  ' . preg_replace('/HPHP::/', '', $nihelper_sig) .
               ";\n");
      }
      $ni_func = $nihelper_sig . " {\n  return ";
      if ($obj->returnByRef) {
        $ni_func .= 'strongBind(';
      }
      $ni_func .= 'HPHP::f_' . $obj->name . '(';
      $paramStr = '';
      if ($obj->isVarargs) {
        $paramStr .= '_argc';
      }
      for ($i = 0; $i < count($obj->params); ++$i) {
        if ($paramStr != '') $paramStr .= ', ';
        if ($obj->params[$i]->byRef) {
          $paramStr .= 'ref(' . $obj->params[$i]->name . ')';
        } else {
          $paramStr .= $obj->params[$i]->name;
        }
      }
      if ($obj->isVarargs) {
        $paramStr .= ', _argv';
      }
      $ni_func .= $paramStr;
      $ni_func .= ')';
      if ($obj->returnByRef) {
        $ni_func .= ')';
      }
      $ni_func .= ";\n}\n\n";
      fwrite($ni_cpp, $ni_func);
    }

    if ($produceHeaderFile) {
      fwrite($ni_h, "}\n");
      fwrite($ni_h, "\n#endif // __HHBC_EXT_NOINLINE_H__\n");
    }

    fclose($ni_cpp);
    $ni_cpp = null;
    if ($produceHeaderFile) {
      fclose($ni_h);
      $ni_h = null;
    }

    `mv -f $ni_cpp_tempnam $outputPath/ext_noinline.cpp`;
    if ($produceHeaderFile) {
      `mv -f $ni_h_tempnam $outputPath/ext_noinline.h`;
    }
  } catch (Exception $e) {
    if ($ni_cpp) fclose($ni_cpp);
    if ($ni_cpp_tempnam) `rm -rf $ni_cpp_tempnam`;
    if ($produceHeaderFile) {
      if ($ni_h) fclose($ni_h);
      if ($ni_h_tempnam) `rm -rf $ni_h_tempnam`;
    }
  }
}

phase1();

