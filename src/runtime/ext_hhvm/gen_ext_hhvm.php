<?php

/*
 * This produces a foo.ext_hhvm.cpp for a given object file.  It
 * inspects all the exported names from the .o, compares them to known
 * extension function names, and generates stubs for calling functions
 * or class methods from the VM.  It also generates VM::Instance
 * subclasses for extension classes.
 */

global $scriptPath, $outputPath, $extension_src_path;
global $extension_lib_path, $extensions, $lib_dir;
global $extra_idl;
$scriptPath = dirname(__FILE__);
$output_file = $argv[1];
$ext_hhvm_path = $argv[2];
$lib_dir = $argv[3];
$extension_src_path = $argv[4];
$extension_lib_path = $argv[5];
$current_object_file = $argv[6];
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

$cppPassByRef = array('HPHP::String const&' => true,
                      'HPHP::Array const&' => true,
                      'HPHP::Object const&' => true,
                      'HPHP::Variant const&' => true,
                      'HPHP::Variant' => true,
                      'HPHP::VRefParamValue const&' => true);

$cppReturnByRef = array('HPHP::String' => true,
                        'HPHP::Array' => true,
                        'HPHP::Object' => true,
                        'HPHP::Variant' => true);

$knownStringConstants = array('k_HPHP_TRIM_CHARLIST' => true);

function isTypeCppPassByRef($t) {
  global $cppPassByRef;
  return isset($cppPassByRef[$t]);
}

function isTypeCppReturnByRef($t) {
  global $cppReturnByRef;
  return isset($cppReturnByRef[$t]);
}

function defValNeedsVariable($p) {
  global $knownStringConstants;
  if ($p->defVal == '') {
    return false;
  }
  if (isTypeCppPassByRef($p->type)) {
    if ($p->type == 'HPHP::String const&' &&
        $p->defVal == 'null_string') {
      return false;
    } else if ($p->type == 'HPHP::String const&' &&
               ($p->defVal == 'empty_string')) {
      return false;
    } else if ($p->type == 'HPHP::String const&' &&
               isset($knownStringConstants[$p->defVal])) {
      return false;
    } else if ($p->type == 'HPHP::Array const&' &&
               $p->defVal == 'null_array') {
      return false;
    } else if ($p->type == 'HPHP::Object const&' &&
               $p->defVal == 'null_object') {
      return false;
    } else if ($p->type == 'HPHP::Variant const&' &&
               $p->defVal == 'null_variant') {
      return false;
    } else {
      // All of our tricks have failed, we must set up a local variable
      // to pass by reference
      return true;
    }
  }
  return false;
}

function getDisplayName($obj) {
  if ($obj->className === null) {
    return $obj->name;
  } else {
    return $obj->className . '::' . $obj->name;
  }
}

function emitCall($obj, $prefix, $ref) {
  $ret = '';
  $firstParam = true;
  $fnName = $prefix . getUniqueFuncName($obj);
  $ret .= $fnName . '(';
  $retval = '';
  if ($ref) {
    $retval = '(rv)';
  } else {
    $retval = '(&(rv))';
  }
  if (isTypeCppReturnByRef($obj->returnType)) {
    $firstParam = false;
    if ($obj->returnType == 'HPHP::Variant') {
      $ret .= $retval;
    } else {
      $ret .= '(Value*)' . $retval;
    }
  }
  if ($obj->className !== null) {
    if ($obj->isStatic) {
      if (!$firstParam) {
        $ret .= ', ';
      }
      $firstParam = false;
      $ret .= '("' . $obj->className . '")';
    } else {
      if (!$firstParam) {
        $ret .= ', ';
      }
      $firstParam = false;
      $ret .= '(this_)';
    }
  }
  if ($obj->isVarargs) {
    if (!$firstParam) {
      $ret .= ', ';
    }
    $firstParam = false;
    $ret .= '(count)';
  }
  foreach ($obj->params as $k => $p) {
    if (!$firstParam) {
      $ret .= ', ';
    }
    $firstParam = false;
    if ($p->defVal != '') {
      $ret .= '(count > ' . $k . ') ? ';
    }
    if (isTypeCppPassByRef($p->type)) {
      if ($p->type == 'HPHP::Variant' ||
          $p->type == 'HPHP::Variant const&' ||
          $p->type == 'HPHP::VRefParamValue const&') {
        $ret .= '(args-' . $k . ')';
      } else {
        $ret .= '(Value*)(args-' . $k . ')';
      }
    } else {
      if ($p->type == 'double') {
        $ret .= '(args[-' . $k . '].m_data.dbl)';
      } else {
        $ret .= '(' . $p->type . ')(args[-' . $k . '].m_data.num)';
      }
    }
    if ($p->defVal != '') {
      if (defValNeedsVariable($p)) {
        $ret .= ' : ';
        if ($p->type == 'HPHP::Variant' ||
            $p->type == 'HPHP::Variant const&' ||
            $p->type == 'HPHP::VRefParamValue const&') {
          $ret .= '(TypedValue*)';
        } else {
          $ret .= '(Value*)';
        }
        $ret .= '(&defVal' . $k . ')';
      } else if (isTypeCppPassByRef($p->type)) {
        if ($p->type == 'HPHP::Variant' ||
            $p->type == 'HPHP::Variant const&' ||
            $p->type == 'HPHP::VRefParamValue const&') {
          $ret .= ' : (TypedValue*)(&' . $p->defVal . ')';
        } else {
          $ret .= ' : (Value*)(&' . $p->defVal . ')';
        }
      } else {
        $ret .= ' : (' . $p->type . ')(' . $p->defVal . ')';
      }
    }
  }
  if ($obj->isVarargs) {
    $ret .= ', (Value*)(&extraArgs)';
  }
  $ret .= ')';
  return $ret;
}

function copyAndReturnRV($indent, $obj) {
  $numLocals = $obj->maxNumParams;
  $noThis = ($obj->className === null) || $obj->isStatic;
  $func = $noThis ? "frame_free_locals_no_this_inl"
                  : "frame_free_locals_inl";
  return
    $indent . $func . "(ar, " . $numLocals . ");\n" .
    $indent . "memcpy(&ar->m_r, &rv, sizeof(TypedValue));\n" .
    $indent . "return &ar->m_r;\n";
}

function emitExtCall($obj, $ext_hhvm_cpp, $indent, $prefix) {
  $func_call_prefix = '';
  $func_call_suffix = ";\n" . copyAndReturnRV($indent, $obj);
  if (!isTypeCppReturnByRef($obj->returnType)) {
    if ($obj->returnType == 'bool') {
      fwrite($ext_hhvm_cpp, $indent . 'rv._count = 0;' . "\n");
      fwrite($ext_hhvm_cpp, $indent . 'rv.m_type = KindOfBoolean;' . "\n");
      $func_call_prefix = 'rv.m_data.num = (';
      $func_call_suffix = ") ? 1LL : 0LL;\n" . copyAndReturnRV($indent, $obj);
    } else if ($obj->returnType == 'int') {
      fwrite($ext_hhvm_cpp, $indent . 'rv._count = 0;' . "\n");
      fwrite($ext_hhvm_cpp, $indent . 'rv.m_type = KindOfInt64;' . "\n");
      $func_call_prefix = 'rv.m_data.num = (long long)';
      $func_call_suffix = ";\n" . copyAndReturnRV($indent, $obj);
    } else if ($obj->returnType == 'long long') {
      fwrite($ext_hhvm_cpp, $indent . 'rv._count = 0;' . "\n");
      fwrite($ext_hhvm_cpp, $indent . 'rv.m_type = KindOfInt64;' . "\n");
      $func_call_prefix = 'rv.m_data.num = (long long)';
      $func_call_suffix = ";\n" . copyAndReturnRV($indent, $obj);
    } else if ($obj->returnType == 'double') {
      fwrite($ext_hhvm_cpp, $indent . 'rv._count = 0;' . "\n");
      fwrite($ext_hhvm_cpp, $indent . 'rv.m_type = KindOfDouble;' . "\n");
      $func_call_prefix = 'rv.m_data.dbl = ';
      $func_call_suffix = ";\n" . copyAndReturnRV($indent, $obj);
    } else if ($obj->returnType == 'void') {
      fwrite($ext_hhvm_cpp, $indent . 'rv.m_data.num = 0LL;' . "\n");
      fwrite($ext_hhvm_cpp, $indent . 'rv._count = 0;' . "\n");
      fwrite($ext_hhvm_cpp, $indent . 'rv.m_type = KindOfNull;' . "\n");
      $func_call_prefix = '';
      $func_call_suffix = ";\n" . copyAndReturnRV($indent, $obj);
    } else {
      throw new Exception("Unrecognized return type for " .
                          getDisplayName($obj));
    }
  } else {
    if ($obj->returnType == 'HPHP::String') {
      fwrite($ext_hhvm_cpp, $indent . 'rv._count = 0;' . "\n");
      fwrite($ext_hhvm_cpp, $indent . 'rv.m_type = KindOfString;' . "\n");
      $func_call_prefix = '';
      $func_call_suffix = ";\n" . $indent . 'if (rv.m_data.num == 0LL) ' .
        'rv.m_type = KindOfNull;' . "\n" . copyAndReturnRV($indent, $obj);
    } else if ($obj->returnType == 'HPHP::Array') {
      fwrite($ext_hhvm_cpp, $indent . 'rv._count = 0;' . "\n");
      fwrite($ext_hhvm_cpp, $indent . 'rv.m_type = KindOfArray;' . "\n");
      $func_call_prefix = '';
      $func_call_suffix = ";\n" . $indent . 'if (rv.m_data.num == 0LL) ' .
        'rv.m_type = KindOfNull;' . "\n" . copyAndReturnRV($indent, $obj);
    } else if ($obj->returnType == 'HPHP::Object') {
      fwrite($ext_hhvm_cpp, $indent . 'rv._count = 0;' . "\n");
      fwrite($ext_hhvm_cpp, $indent . 'rv.m_type = KindOfObject;' . "\n");
      $func_call_prefix = '';
      $func_call_suffix = ";\n" . $indent . 'if (rv.m_data.num == 0LL) ' .
        'rv.m_type = KindOfNull;' . "\n" . copyAndReturnRV($indent, $obj);
    } else if ($obj->returnType == 'HPHP::Variant') {
      $func_call_prefix = '';
      $func_call_suffix = ";\n" . $indent . 'if (rv.m_type == KindOfUninit) ' .
        'rv.m_type = KindOfNull;' . "\n" . copyAndReturnRV($indent, $obj);
    } else {
      throw new Exception("Unrecognized return type for " .
                          getDisplayName($obj));
    }
  }
  // Set up extraArgs
  if ($obj->isVarargs) {
    emitBuildExtraArgs($obj, $ext_hhvm_cpp, $indent);
  }
  // Set up variables for default values if needed
  foreach ($obj->params as $k => $p) {
    if (defValNeedsVariable($p)) {
      $defValType = $p->type;
      if (substr($defValType, -7) == ' const&') {
        $defValType = substr($defValType, 0, -7);
      }
      $defValType = preg_replace('/HPHP::/', '', $defValType);
      fwrite($ext_hhvm_cpp, $indent . $defValType . ' defVal' . $k);
      if ($defValType != 'Variant' ||
          ($p->defVal != 'null' && $p->defVal != 'null_variant')) {
        fwrite($ext_hhvm_cpp, ' = ' . $p->defVal);
      }
      fwrite($ext_hhvm_cpp, ";\n");
    }
  }
  // Emit the function call
  fwrite($ext_hhvm_cpp, $indent);
  fwrite($ext_hhvm_cpp, $func_call_prefix);
  fwrite($ext_hhvm_cpp, emitCall($obj, $prefix, false));
  fwrite($ext_hhvm_cpp, $func_call_suffix);
}

function emitBuildExtraArgs($obj, $ext_hhvm_cpp, $indent) {
  fwrite($ext_hhvm_cpp, $indent . "Array extraArgs;\n");
  fwrite($ext_hhvm_cpp, $indent . "{\n");
  fwrite($ext_hhvm_cpp, $indent . '  HPHP::VM::ExtraArgs* ea UNUSED = ' .
                                  'ar->getExtraArgs();' . "\n");
  $argstr = 'extraArg';
  fwrite($ext_hhvm_cpp, $indent . "  ArrayInit ai(count-" . $obj->maxNumParams .
         ", false);\n");
  fwrite($ext_hhvm_cpp, $indent . "  for (long long i = " . $obj->maxNumParams .
         "; i < count; ++i) {\n");
  fwrite($ext_hhvm_cpp, $indent . '    TypedValue* extraArg = ea->getExtraArg(i-' . $obj->maxNumParams . ');'."\n");
  fwrite($ext_hhvm_cpp, $indent . "    if (tvIsStronglyBound(" . $argstr . ")) {\n");
  fwrite($ext_hhvm_cpp, $indent . "      ai.setRef(i-" . $obj->maxNumParams .
         ", tvAsVariant(" . $argstr . "));\n");
  fwrite($ext_hhvm_cpp, $indent . "    } else {\n");
  fwrite($ext_hhvm_cpp, $indent . "      ai.set(i-" . $obj->maxNumParams .
         ", tvAsVariant(" . $argstr . "));\n");
  fwrite($ext_hhvm_cpp, $indent . "    }\n");
  fwrite($ext_hhvm_cpp, $indent . "  }\n");
  fwrite($ext_hhvm_cpp, $indent . "  extraArgs = ai.create();\n");
  fwrite($ext_hhvm_cpp, $indent . "}\n");
}

function emitRemappedFuncDecl($obj, $ext_hhvm_cpp, $indent, $prefix,
                              $mangleMap) {
  $sig = $obj->getHphpSig();
  $regs = array('rdi','rsi','rdx','rcx','r8','r9');
  $dblRegs = array('xmm0','xmm1','xmm2','xmm3','xmm4','xmm5','xmm6','xmm7');
  $nextReg = 0;
  $nextDblReg = 0;
  $nextStackOff = 0;
  fwrite($ext_hhvm_cpp, "/*\n");
  fwrite($ext_hhvm_cpp, $obj->returnType . " " . $sig . "\n" .
         $obj->mangledName . "\n\n");
  if ($obj->returnType == 'double') {
    fwrite($ext_hhvm_cpp, "(return value) => xmm0\n");
  } else if ($obj->returnType != 'void') {
    fwrite($ext_hhvm_cpp, "(return value) => rax\n");
  }
  $newsig = '';
  if (isTypeCppReturnByRef($obj->returnType)) {
    if ($obj->returnType == 'HPHP::Variant') {
      $newsig .= 'TypedValue* ';
    } else {
      $newsig .= 'Value* ';
    }
  } else {
    $newsig .= $obj->returnType . ' ';
  }
  $newsig .= $prefix . getUniqueFuncName($obj) . '(';
  $firstParam = true;
  if (isTypeCppReturnByRef($obj->returnType)) {
    $firstParam = false;
    if ($obj->returnType == 'HPHP::Variant') {
      $newsig .= 'TypedValue* _rv';
    } else {
      $newsig .= 'Value* _rv';
    }
    fwrite($ext_hhvm_cpp, "_rv => " . $regs[$nextReg] . "\n");
    ++$nextReg;
  }

  if ($obj->className !== null) {
    if ($obj->isStatic) {
      if (!$firstParam) {
        $newsig .= ", ";
      }
      $firstParam = false;
      $newsig .= "char const* cls_";
      fwrite($ext_hhvm_cpp, "cls_ => " . $regs[$nextReg] . "\n");
      ++$nextReg;
    } else {
      if (!$firstParam) {
        $newsig .= ", ";
      }
      $firstParam = false;
      $newsig .= "ObjectData* this_";
      fwrite($ext_hhvm_cpp, "this_ => " . $regs[$nextReg] . "\n");
      ++$nextReg;
    }
  }
  if ($obj->isVarargs) {
    if (!$firstParam) {
      $newsig .= ", ";
    }
    $firstParam = false;
    $newsig .= "long long _argc";
    fwrite($ext_hhvm_cpp, "_argc => " . $regs[$nextReg] . "\n");
    ++$nextReg;
  }
  foreach ($obj->params as $p) {
    if (!$firstParam) {
      $newsig .= ", ";
    }
    $firstParam = false;
    if (isTypeCppPassByRef($p->type)) {
      if ($p->type == 'HPHP::Variant' ||
          $p->type == 'HPHP::Variant const&' ||
          $p->type == 'HPHP::VRefParamValue const&') {
        $newsig .= 'TypedValue*';
      } else {
        $newsig .= 'Value*';
      }
      if ($nextReg >= count($regs)) {
        $nextStackOff = ($nextStackOff+7) & ~7;
        fwrite($ext_hhvm_cpp, $p->name . " => st" . $nextStackOff . "\n");
        $nextStackOff += 8;
      } else {
        fwrite($ext_hhvm_cpp, $p->name . " => " . $regs[$nextReg] . "\n");
        ++$nextReg;
      }
    } else {
      $newsig .= $p->type;
      if (($nextReg >= count($regs) && $p->type != 'double') ||
          ($nextDblReg >= count($dblRegs) && $p->type == 'double')) {
        $nextStackOff = ($nextStackOff+7) & ~7;
        fwrite($ext_hhvm_cpp, $p->name . " => st" . $nextStackOff . "\n");
        $nextStackOff += 8;
      } else {
        if ($p->type != 'double') {
          fwrite($ext_hhvm_cpp, $p->name . " => " . $regs[$nextReg] . "\n");
          ++$nextReg;
        } else {
          fwrite($ext_hhvm_cpp, $p->name . " => " . $dblRegs[$nextDblReg] .
                 "\n");
          ++$nextDblReg;
        }
      }
    }
    $newsig .= ' ' . $p->name;
  }
  if ($obj->isVarargs) {
    if (!$firstParam) {
      $newsig .= ", ";
    }
    $firstParam = false;
    $newsig .= "Value* _argv";
    if ($nextReg < count($regs)) {
      fwrite($ext_hhvm_cpp, "_argv => " . $regs[$nextReg] . "\n");
      ++$nextReg;
    } else {
      fwrite($ext_hhvm_cpp, "_argv => st" . $nextStackOff . "\n");
      $nextStackOff += 8;
    }
  }
  $newsig .= ')';
  fwrite($ext_hhvm_cpp, "*/\n\n");

  fwrite($ext_hhvm_cpp, $newsig . ' asm("' . $mangleMap[$sig] . '");' .
         "\n\n");
}

function emitTypeCheckCondition($obj, $ext_hhvm_cpp) {
  $firstParam = true;
  for ($k = $obj->maxNumParams-1; $k >= 0; $k = $k - 1) {
    $p = $obj->params[$k];
    if ($p->type != 'HPHP::Variant' &&
        $p->type != 'HPHP::Variant const&' &&
        $p->type != 'HPHP::VRefParamValue const&') {
      if (!$firstParam) {
        fwrite($ext_hhvm_cpp, ' && ');
      }
      $firstParam = false;
      $optional = ($k >= $obj->minNumParams);
      if ($optional) {
        fwrite($ext_hhvm_cpp, '(count <= ' . $k . ' || ');
      }
      if ($p->type == 'HPHP::String const&') {
        fwrite($ext_hhvm_cpp, 'IS_STRING_TYPE((args-' . $k .
               ')->m_type)');
      } else {
        fwrite($ext_hhvm_cpp, '(args-' . $k . ')->m_type == ');
        switch ($p->type) {
          case 'bool': fwrite($ext_hhvm_cpp, 'KindOfBoolean'); break;
          case 'int': fwrite($ext_hhvm_cpp, 'KindOfInt64'); break;
          case 'long long': fwrite($ext_hhvm_cpp, 'KindOfInt64'); break;
          case 'double': fwrite($ext_hhvm_cpp, 'KindOfDouble'); break;
          case 'HPHP::Array const&':
            fwrite($ext_hhvm_cpp, 'KindOfArray'); break;
          case 'HPHP::Object const&':
            fwrite($ext_hhvm_cpp, 'KindOfObject'); break;
          default: /*ERROR*/ break;
        }
      }
      if ($optional) {
        fwrite($ext_hhvm_cpp, ')');
      }
    }
  }
}

function emitCast($p, $i, $ext_hhvm_cpp, $indent, $doCheck) {
  if ($doCheck) {
    fwrite($ext_hhvm_cpp, $indent . 'if (');
    if ($p->type == 'HPHP::String const&') {
      fwrite($ext_hhvm_cpp, '!IS_STRING_TYPE((args-' . $i .
             ')->m_type)');
    } else {
      fwrite($ext_hhvm_cpp, '(args-' . $i . ')->m_type != ');
      switch ($p->type) {
        case 'bool': fwrite($ext_hhvm_cpp, 'KindOfBoolean'); break;
        case 'int': fwrite($ext_hhvm_cpp, 'KindOfInt64'); break;
        case 'long long': fwrite($ext_hhvm_cpp, 'KindOfInt64'); break;
        case 'double': fwrite($ext_hhvm_cpp, 'KindOfDouble'); break;
        case 'HPHP::Array const&':
          fwrite($ext_hhvm_cpp, 'KindOfArray');
          break;
        case 'HPHP::Object const&':
          fwrite($ext_hhvm_cpp, 'KindOfObject');
          break;
        default: //ERROR
        break;
      }
    }
    fwrite($ext_hhvm_cpp, ") {\n");
    $indent .= '  ';
  }
  fwrite($ext_hhvm_cpp, $indent . 'tvCastTo');
  switch ($p->type) {
    case 'bool': fwrite($ext_hhvm_cpp, 'Boolean'); break;
    case 'int': fwrite($ext_hhvm_cpp, 'Int64'); break;
    case 'long long': fwrite($ext_hhvm_cpp, 'Int64'); break;
    case 'double': fwrite($ext_hhvm_cpp, 'Double'); break;
    case 'HPHP::String const&': fwrite($ext_hhvm_cpp, 'String'); break;
    case 'HPHP::Array const&': fwrite($ext_hhvm_cpp, 'Array'); break;
    case 'HPHP::Object const&': fwrite($ext_hhvm_cpp, 'Object'); break;
    default: //ERROR
    break;
  }
  fwrite($ext_hhvm_cpp, 'InPlace(args-' . $i . ");\n");
  if ($doCheck) {
    $indent = substr($indent, 2);
    fwrite($ext_hhvm_cpp, $indent . "}\n");
  }
}

function emitCasts($obj, $ext_hhvm_cpp, $indent) {
  if ($obj->numTypeChecks == 0) {
    return;
  } else if ($obj->numTypeChecks == 1) {
    for ($i = $obj->maxNumParams-1; $i >= 0; $i = $i - 1) {
      $p = $obj->params[$i];
      if ($p->type != 'HPHP::Variant' &&
          $p->type != 'HPHP::VRefParamValue const&' &&
          $p->type != 'HPHP::Variant const&') {
        // The slow path should only be called when one of the typechecks
        // failed. Thus, if there is only one parameter that required a
        // typecheck, we don't need to check again here.
        emitCast($p, $i, $ext_hhvm_cpp, $indent, false);
      }
    }
    return;
  }
  if ($obj->minNumParams != $obj->maxNumParams) {
    fwrite($ext_hhvm_cpp, $indent . "switch (count) {\n");
    for ($i = $obj->maxNumParams-1; $i > $obj->minNumParams-1; $i = $i - 1) {
      $p = $obj->params[$i];
      if ($i == $obj->maxNumParams-1) {
        fwrite($ext_hhvm_cpp, $indent . 'default: // count >= ' .
               $obj->maxNumParams . "\n");
      } else {
        fwrite($ext_hhvm_cpp, $indent . 'case ' . ($i+1) . ":\n");
      }
      if ($p->type != 'HPHP::Variant' &&
          $p->type != 'HPHP::VRefParamValue const&' &&
          $p->type != 'HPHP::Variant const&') {
        emitCast($p, $i, $ext_hhvm_cpp, $indent . '  ', true);
      }
    }
    fwrite($ext_hhvm_cpp, $indent . 'case ' . $obj->minNumParams . ":\n");
    fwrite($ext_hhvm_cpp, $indent . "  break;\n");
    fwrite($ext_hhvm_cpp, $indent . "}\n");
  }
  for ($i = $obj->minNumParams-1; $i >= 0; $i = $i - 1) {
    $p = $obj->params[$i];
    if ($p->type != 'HPHP::Variant' &&
        $p->type != 'HPHP::VRefParamValue const&' &&
        $p->type != 'HPHP::Variant const&') {
      emitCast($p, $i, $ext_hhvm_cpp, $indent, true);
    }
  }
}

function emitSlowPathHelper($obj, $ext_hhvm_cpp, $indent, $prefix) {
  fwrite($ext_hhvm_cpp, $indent . "TypedValue* args UNUSED = " .
                                  "((TypedValue*)ar) - 1;\n");
  $func_call_prefix = 'return ';
  $func_call_suffix = ";\n";
  if (!isTypeCppReturnByRef($obj->returnType)) {
    if ($obj->returnType == 'bool') {
      fwrite($ext_hhvm_cpp, $indent . 'rv->_count = 0;' . "\n");
      fwrite($ext_hhvm_cpp, $indent . 'rv->m_type = KindOfBoolean;' . "\n");
      $func_call_prefix = 'rv->m_data.num = (';
      $func_call_suffix = ") ? 1LL : 0LL;\n" . $indent . "return rv;\n";
    } else if ($obj->returnType == 'int') {
      fwrite($ext_hhvm_cpp, $indent . 'rv->_count = 0;' . "\n");
      fwrite($ext_hhvm_cpp, $indent . 'rv->m_type = KindOfInt64;' . "\n");
      $func_call_prefix = 'rv->m_data.num = (long long)';
      $func_call_suffix = ";\n" . $indent . "return rv;\n";
    } else if ($obj->returnType == 'long long') {
      fwrite($ext_hhvm_cpp, $indent . 'rv->_count = 0;' . "\n");
      fwrite($ext_hhvm_cpp, $indent . 'rv->m_type = KindOfInt64;' . "\n");
      $func_call_prefix = 'rv->m_data.num = (long long)';
      $func_call_suffix = ";\n" . $indent . "return rv;\n";
    } else if ($obj->returnType == 'double') {
      fwrite($ext_hhvm_cpp, $indent . 'rv->_count = 0;' . "\n");
      fwrite($ext_hhvm_cpp, $indent . 'rv->m_type = KindOfDouble;' . "\n");
      $func_call_prefix = 'rv->m_data.dbl = ';
      $func_call_suffix = ";\n" . $indent . "return rv;\n";
    } else if ($obj->returnType == 'void') {
      fwrite($ext_hhvm_cpp, $indent . 'rv->m_data.num = 0LL;' . "\n");
      fwrite($ext_hhvm_cpp, $indent . 'rv->_count = 0;' . "\n");
      fwrite($ext_hhvm_cpp, $indent . 'rv->m_type = KindOfNull;' . "\n");
      $func_call_prefix = '';
      $func_call_suffix = ";\n" . $indent . "return rv;\n";
    } else {
      throw new Exception("Unrecognized return type for " .
                          getDisplayName($obj));
    }
  } else {
    if ($obj->returnType == 'HPHP::String') {
      fwrite($ext_hhvm_cpp, $indent . 'rv->_count = 0;' . "\n");
      fwrite($ext_hhvm_cpp, $indent . 'rv->m_type = KindOfString;' . "\n");
      $func_call_prefix = '';
      $func_call_suffix = ";\n" . $indent . 'if (rv->m_data.num == 0LL) ' .
        'rv->m_type = KindOfNull;' . "\n" . $indent . 'return rv;' . "\n";
    } else if ($obj->returnType == 'HPHP::Array') {
      fwrite($ext_hhvm_cpp, $indent . 'rv->_count = 0;' . "\n");
      fwrite($ext_hhvm_cpp, $indent . 'rv->m_type = KindOfArray;' . "\n");
      $func_call_prefix = '';
      $func_call_suffix = ";\n" . $indent . 'if (rv->m_data.num == 0LL) ' .
        'rv->m_type = KindOfNull;' . "\n" . $indent . 'return rv;' . "\n";
    } else if ($obj->returnType == 'HPHP::Object') {
      fwrite($ext_hhvm_cpp, $indent . 'rv->_count = 0;' . "\n");
      fwrite($ext_hhvm_cpp, $indent . 'rv->m_type = KindOfObject;' . "\n");
      $func_call_prefix = '';
      $func_call_suffix = ";\n" . $indent . 'if (rv->m_data.num == 0LL)' .
        'rv->m_type = KindOfNull;' . "\n" . $indent . 'return rv;' . "\n";
    } else if ($obj->returnType == 'HPHP::Variant') {
      $func_call_prefix = '';
      $func_call_suffix = ";\n" . $indent . 'if (rv->m_type == KindOfUninit) ' .
        'rv->m_type = KindOfNull;' . "\n" . $indent . 'return rv;' . "\n";
    } else {
      throw new Exception("Unrecognized return type for " .
                          getDisplayName($obj));
    }
  }
  // Cast each argument to the required type as needed
  emitCasts($obj, $ext_hhvm_cpp, $indent);
  // Set up extraArgs
  if ($obj->isVarargs) {
    emitBuildExtraArgs($obj, $ext_hhvm_cpp, $indent);
  }
  // Set up variables for default values if needed
  foreach ($obj->params as $k => $p) {
    if (defValNeedsVariable($p)) {
      $defValType = $p->type;
      if (substr($defValType, -7) == ' const&') {
        $defValType = substr($defValType, 0, -7);
      }
      $defValType = preg_replace('/HPHP::/', '', $defValType);
      fwrite($ext_hhvm_cpp, $indent . $defValType . ' defVal' . $k);
      if ($defValType != 'Variant' ||
          ($p->defVal != 'null' && $p->defVal != 'null_variant')) {
        fwrite($ext_hhvm_cpp, ' = ' . $p->defVal);
      }
      fwrite($ext_hhvm_cpp, ";\n");
    }
  }
  // Emit the function call
  fwrite($ext_hhvm_cpp, $indent);
  fwrite($ext_hhvm_cpp, $func_call_prefix);
  fwrite($ext_hhvm_cpp, emitCall($obj, $prefix, true));
  fwrite($ext_hhvm_cpp, $func_call_suffix);
}

function emit_instance_definition($out, $cname) {
  fwrite($out, "class c_" . $cname . "_Instance");
  fwrite($out, " : public c_" . $cname . " {\n");
  fwrite($out, "public:\n");

  // constructor
  fwrite($out, "  c_" . $cname . "_Instance" .
                        " (HPHP::VM::Class* cls, unsigned nProps) {\n");
                             // See the comment in instance.h
                             // about why we need to treat this as
                             // a GC root during construction.
  fwrite($out, "    DECLARE_STACK_GC_ROOT(ObjectData, this);\n");
  fwrite($out, "    m_cls = cls;\n");
  fwrite($out, "    setAttributes(cls->getODAttrs()\n");
  fwrite($out, "                  | (cls->clsInfo()\n");
  fwrite($out, "                     ? 0 : IsInstance));\n");
  fwrite($out, "    m_propVec = (TypedValue *)((uintptr_t)this" .
                        " + sizeof(c_" . $cname . "));\n");
  fwrite($out, "    if (cls->needInitialization()) {\n");
  fwrite($out, "      cls->initialize();\n");
  fwrite($out, "    }\n");
  fwrite($out, "    if (nProps > 0) {\n");
  fwrite($out, "      if (cls->pinitVec().size() > 0) {\n");
  fwrite($out, "        initialize(nProps);\n");
  fwrite($out, "      } else {\n");
  fwrite($out, "        ASSERT(nProps == " .
                        "cls->declPropInit().size());\n");
  fwrite($out, "        memcpy(m_propVec, " .
                        "&cls->declPropInit()[0], nProps * " .
                        "sizeof(TypedValue));\n");
  fwrite($out, "      }\n");
  fwrite($out, "    }\n");
  fwrite($out, "  }\n");

  // static method for creating a new instance
  fwrite($out, "  static HPHP::VM::Instance* new_Instance" .
                        "(HPHP::VM::Class* cls) {\n");
  fwrite($out, "    size_t nProps = " .
                        "cls->numDeclProperties();\n");
  fwrite($out, "    size_t builtinPropSize = " .
                        "sizeof(c_" . $cname . ") - sizeof(ObjectData);\n");
  fwrite($out, "    size_t size = sizeForNProps(nProps)" .
                        " + builtinPropSize;\n");
  fwrite($out, "    HPHP::VM::Instance *inst = " .
                        "(HPHP::VM::Instance*)ALLOCOBJSZ(size);\n");
  fwrite($out, "    new ((void *)inst) c_" . $cname . "_Instance" .
                        "(cls, nProps);\n");
  fwrite($out, "    return inst;\n");
  fwrite($out, "  }\n");

  // delete operator
  fwrite($out, "  void operator delete(void *p) {\n");
  fwrite($out, "    c_${cname}_Instance *this_ = " .
                        "(c_${cname}_Instance*)p;\n");
  fwrite($out, "    size_t nProps = " .
                        "this_->m_cls->numDeclProperties();\n");
  fwrite($out, "    size_t builtinPropSize UNUSED = " .
                        "sizeof(c_" . $cname . ") - sizeof(ObjectData);\n");
  fwrite($out, "    for (size_t i = 0; i < nProps; ++i) {\n");
  fwrite($out, "      TypedValue *prop = &this_->m_propVec[i];\n");
  fwrite($out, "      tvRefcountedDecRef(prop);\n");
  fwrite($out, "    }\n");
  fwrite($out, "    DELETEOBJSZ(sizeForNProps(nProps) + ".
               "builtinPropSize)(this_);\n");
  fwrite($out, "  }\n");

  // virtual methods that need to be redefined in the leaf
  // o_instanceof()
  fwrite($out, "  virtual bool o_instanceof" .
                        "(const HPHP::String& s) const {\n");
  fwrite($out, "    return Instance::o_instanceof(s) || ".
                        "c_" . $cname . "::o_instanceof(s);\n");
  fwrite($out, "  }\n");

  // o_realProp()
  fwrite($out, "  virtual Variant* o_realProp" .
                       "(CStrRef s, int flags, CStrRef context) const {\n");
  fwrite($out, "    Variant *v = " .
                        "Instance::o_realProp(s, flags, context);\n");
  fwrite($out, "    if (v) return v;\n");
  fwrite($out, "    return c_" . $cname .
                        "::o_realProp(s, flags, context);\n");
  fwrite($out, "  }\n");

  // o_realPropPublic()
  fwrite($out, "  virtual Variant* o_realPropPublic" .
                       "(CStrRef s, int flags) const {\n");
  fwrite($out, "    Variant *v = " .
                        "Instance::o_realPropPublic(s, flags);\n");
  fwrite($out, "    if (v) return v;\n");
  fwrite($out, "    return c_" . $cname .
                        "::o_realPropPublic(s, flags);\n");
  fwrite($out, "  }\n");

  // o_setArray()
  fwrite($out, "  virtual void o_setArray(CArrRef props) {\n");
  fwrite($out, "    ClassInfo::SetArray" .
                        "(this, o_getClassPropTable(), props);\n");
  fwrite($out, "  }\n");

  // o_getArray()
  fwrite($out, "  virtual void o_getArray" .
                        "(Array &props, bool pubOnly) const {\n");
  fwrite($out, "    ClassInfo::GetArray" .
                        "(this, o_getClassPropTable(), props, false);\n");
  fwrite($out, "}\n");

  // cloneImpl
  fwrite($out, "  virtual ObjectData* cloneImpl() {\n");
  fwrite($out, "    return Instance::cloneImpl();\n");
  fwrite($out, "  }\n");

  // cloneSet
  fwrite($out, "  virtual void cloneSet(ObjectData *clone) {\n");
  fwrite($out, "    c_" . $cname . "::cloneSet(clone);\n");
  fwrite($out, "    Instance::cloneSet(clone);\n");
  fwrite($out, "  }\n");

  fwrite($out, "};\n\n");

  // Generate code for leaf constructor
  fwrite($out, "HPHP::VM::Instance* new_" . $cname .
                        "_Instance(HPHP::VM::Class* cls) {\n");
  fwrite($out, "  return c_" . $cname .
                        "_Instance::new_Instance(cls);\n");
  fwrite($out, "}\n\n");

}

function phase2() {
  global $scriptPath, $output_file, $extension_src_path;
  global $extension_lib_path, $extensions;

  $ext_func_info = array();
  $ext_class_info = array();
  $mangleMap = generateMangleMap();
  parseIDLForFunctions($ext_func_info, $mangleMap,
                       $scriptPath . '/../../idl/', 0);
  parseIDLForMethods($ext_class_info, $mangleMap, $scriptPath . '/../../idl/');
  $sepExtDirs = getSepExtDirs($extension_src_path, $extensions);
  $sepExtHeaders = getSepExtHeaders($extension_src_path, $extensions);
  foreach ($sepExtDirs as $dir) {
    parseIDLForFunctions($ext_func_info, $mangleMap, $dir, 0);
    parseIDLForMethods($ext_class_info, $mangleMap, $dir);
  }

  $ext_hhvm_cpp_tempnam = null;
  $ext_hhvm_cpp = null;

  try {
    $ext_hhvm_cpp_tempnam = tempnam('/tmp', 'ext_hhvm.cpp.tmp');
    $ext_hhvm_cpp = fopen($ext_hhvm_cpp_tempnam, 'w');

    emit_all_includes($ext_hhvm_cpp, $sepExtHeaders);
    fwrite($ext_hhvm_cpp, "namespace HPHP {\n\n");

    // Generate code for extension functions
    foreach ($ext_func_info as $obj) {
      if (!$obj->mangledName) continue;

      // Emit the fh_ function declaration
      $indent = '';
      emitRemappedFuncDecl($obj, $ext_hhvm_cpp, $indent, 'fh_', $mangleMap);
      // Emit the fg1_ function if needed
      if ($obj->numTypeChecks > 0) {
        fwrite($ext_hhvm_cpp, "TypedValue * fg1_" . $obj->name .
          "(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) " .
          "__attribute__((noinline,cold));\n");
        fwrite($ext_hhvm_cpp, "TypedValue * fg1_" . $obj->name .
          "(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {\n");
        $indent = '  ';
        emitSlowPathHelper($obj, $ext_hhvm_cpp, $indent, 'fh_');
        fwrite($ext_hhvm_cpp, "}\n\n");
      }
      // Start emitting the fg_ function
      fwrite($ext_hhvm_cpp, "TypedValue* fg_" . $obj->name .
        "(HPHP::VM::ActRec *ar) {\n");
      $indent = '  ';
      fwrite($ext_hhvm_cpp, $indent . "EXCEPTION_GATE_ENTER();\n");
      $indent .= '  ';
      fwrite($ext_hhvm_cpp, $indent . "TypedValue rv;\n");
      fwrite($ext_hhvm_cpp, $indent . "long long count = ar->numArgs();\n");
      fwrite($ext_hhvm_cpp, $indent . "TypedValue* args UNUSED = ((TypedValue*)ar) - 1;\n");
      $firstParam = true;
      $needElseClause = false;
      if ($obj->isVarargs) {
        if ($obj->minNumParams > 0) {
          fwrite($ext_hhvm_cpp, $indent . 'if (count >= ' . $obj->minNumParams .
                 'LL) {' . "\n");
          $indent .= '  ';
          $needElseClause = true;
        }
      } else {
        if ($obj->minNumParams == $obj->maxNumParams) {
          fwrite($ext_hhvm_cpp, $indent . 'if (count == ' . $obj->minNumParams .
                 'LL) {' . "\n");
        } else if ($obj->minNumParams == 0) {
          fwrite($ext_hhvm_cpp, $indent . 'if (count <= ' . $obj->maxNumParams .
                 'LL) {' . "\n");
        } else {
          fwrite($ext_hhvm_cpp, $indent . 'if (count >= ' . $obj->minNumParams .
                 'LL && count <= ' . $obj->maxNumParams . 'LL) {' . "\n");
        }
        $indent .= '  ';
        $needElseClause = true;
      }
      if ($obj->numTypeChecks > 0) {
        fwrite($ext_hhvm_cpp, $indent . 'if (');
        emitTypeCheckCondition($obj, $ext_hhvm_cpp);
        fwrite($ext_hhvm_cpp, ") {\n");
        $indent .= '  ';
      }
      emitExtCall($obj, $ext_hhvm_cpp, $indent, 'fh_');
      if ($obj->numTypeChecks > 0) {
        $indent = substr($indent, 2);
        fwrite($ext_hhvm_cpp, $indent . "} else {\n");
        $indent .= '  ';
        fwrite($ext_hhvm_cpp, $indent . "fg1_" . $obj->name .
               "(&rv, ar, count);\n");
        fwrite($ext_hhvm_cpp, copyAndReturnRV($indent, $obj));
        $indent = substr($indent, 2);
        fwrite($ext_hhvm_cpp, $indent . "}\n");
      }
      if ($needElseClause) {
        $indent = substr($indent, 2);
        fwrite($ext_hhvm_cpp, $indent . '} else {' . "\n");
        $indent .= '  ';
        if ($obj->isVarargs) {
          fwrite($ext_hhvm_cpp, $indent . 'throw_missing_arguments_nr("' .
                 $obj->name . '", count+1, 1);' . "\n");
        } else {
          if ($obj->minNumParams == 0) {
            fwrite($ext_hhvm_cpp, $indent . 'throw_toomany_arguments_nr("' .
                   $obj->name . '", ' . $obj->maxNumParams . ', 1);' . "\n");
          } else {
            fwrite($ext_hhvm_cpp, $indent . 'throw_wrong_arguments_nr("' .
                   $obj->name . '", count, ' . $obj->minNumParams . ', ' .
                   $obj->maxNumParams . ', 1);' . "\n");
          }
        }
        $indent = substr($indent, 2);
        fwrite($ext_hhvm_cpp, $indent . '}' . "\n");
        fwrite($ext_hhvm_cpp, $indent . 'rv.m_data.num = 0LL;' . "\n");
        fwrite($ext_hhvm_cpp, $indent . 'rv._count = 0;' . "\n");
        fwrite($ext_hhvm_cpp, $indent . 'rv.m_type = KindOfNull;' . "\n");
        fwrite($ext_hhvm_cpp, copyAndReturnRV($indent, $obj));
      }
      $indent = substr($indent, 2);
      fwrite($ext_hhvm_cpp, $indent . "EXCEPTION_GATE_RETURN(&ar->m_r);\n");
      fwrite($ext_hhvm_cpp, "}\n\n\n\n");
    }

    // Extension classes; member functions and the actual instance
    // object.
    foreach ($ext_class_info as $cname => $method_info) {
      foreach ($method_info as $obj) {
        if (!$obj->mangledName) continue;

        // Emit the instance definition only in in the file that
        // contains the classes' constructor function.  This is just to
        // avoid multiple definition issues when an extension class is
        // spread among a few cpp files.
        if ($obj->name == "__construct") {
          emit_instance_definition($ext_hhvm_cpp, $cname);
        }

        $indent = '';
        // Emit the th_ function declaration
        emitRemappedFuncDecl($obj, $ext_hhvm_cpp, $indent, 'th_', $mangleMap);
        // Emit the tg1_ function if needed
        if ($obj->numTypeChecks > 0) {
          fwrite($ext_hhvm_cpp, "TypedValue* tg1_" . getUniqueFuncName($obj) .
            "(TypedValue* rv, HPHP::VM::ActRec* ar, long long count" .
            (!$obj->isStatic ? ", ObjectData* this_" : "") .
            ") __attribute__((noinline,cold));\n");
          fwrite($ext_hhvm_cpp, "TypedValue* tg1_" . getUniqueFuncName($obj) .
            "(TypedValue* rv, HPHP::VM::ActRec* ar, long long count" .
            (!$obj->isStatic ? ", ObjectData* this_" : "") .
            ") {\n");
          $indent = '  ';
          emitSlowPathHelper($obj, $ext_hhvm_cpp, $indent, 'th_');
          fwrite($ext_hhvm_cpp, "}\n\n");
          $indent = '';
        }
        // Start emitting the tg_ function
        fwrite($ext_hhvm_cpp, "TypedValue* tg_" . getUniqueFuncName($obj) .
          "(HPHP::VM::ActRec *ar) {\n");
        $indent = '  ';
        fwrite($ext_hhvm_cpp, $indent . "EXCEPTION_GATE_ENTER();\n");
        $indent .= '  ';
        fwrite($ext_hhvm_cpp, $indent . "TypedValue rv;\n");
        fwrite($ext_hhvm_cpp, $indent . "long long count = ar->numArgs();\n");
        fwrite($ext_hhvm_cpp, $indent . "TypedValue* args UNUSED "
                              . "= ((TypedValue*)ar) - 1;\n");
        if (!$obj->isStatic) {
          fwrite($ext_hhvm_cpp,
                 $indent . "ObjectData* this_ = (ar->hasThis() ? " .
                 "ar->getThis() : NULL);\n");
        }
        $firstParam = true;
        $needElseClause = false;
        $needsNullReturn = false;
        if (!$obj->isStatic) {
          fwrite($ext_hhvm_cpp, $indent . "if (this_) {\n");
          $indent .= '  ';
          $needsNullReturn = true;
        }
        if ($obj->isVarargs) {
          if ($obj->minNumParams > 0) {
            fwrite($ext_hhvm_cpp, $indent . 'if (count >= ' .
                   $obj->minNumParams . 'LL) {' . "\n");
            $indent .= '  ';
            $needElseClause = true;
            $needsNullReturn = true;
          }
        } else {
          if ($obj->minNumParams == $obj->maxNumParams) {
            fwrite($ext_hhvm_cpp, $indent . 'if (count == ' .
                   $obj->minNumParams . 'LL) {' . "\n");
          } else if ($obj->minNumParams == 0) {
            fwrite($ext_hhvm_cpp, $indent . 'if (count <= ' .
                   $obj->maxNumParams . 'LL) {' . "\n");
          } else {
            fwrite($ext_hhvm_cpp, $indent . 'if (count >= ' .
                   $obj->minNumParams . 'LL && count <= ' . $obj->maxNumParams .
                   'LL) {' . "\n");
          }
          $indent .= '  ';
          $needElseClause = true;
          $needsNullReturn = true;
        }
        if ($obj->numTypeChecks > 0) {
          fwrite($ext_hhvm_cpp, $indent . 'if (');
          emitTypeCheckCondition($obj, $ext_hhvm_cpp);
          fwrite($ext_hhvm_cpp, ") {\n");
          $indent .= '  ';
        }
        emitExtCall($obj, $ext_hhvm_cpp, $indent, 'th_');
        if ($obj->numTypeChecks > 0) {
          $indent = substr($indent, 2);
          fwrite($ext_hhvm_cpp, $indent . "} else {\n");
          $indent .= '  ';
          fwrite($ext_hhvm_cpp, $indent . "tg1_" .
                 getUniqueFuncName($obj) . "(&rv, ar, count " .
                 (!$obj->isStatic ? ", this_" : "") . ");\n");
          fwrite($ext_hhvm_cpp, copyAndReturnRV($indent, $obj));
          $indent = substr($indent, 2);
          fwrite($ext_hhvm_cpp, $indent . "}\n");
        }
        if ($needElseClause) {
          $indent = substr($indent, 2);
          fwrite($ext_hhvm_cpp, $indent . '} else {' . "\n");
          $indent .= '  ';
          if ($obj->isVarargs) {
            fwrite($ext_hhvm_cpp, $indent . 'throw_missing_arguments_nr("' .
                   $obj->className . '::' . $obj->name . '", count+1, 1);' .
                   "\n");
          } else {
            if ($obj->minNumParams == 0) {
              fwrite($ext_hhvm_cpp, $indent . 'throw_toomany_arguments_nr("' .
                     $obj->className . '::' . $obj->name . '", ' .
                     $obj->maxNumParams . ', 1);' . "\n");
            } else {
              fwrite($ext_hhvm_cpp, $indent . 'throw_wrong_arguments_nr("' .
                     $obj->className . '::' . $obj->name . '", count, ' .
                     $obj->minNumParams . ', ' . $obj->maxNumParams . ', 1);' .
                     "\n");
            }
          }
          $indent = substr($indent, 2);
          fwrite($ext_hhvm_cpp, $indent . '}' . "\n");
        }
        if (!$obj->isStatic) {
          $indent = substr($indent, 2);
          fwrite($ext_hhvm_cpp, $indent . "} else {\n");
          $indent .= '  ';
          fwrite($ext_hhvm_cpp, $indent . 'throw_instance_method_fatal("' .
                 $obj->className . '::' . $obj->name . '");' . "\n");
          $indent = substr($indent, 2);
          fwrite($ext_hhvm_cpp, $indent . "}\n");
        }
        if ($needsNullReturn) {
          fwrite($ext_hhvm_cpp, $indent . 'rv.m_data.num = 0LL;' . "\n");
          fwrite($ext_hhvm_cpp, $indent . 'rv._count = 0;' . "\n");
          fwrite($ext_hhvm_cpp, $indent . 'rv.m_type = KindOfNull;' . "\n");
          fwrite($ext_hhvm_cpp, copyAndReturnRV($indent, $obj));
        }

        $indent = substr($indent, 2);
        fwrite($ext_hhvm_cpp, $indent . "EXCEPTION_GATE_RETURN(&ar->m_r);\n");
        fwrite($ext_hhvm_cpp, "}\n\n");
      }
    }

    fwrite($ext_hhvm_cpp, "\n} // !HPHP\n\n");

    fclose($ext_hhvm_cpp);
    $ext_hhvm_cpp = null;
    `mv -f $ext_hhvm_cpp_tempnam $output_file`;
  } catch (Exception $e) {
    if ($ext_hhvm_cpp) fclose($ext_hhvm_cpp);
    if ($ext_hhvm_cpp_tempnam) `rm -rf $ext_hhvm_cpp_tempnam`;
  }
}

phase2();

