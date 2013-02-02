<?php

// $scriptPath should be initialized by the script that includes this file
global $scriptPath, $ext_hhvm_path;
global $idl_path;

require_once $idl_path . 'base.php';

function generateMangleMap() {
  global $scriptPath, $extension_lib_path, $extensions;

  $mangleMap = array();

  // There are a few C++ functions that begin with "f_" that are present
  // in the object files but do not have a corresponding entry in the IDL
  // files. We ignore these.
  $ignoreList = array(
    'HPHP::f_readlink_internal(HPHP::String const&, bool)' => true,
    'HPHP::f_hphp_splfileobject_haschildren(HPHP::Object const&)' => true,
    'HPHP::f_var_dump(HPHP::Variant const&)' => true
  );

  // Build the mangle map, using c++filt to demangle the mangled names
  $mlist = '';
  global $current_object_file;
  if (!file_exists($current_object_file)) {
    echo "Object file doesn't exist! ($current_object_file)\n";
    exit(1);
  }
  $raw_read_elf = shell_exec(
    "readelf -s -W $current_object_file | grep FUNC.*GLOBAL");
  $mangled = explode("\n", $raw_read_elf);
  for ($i = 0; $i < count($mangled); $i++) {
    $m = trim($mangled[$i]);
    if ($m == '') continue;
    $m = trim(preg_replace('/^.*DEFAULT[0-9 ]*/', '', $m));
    $mlist .= $m . "\n";
  }

  $tmpFilepath = tempnam('/tmp', 'ext_hhvm_tmp');
  $tmp_file = fopen($tmpFilepath, 'w');
  fwrite($tmp_file, $mlist);
  fclose($tmp_file);
  $dlist = `cat $tmpFilepath | c++filt`;
  `rm -rf $tmpFilepath`;

  $marr = explode("\n", $mlist);
  $darr = explode("\n", $dlist);
  for ($i = 0; $i < count($marr) && $i < count($darr); ++$i) {
    $m = $marr[$i];
    $d = $darr[$i];
    if ($m == '' || $d == '') continue;
    if (isset($ignoreList[$d])) continue;
    /*
     * We only care about functions that start with "f_" or "fni_"
     * and methods that start with "t_" ot "ti_"
     *
     * fni_ are our stubs in noinline.cpp for extensions that are
     * normally defined inline only.
     */
    if (preg_match('/^HPHP::f(ni)?_/', $d) ||
        preg_match('/^HPHP::c_[A-Za-z0-9_]*::t(i)?_/', $d)) {
      $mangleMap[$d] = $m;
    }
  }

  return $mangleMap;
}

function parseIDLParams($idlFunc, $isMagicMethod = false) {
  $params = array();
  foreach ($idlFunc['args'] as $arg) {
    $paramname = $arg['name'];
    $parambyRef = array_key_exists('ref', $arg) && $arg['ref'];
    $paramtype = mapType(typeidlname($arg['type'], 'void'), $parambyRef);
    if ($isMagicMethod) {
      $paramtype = 'HPHP::Variant';
    }
    $paramdefValue = array_key_exists('default', $arg)
                     ? $arg['default']
                     : 'NULL';
    if ($paramdefValue == 'NULL') {
      $paramdefValue = '';
    } else if ($paramdefValue == '""') {
      $paramdefValue = 'empty_string';
    }
    $params[] = new PhpParam($paramname, $paramtype, $paramdefValue,
                             $parambyRef);
  }
  return $params;
}

function parseIDLFunc($idlFunc, $mangleMap) {
  $name = $idlFunc['name'];
  $returnByRef = array_key_exists('ref', $idlFunc) && $idlFunc['ref'];
  $returnType = mapReturnType(typeidlname($idlFunc['return'], 'void'),
                              $returnByRef);
  $isVarargs = $idlFunc['flags'] & (VariableArguments | RefVariableArguments |
                                    MixedVariableArguments);
  $params = parseIDLParams($idlFunc);
  $obj = new PhpExtFunc();
  $obj->initFunc($name, $returnType, $returnByRef, $params, $isVarargs);
  $sig = $obj->getHphpSig();

  // The implementation of some extension functions are in the .h file,
  // so we don't have a mangled names for them.
  if (!isset($mangleMap[$sig])) {
    $obj->prefix = 'fni_';
    $sig = $obj->getHphpSig();
  }

  if (isset($mangleMap[$sig])) {
    $obj->mangledName = $mangleMap[$sig];
  } else {
    $obj->mangledName = null;
  }

  return $obj;
}

function parseIDLMethod($idlFunc, $mangleMap, $className) {
  $name = $idlFunc['name'];
  $isMagicMethod = false;
  if ($name === '__get' || $name === '__set' || $name === '__isset' ||
      $name === '__unset' || $name === '__call') {
    $isMagicMethod = true;
  }
  $returnByRef = array_key_exists('ref', $idlFunc) && $idlFunc['ref'];
  $returnType = mapReturnType(typeidlname($idlFunc['return'], 'void'),
                              $returnByRef);
  $isVarargs = $idlFunc['flags'] & (VariableArguments | RefVariableArguments |
                                    MixedVariableArguments);
  $isStatic = $idlFunc['flags'] & IsStatic;
  $params = parseIDLParams($idlFunc, $isMagicMethod);
  $obj = new PhpExtFunc();
  $obj->initMethod($className, $name, $returnType, $returnByRef, $params,
                   $isVarargs, $isStatic);
  $sig = $obj->getHphpSig();
  if (isset($mangleMap[$sig])) {
    $obj->mangledName = $mangleMap[$sig];
  } else {
    $obj->mangledName = null;
  }
  return $obj;
}

function findIDLFiles($idlDir) {
  global $extra_idl;
  $files = array_filter(
    scandir($idlDir),
    function ($f) {
      return preg_match('/\.idl\.php$/', $f);
    });

  $files = array_map(function($e) use($idlDir) {
      return $idlDir.'/'.$e;
    }, $files);

  $files = array_merge($files, $extra_idl);
  return $files;
}

// Parse the IDL files for info about extension functions
function parseIDLForFunctions(&$ext_func_info, $mangleMap, $idlDir) {
  global $scriptPath;
  $files = findIDLFiles($idlDir);
  foreach ($files as $file) {
    require($file);
    global $funcs;
    foreach ($funcs as $func) {
      $ext_func_info[] = parseIDLFunc($func, $mangleMap);
    }
    ResetSchema();
  }
}

// Parse the IDL files for info about extension methods
function parseIDLForMethods(&$ext_class_info, $mangleMap, $idlDir) {
  global $scriptPath;
  $files = findIDLFiles($idlDir);
  foreach ($files as $file) {
    require($file);
    global $classes;
    foreach ($classes as $cls) {
      $cname = $cls['name'];
      if (strtolower($cname) === 'closure') {
        // Ignore the C++ version of Closure; the VM uses the
        // pure-PHP Closure class declared in systemlib.php
        continue;
      }
      $method_info = array();
      foreach ($cls['methods'] as $method) {
        $method_info[] = parseIDLMethod($method, $mangleMap, $cname);
      }
      $ext_class_info[$cname] = $method_info;
    }
    ResetSchema();
  }
}

function getSepExtDirs($extension_src_path, $extensions) {
  $dirs = array();
  foreach($extensions as $ext) {
    $dirs[] = $extension_src_path . '/' . $ext;
  }
  return $dirs;
}

function getSepExtHeaders($extension_src_path, $extensions) {
  $headers = array();
  foreach($extensions as $ext) {
    $headers[] = $extension_src_path . '/' . $ext . '/ext_' . $ext . '.h';
  }
  return $headers;
}

class PhpParam {
  public $name;
  public $type;
  public $defVal;
  public $byRef;
  public function __construct($n, $t, $d, $ref) {
    $this->name = $n;
    $this->type = $t;
    $this->defVal = $d;
    $this->byRef = $ref;
  }
}

class PhpExtFunc {
  public $name;
  public $className;
  public $returnType;
  public $returnByRef;
  public $params;
  public $isVarargs;
  public $isStatic;
  public $mangledName;
  public $prefix;
  public $minNumParams;
  public $maxNumParams;
  public $numTypeChecks;

  public function initFunc($name, $returnType, $returnByRef, $params,
                           $isVarargs) {
    $this->className = null;
    $this->name = (string)$name;
    $this->returnType = $returnType;
    $this->returnByRef = (bool)$returnByRef;
    $this->params = $params;
    $this->isVarargs = (bool)$isVarargs;
    $this->isStatic = null;
    $this->mangledName = null;
    $this->initComputedProps();

    if ($this->name != strtolower($this->name)) {
      echo "Extension function name must be all " .
           "lowercase: " . $this->name . "\n";
      echo "ext_hhvm generation failed\n";
      exit(1);
    }
  }

  public function initMethod($className, $name, $returnType, $returnByRef,
                             $params, $isVarargs, $isStatic) {
    $this->className = (string)$className;
    $this->name = (string)$name;
    $this->returnType = $returnType;
    $this->returnByRef = (bool)$returnByRef;
    $this->params = $params;
    $this->isVarargs = (bool)$isVarargs;
    $this->isStatic = (bool)$isStatic;
    $this->mangledName = null;
    $this->initComputedProps();
  }

  private function initComputedProps() {
    if ($this->className === null) {
      $this->prefix = 'f_';
    } else {
      $this->prefix = $this->isStatic ? 'ti_' : 't_';
    }
    $this->maxNumParams = count($this->params);
    $this->minNumParams = 0;
    foreach ($this->params as $p) {
      if ($p->defVal == '') {
        $this->minNumParams = $this->minNumParams + 1;
      }
    }
    $this->numTypeChecks = 0;
    foreach ($this->params as $p) {
      if ($p->type != 'HPHP::Variant' &&
          $p->type != 'HPHP::VRefParamValue const&' &&
          $p->type != 'HPHP::Variant const&') {
        $this->numTypeChecks = $this->numTypeChecks + 1;
      }
    }
  }

  public function getHphpSig($withReturnType = false,
                             $withParamNames = false,
                             $prefix = null) {
    $sig = '';
    if ($withReturnType) $sig .= $this->returnType . ' ';
    if ($prefix === null) $prefix = $this->prefix;
    if ($this->className === null) {
      $sig .= 'HPHP::' . $prefix . strtolower($this->name) . '(';
    } else {
      $sig .= 'HPHP::c_' . $this->className . '::' . $prefix .
        strtolower($this->name) . '(';
    }
    $paramsig = '';
    if ($this->isStatic) {
      $paramsig = 'char const*';
      if ($withParamNames) {
        $paramsig .= ' cls';
      }
    }
    if ($this->isVarargs) {
      if ($paramsig != '') $paramsig .= ', ';
      $paramsig = 'int';
      if ($withParamNames) {
        $paramsig .= ' _argc';
      }
    }
    foreach ($this->params as $p) {
      if ($paramsig != '') $paramsig .= ', ';
      $paramsig .= $p->type;
      if ($withParamNames) {
        $paramsig .= ' ' . $p->name;
      }
    }
    if ($this->isVarargs) {
      $paramsig .= ', HPHP::Array const&';
      if ($withParamNames) {
        $paramsig .= ' _argv';
      }
    }
    $sig .= $paramsig . ')';
    return $sig;
  }
}

$typeMap = array('Boolean' => 'bool',
                 'Int32' => 'int',
                 'Int64' => 'long long',
                 'Double' => 'double',
                 'String' => 'HPHP::String const&',
                 'Int64Vec' => 'HPHP::Array const&',
                 'StringVec' => 'HPHP::Array const&',
                 'VariantVec' => 'HPHP::Array const&',
                 'Int64Map' => 'HPHP::Array const&',
                 'StringMap' => 'HPHP::Array const&',
                 'VariantMap' => 'HPHP::Array const&',
                 'Object' => 'HPHP::Object const&',
                 'Resource' => 'HPHP::Object const&',
                 'Variant' => 'HPHP::Variant const&',
                 'Numeric' => 'HPHP::Variant const&',
                 'Primitive' => 'HPHP::Variant const&',
                 'PlusOperand' => 'HPHP::Variant const&',
                 'Sequence' => 'HPHP::Variant const&',
                 'Any' => 'HPHP::Variant const&');

$returnTypeMap = array('Boolean' => 'bool',
                       'Int32' => 'int',
                       'Int64' => 'long long',
                       'Double' => 'double',
                       'String' => 'HPHP::String',
                       'Int64Vec' => 'HPHP::Array',
                       'StringVec' => 'HPHP::Array',
                       'VariantVec' => 'HPHP::Array',
                       'Int64Map' => 'HPHP::Array',
                       'StringMap' => 'HPHP::Array',
                       'VariantMap' => 'HPHP::Array',
                       'Object' => 'HPHP::Object',
                       'Resource' => 'HPHP::Object',
                       'Variant' => 'HPHP::Variant',
                       'Numeric' => 'HPHP::Variant',
                       'Primitive' => 'HPHP::Variant',
                       'PlusOperand' => 'HPHP::Variant',
                       'Sequence' => 'HPHP::Variant',
                       'Any' => 'HPHP::Variant');

function mapType($t, $byRef = false) {
  global $typeMap;
  if ($byRef) {
    return 'HPHP::VRefParamValue const&';
  }
  if (isset($typeMap[$t])) {
    $t = $typeMap[$t];
  } else {
    $tlen = strlen($t);
    if ($tlen >= 2 && $t[0] === "'" && $t[$tlen-1] === "'") {
      $t = $typeMap['Object'];
    }
  }
  return $t;
}

function mapReturnType($t, $byRef = false) {
  global $returnTypeMap;
  if ($byRef) {
    return 'HPHP::Variant';
  }
  if (isset($returnTypeMap[$t])) {
    $t = $returnTypeMap[$t];
  } else {
    $tlen = strlen($t);
    if ($tlen >= 2 && $t[0] === "'" && $t[$tlen-1] === "'") {
      $t = $returnTypeMap['Object'];
    }
  }
  return $t;
}

function getUniqueFuncName($obj) {
  if ($obj->className === null) {
    return $obj->name;
  } else {
    return strlen($obj->className) . $obj->className . '_' . $obj->name;
  }
}

function emit_include($out, $include) {
  fwrite($out, "#include <$include>\n");
}

function emit_all_includes($out, $sepExtHeaders) {
  // TODO: we should try to cut down on the fat dependencies here
  // (class.h, etc)...
  emit_include($out, "runtime/ext_hhvm/ext_hhvm.h");
  emit_include($out, "runtime/base/builtin_functions.h");
  emit_include($out, "runtime/base/array/array_init.h");
  emit_include($out, "runtime/ext/ext.h");
  emit_include($out, "runtime/vm/class.h");
  emit_include($out, "runtime/vm/runtime.h");
  emit_include($out, "exception");
  foreach($sepExtHeaders as $header) {
    emit_include($out, $header);
  }
  fwrite($out, "\n");
}

function install_file($src, $dst) {
  $dir = dirname($dst);
  `mkdir -p $dir`;
  `mv -f $src $dst`;
}
