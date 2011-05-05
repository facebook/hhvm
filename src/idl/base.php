<?php

if (file_exists('../system/globals/constants.php')) {
  @require_once '../system/globals/constants.php';
}

///////////////////////////////////////////////////////////////////////////////
// types

define('Boolean',      1);
define('Int32',        4);
define('Int64',        5);
define('Double',       6);
define('String',       7);
define('Int64Vec',     8);
define('StringVec',    9);
define('VariantVec',  10);
define('Int64Map',    11);
define('StringMap',   12);
define('VariantMap',  13);
define('Object',      14);
define('Resource',    15);
define('Variant',     16);
define('Numeric',     17);
define('Primitive',   18);
define('PlusOperand', 19);
define('Sequence',    20);
define('Any',         21);

define('TypeMask',    0x00FF);
define('Reference',   0x0100);

$TYPENAMES = array
  (Boolean     => array('name' => 'bool',          'enum' => 'Boolean',
                        'idlname' => 'Boolean',    'phpname' => 'bool'),
   Int32       => array('name' => 'int',           'enum' => 'Int32',
                        'idlname' => 'Int32',      'phpname' => 'int'),
   Int64       => array('name' => 'int64',         'enum' => 'Int64',
                        'idlname' => 'Int64',      'phpname' => 'int'),
   Double      => array('name' => 'double',        'enum' => 'Double',
                        'idlname' => 'Double',     'phpname' => 'float'),
   String      => array('name' => 'String',        'enum' => 'String',
                        'idlname' => 'String',     'phpname' => 'string'),
   Int64Vec    => array('name' => 'Array',         'enum' => 'Array',
                        'idlname' => 'Int64Vec',   'phpname' => 'vector'),
   StringVec   => array('name' => 'Array',         'enum' => 'Array',
                        'idlname' => 'StringVec',  'phpname' => 'vector'),
   VariantVec  => array('name' => 'Array',         'enum' => 'Array',
                        'idlname' => 'VariantVec', 'phpname' => 'vector'),
   Int64Map    => array('name' => 'Array',         'enum' => 'Array',
                        'idlname' => 'Int64Map',   'phpname' => 'map'),
   StringMap   => array('name' => 'Array',         'enum' => 'Array',
                        'idlname' => 'StringMap',  'phpname' => 'map'),
   VariantMap  => array('name' => 'Array',         'enum' => 'Array',
                        'idlname' => 'VariantMap', 'phpname' => 'map'),
   Object      => array('name' => 'Object',        'enum' => 'Object',
                        'idlname' => 'Object',     'phpname' => 'object'),
   Resource    => array('name' => 'Object',        'enum' => 'Object',
                        'idlname' => 'Resource',   'phpname' => 'resource'),
   Variant     => array('name' => 'Variant',       'enum' => 'Variant',
                        'idlname' => 'Variant',    'phpname' => 'mixed'),
   Numeric     => array('name' => 'Numeric',       'enum' => 'Numeric',
                        'idlname' => 'Numeric',    'phpname' => 'number'),
   Primitive   => array('name' => 'Primitive',     'enum' => 'Primitive',
                        'idlname' => 'Primitive',  'phpname' => 'num|string'),
   PlusOperand => array('name' => 'PlusOperand',   'enum' => 'PlusOperand',
                        'idlname' => 'PlusOperand','phpname' => 'num|array'),
   Sequence    => array('name' => 'Sequence',      'enum' => 'Sequence',
                        'idlname' => 'Sequence',  'phpname' => 'string|array'),
   Any         => array('name' => 'Variant',       'enum' => 'Some',
                        'idlname' => 'Any',        'phpname' => 'mixed'),
   );

$REFNAMES = array('String'      => 'CStrRef',
                  'Array'       => 'CArrRef',
                  'Object'      => 'CObjRef',
                  'Variant'     => 'CVarRef',
                  'Numeric'     => 'CVarRef',
                  'Primitive'   => 'CVarRef',
                  'PlusOperand' => 'CVarRef',
                  'Sequence'    => 'CVarRef',
                  );

$MAGIC_METHODS = array('__get' => 'ObjectData::UseGet',
                       '__set' => 'ObjectData::UseSet',
                       '__call' => 'ObjectData::HasCall',
                       '__callStatic' => 'ObjectData::HasCallStatic',
                       '__unset' => 'ObjectData::UseUnset',
                       '__isset' => 0);

function get_idl_name($type, $null = '') {
  global $TYPENAMES;
  if ($type === null) {
    return $null;
  }
  if (is_string($type)) {
    return "'$type'";
  }
  if ($type & Reference) {
    return $TYPENAMES[$type & ~Reference]['idlname'] . ' | Reference';
  }
  return $TYPENAMES[$type]['idlname'];
}

function get_php_name($type, $null = 'mixed') {
  global $TYPENAMES;
  if ($type === null) {
    return $null;
  }
  if (is_string($type)) {
    return $type;
  }
  if ($type & Reference) {
    return $TYPENAMES[$type & ~Reference]['phpname'];
  }
  return $TYPENAMES[$type]['phpname'];
}

///////////////////////////////////////////////////////////////////////////////
// flags

// ClassInfo attributes, and these numbers need to be consistent with them!
define('IsAbstract',                     1 <<  4);
define('IsFinal',                        1 <<  5);
define('IsPublic',                       1 <<  6);
define('IsProtected',                    1 <<  7);
define('IsPrivate',                      1 <<  8);
define('IsStatic',                       1 <<  9);
define('HasDocComment',                  1 << 14);
define('HipHopSpecific',                 1 << 16);
define('VariableArguments',              1 << 17);
define('RefVariableArguments',           1 << 18);
define('MixedVariableArguments',         1 << 19);
define('FunctionIsFoldable',             1 << 20);
define('NoEffect',                       1 << 21);
define('NoInjection',                    1 << 22);
define('HasOptFunction',                 1 << 23);
define('AllowIntercept',                 1 << 24);
define('NoProfile',                      1 << 25);
define('ContextSensitive',               1 << 26);
define('NoDefaultSweep',                 1 << 27);

// Mask for checking the flags related to variable arguments
define('VarArgsMask', (VariableArguments | RefVariableArguments |
                       MixedVariableArguments));

function get_flag_names($arr, $name) {
  $flag = 0;
  if (!empty($arr[$name])) {
    $flag |= $arr[$name];
  }
  if ($flag == 0) return '';

  $ret = '';
  if ($flag & IsAbstract            ) $ret .= ' | IsAbstract'            ;
  if ($flag & IsFinal               ) $ret .= ' | IsFinal'               ;
  if ($flag & IsPublic              ) $ret .= ' | IsPublic'              ;
  if ($flag & IsProtected           ) $ret .= ' | IsProtected'           ;
  if ($flag & IsPrivate             ) $ret .= ' | IsPrivate'             ;
  if ($flag & IsStatic              ) $ret .= ' | IsStatic'              ;
  if ($flag & HasDocComment         ) $ret .= ' | HasDocComment'         ;
  if ($flag & HipHopSpecific        ) $ret .= ' | HipHopSpecific'        ;
  if ($flag & VariableArguments     ) $ret .= ' | VariableArguments'     ;
  if ($flag & RefVariableArguments  ) $ret .= ' | RefVariableArguments'  ;
  if ($flag & MixedVariableArguments) $ret .= ' | MixedVariableArguments';
  if ($flag & FunctionIsFoldable    ) $ret .= ' | FunctionIsFoldable'    ;
  if ($flag & NoEffect              ) $ret .= ' | NoEffect'              ;
  if ($flag & NoInjection           ) $ret .= ' | NoInjection'           ;
  if ($flag & HasOptFunction        ) $ret .= ' | HasOptFunction'        ;
  if ($flag & AllowIntercept        ) $ret .= ' | AllowIntercept'        ;
  if ($flag & NoProfile             ) $ret .= ' | NoProfile'             ;
  if ($flag & ContextSensitive      ) $ret .= ' | ContextSensitive'      ;
  if ($flag & NoDefaultSweep        ) $ret .= ' | NoDefaultSweep'        ;

  if ($ret == '') {
    throw new Exception("invalid flag $flag");
  }
  return substr($ret, 2);
}

///////////////////////////////////////////////////////////////////////////////
// schema functions that will be used (and only used) by schemas

$current_class = '';
$preamble = '';
$funcs = array();
$classes = array();
$constants = array();

function DefinePreamble($p) {
  global $preamble;
  $preamble .= $p;
}

function BeginClass($class) {
  global $classes, $current_class;
  $current_class = $class['name'];

  if (!isset($class['parent'])) $class['parent'] = null;
  if (!isset($class['ifaces'])) $class['ifaces'] = array();
  if (!isset($class['bases']))  $class['bases'] = array();

  $class['methods'] = array();
  $class['properties'] = array();
  $class['consts'] = array();

  if (empty($class['flags'])) {
    $class['flags'] = 0;
  }
  $doc = get_class_doc_comments($class);
  if (!empty($doc)) {
    $class['flags'] |= HasDocComment;
    $class['doc'] = $doc;
  } else {
    $class['flags'] &= ~HasDocComment;
    $class['doc'] = null;
  }

  $classes[$current_class] = $class;
}

function EndClass() {
  global $classes, $current_class;

  $have_ctor = false;
  $have_dtor = false;
  foreach ($classes[$current_class]['methods'] as $method) {
    if ($method['name'] == '__construct') $have_ctor = true;
    if ($method['name'] == '__destruct') $have_dtor = true;
  }

  // We don't have the information to autogenerate a ctor def,
  // so make the user do it.
  if (!$have_ctor) {
    throw new Exception("No constructor defined for class $current_class");
  }
  // Generate the appropriate IDL for the dtor, if it doesn't exist.
  if (!$have_dtor) {
    DefineFunction(array('name' => '__destruct',
                         'return' => array('type' => Variant)));
  }

  $current_class = '';
}

function DefineProperty($property) {
  global $classes, $current_class;
  $classes[$current_class]['properties'][] = $property;
}

function DefineConstant($const) {
  global $constants, $classes, $current_class;
  if (empty($current_class)) {
    $constants[] = $const;
  } else {
    $classes[$current_class]['consts'][] = $const;
  }
}

function DefineFunction($func) {
  global $classes, $current_class;

  if (empty($func['flags'])) {
    $func['flags'] = 0;
  }
  if ($current_class && $classes[$current_class]['flags'] & HipHopSpecific) {
    $func['flags'] |= HipHopSpecific;
  }
  if (!isset($func['return'])) $func['return'] = array();
  $func['ret_desc'] = idx($func['return'], 'desc');
  $func['return'] = idx($func['return'], 'type');
  if ($func['return'] & Reference) {
    $func['ref'] = true;
    $func['return'] = Variant | ($func['return'] & (~TypeMask));
  }
  $args = array();
  if (!empty($func['args'])) {
    foreach ($func['args'] as $arg) {
      if (array_key_exists('value', $arg)) {
        if (!is_string($arg['value']) || $arg['value'] === '') {
          throw new Exception('default value has to be non-empty string for '.
                              $func['name'].'(..'.$arg['name'].'..)');
        }
        if (preg_match('/^q_([A-Za-z]+)_(\w+)$/', $arg['value'], $m)) {
          $class = $m[1];
          $constant = $m[2];
          $arg['default'] = "q_${class}_${constant}";
        } else {
          $arg['default'] = $arg['value'];
        }
        $arg['defaultSerialized'] = get_serialized_default($arg['value']);
        $arg['defaultText'] = get_default_text($arg['value']);
      }
      if (idx($arg, 'type') & Reference) {
        $arg['ref'] = true;
        $arg['type'] = Variant | ($arg['type'] & (~TypeMask));
      }
      $args[] = $arg;
    }
    $func['args'] = $args;
  } else {
    $func['args'] = array();
  }

  $doc = get_function_doc_comments($func, $current_class);
  if (!empty($doc)) {
    $func['flags'] |= HasDocComment;
    $func['doc'] = $doc;
  } else {
    $func['flags'] &= ~HasDocComment;
    $func['doc'] = null;
  }

  if (!empty($func['opt'])) {
    $func['flags'] |= HasOptFunction;
  } else {
    $func['flags'] &= ~HasOptFunction;
    $func['opt'] = null;
  }

  global $funcs, $classes, $current_class;
  if (empty($current_class)) {
    $funcs[] = $func;
  } else {
    $classes[$current_class]['methods'][] = $func;
  }
}

///////////////////////////////////////////////////////////////////////////////
// code generation

function typename($type, $prefix = true) {
  if (is_string($type)) {
    if ($prefix) return 'p_' . $type;
    return $type;
  }

  global $TYPENAMES;
  $type = $type & TypeMask;
  if ($type !== 0) {
    if (!isset($TYPENAMES[$type])) {
      exit("Unknown type $type\n");
    }
    return $TYPENAMES[$type]['name'];
  }
  return 'void';
}

function param_typename($arg, $forceRef = false) {
  $type = $arg['type'];
  $ref = idx($arg, 'ref') ?
    ($forceRef === null ?
     "wrap" : ($forceRef === true ? "var" : true)) :
    ($forceRef === true ? "var" : false);

  if (is_string($type)) {
    return 'p_' . $type;
  }

  global $REFNAMES;
  $name = typename($type);
  if ($ref && $name == "Variant") {
    if ($ref === "wrap")
      return "VRefParam";
    else if ($ref === "var")
      return "Variant";
    else
      return "VRefParam";
  }
  if ($ref || !isset($REFNAMES[$name])) {
    return $name;
  }
  return $REFNAMES[$name];
}

function typeenum($type) {
  if (is_string($type)) {
    return 'Void';
  }

  global $TYPENAMES;
  $type = $type & TypeMask;
  if ($type !== 0) {
    return $TYPENAMES[$type]['enum'];
  }
  return 'Void';
}

function fprintType($f, $type) {
  if (is_string($type)) {
    fprintf($f, 'S(999), "%s"', $type);
  } else {
    fprintf($f, 'T(%s)', typeenum($type));
  }
}

function get_serialized_default($s) {
  // These values are special and cannot be returned by
  // ReflectionParameter::getDefaultValue().
  if ($s == 'TimeStamp::Current()' ||
      preg_match('/^k_SQLITE3_/', $s)) {
    return "\x01";
  }

  if (preg_match('/^".*"$/', $s) ||
      preg_match('/^[\-0-9.]+$/', $s) ||
      preg_match('/^0x[0-9a-fA-F]+$/', $s) ||
      preg_match('/^(true|false|null)$/', $s)
     ) {
    return serialize(eval("return $s;"));
  }
  if (preg_match('/^null_(string|array|object|variant)$/', $s)) {
    return serialize(null);
  }
  if (preg_match('/^k_\w+( ?\| ?k_\w+)*$/', $s, $m)) {
    $s = preg_replace('/k_/', '', $s);
    return serialize(eval("return $s;"));
  }
  if (preg_match('/^q_([A-Za-z]+)_(\w+)$/', $s, $m)) {
    $class = $m[1];
    $constant = $m[2];
    return serialize(eval("return $class::$constant;"));
  }
  if ($s == 'RAND_MAX') {
    return serialize(getrandmax());
  }
  if ($s == 'INT_MAX') {
    return serialize((1 << 31) - 1);
  }
  throw new Exception("Unable to serialize default value: [$s]");
}

function get_default_text($s) {
  if (preg_match('/^null_(string|array|object|variant)$/', $s)) {
    return 'null';
  }
  if (preg_match('/^k_\w+( ?\| ?k_\w+)*$/', $s, $m)) {
    return preg_replace('/k_/', '', $s);
  }
  if (preg_match('/^q_([A-Za-z]+)_(\w+)$/', $s, $m)) {
    $class = $m[1];
    $constant = $m[2];
    return "$class::$constant";
  }
  return $s;
}

function generateFuncCPPInclude($func, $f, $newline = true) {
  fprintf($f, '"%s", ', $func['name']);
  fprintType($f, $func['return']);
  fprintf($f, ', S(%d), ', idx($func, 'ref') ? 1 : 0);
  for ($i = 0; $i < count($func['args']); $i++) {
    $arg = $func['args'][$i];
    fprintf($f, '"%s", ', $arg['name']);
    fprintType($f, $arg['type']);
    fprintf($f, ', ');
    if (isset($arg['default'])) {
      fprintf($f, '"%s", ', escape_cpp($arg['defaultSerialized']));
      fprintf($f, '"%s", ', escape_cpp($arg['defaultText']));
    } else {
      fprintf($f, 'NULL, NULL, ');
    }
    fprintf($f, 'S(%d), ', idx($arg, 'ref') ? 1 : 0);
  }
  fprintf($f, "NULL, ");
  fprintf($f, 'S(%d), ', $func['flags']);
  if (!empty($func['doc'])) {
    fprintf($f, '"%s", ', escape_cpp($func['doc']));
  }
  if (!empty($func['opt'])) {
    fprintf($f, 'S(%s), ', $func['opt']);
  }
  if ($newline) fprintf($f, "\n");
}

function generateFuncOptDecls($func, $f) {
  if ($func['opt']) {
    fprintf($f, "extern ExpressionPtr ".
            "%s(CodeGenerator *cg, AnalysisResultConstPtr ar, ".
            "SimpleFunctionCallPtr call, int mode);\n",
            $func['opt']);
  }
}

function generateConstCPPInclude($const, $f) {
  fprintf($f, '"%s", T(%s),'. "\n", $const['name'], typeenum($const['type']));
}

function generateClassCPPInclude($class, $f) {
  fprintf($f, '"%s", "%s", ', $class['name'], strtolower($class['parent']));
  foreach ($class['ifaces'] as $if) {
    fprintf($f, '"%s",', strtolower($if));
  }
  fprintf($f, 'NULL, ');
  foreach ($class['methods'] as $m) {
    generateMethodCPPInclude($m, $f);
    fprintf($f, ",");
  }
  fprintf($f, "NULL,");
  foreach ($class['properties'] as $p) {
    generatePropertyCPPInclude($p, $f);
    fprintf($f, ",");
  }
  fprintf($f, "NULL,");
  foreach ($class['consts'] as $k) {
    fprintf($f, '"%s", T(%s),', $k['name'], typeenum($k['type']));
  }
  fprintf($f, "NULL,\n");
  fprintf($f, 'S(%d), ', $class['flags']);
  if (!empty($class['doc'])) {
    fprintf($f, '"%s", ', escape_cpp($class['doc']));
  }
}

function generateMethodCPPInclude($method, $f) {
  generateFuncCPPInclude($method, $f, false, 'G');
  fprintf($f, "S(%d)", $method['flags']);
}

function generatePropertyCPPInclude($property, $f) {
  fprintf($f, "S(%d), \"%s\", ", $property['flags'], $property['name']);
  fprintType($f, $property['type']);
}

function generateFuncArgsCPPHeader($func, $f, $forceRef = false,
                                   $static = false) {
  $var_arg = ($func['flags'] & VarArgsMask);
  $args = $func['args'];
  fprintf($f, "(");
  if ($static) {
    fprintf($f, "const char* cls ");
  }
  if ($var_arg) fprintf($f, 'int _argc, ');
  for ($i = 0; $i < count($args); $i++) {
    $arg = $args[$i];
    if ($static || $i > 0) fprintf($f, ', ');
    fprintf($f, '%s %s', param_typename($arg, $forceRef),
            $arg['name']);
    if (isset($arg['default'])) {
      fprintf($f, ' = %s', $arg['default']);
    }
  }
  if ($var_arg) {
    fprintf($f, ', CArrRef _argv = null_array');
  }
  fprintf($f, ")");
}

function generateFuncArgsCall($func, $f, $forceRef = false) {
  $var_arg = ($func['flags'] & VarArgsMask);
  $args = $func['args'];
  if ($var_arg) {
    fprintf($f, '_argc, ');
  }
  for ($i = 0; $i < count($args); $i++) {
    $arg = $args[$i];
    fprintf($f, ', ');
    fprintf($f, '%s', $arg['name']);
  }
  if ($var_arg) {
    fprintf($f, ', _argv');
  }
}

function generateFuncCPPHeader($func, $f, $method = false, $forceref = false,
                               $static = false, $class = false) {
  if ($method) {
    fprintf($f, '%s%s %s_%s', $static ? 'static ' : '',
            typename($func['return']), $static ? "ti" : "t",
            strtolower($func['name']));
  } else {
    fprintf($f, '%s f_%s', typename($func['return']), $func['name']);
  }
  generateFuncArgsCPPHeader($func, $f, $forceref, $static);
  fprintf($f, ";\n");

  if ($static && $method) {
    fprintf($f, '  public: static %s t_%s', typename($func['return']),
            strtolower($func['name']));
    // for the actual static call there is no class name needed
    generateFuncArgsCPPHeader($func, $f, $forceref, false);
    fprintf($f, " {\n    return ti_%s(\"%s\"", strtolower($func['name']),
            strtolower($class['name']));
    generateFuncArgsCall($func, $f, $forceref);
    fprintf($f, ");\n  }\n");
  }
}

function generateFuncProfileHeader($func, $f) {
  $var_arg = ($func['flags'] & VarArgsMask);
  $args = $func['args'];

  fprintf($f, 'inline %s x_%s', typename($func['return']), $func['name']);
  generateFuncArgsCPPHeader($func, $f, null);
  fprintf($f, " {\n");

  if (($func['flags'] & NoProfile)) {
    fprintf($f, "  FUNCTION_NOPROFILE_BUILTIN(%s);\n", $func['name']);
  } else if (!($func['flags'] & NoInjection)) {
    fprintf($f, "  FUNCTION_INJECTION_BUILTIN(%s);\n", $func['name']);
  }
  if (isset($func['taint_observer'])) {
    fprintf(
      $f,
      "  TAINT_OBSERVER(%d, %d);\n",
      $func['taint_observer']['set_mask'],
      $func['taint_observer']['clear_mask']);
  } else {
    fprintf($f, "  TAINT_OBSERVER(TAINT_BIT_NONE, TAINT_BIT_NONE);\n");
  }

  fprintf($f, "  ");

  if (typename($func['return']) !== 'void') {
    fprintf($f, "return ");
  }
  fprintf($f, "f_%s(", $func['name']);

  if ($var_arg) fprintf($f, '_argc, ');
  for ($i = 0; $i < count($args); $i++) {
    $arg = $args[$i];
    if ($i > 0) fprintf($f, ', ');
    fprintf($f, '%s', $arg['name']);
  }
  if ($var_arg) {
    fprintf($f, ', _argv');
  }
  fprintf($f, ");\n");

  fprintf($f, "}\n\n");
}

function generateConstCPPHeader($const, $f) {
  $name = typename($const['type']);
  if ($name == 'String') {
    $name = 'StaticString';
  }
  fprintf($f, "extern const %s k_%s;\n", $name, $const['name']);
}

function generateClassCPPHeader($class, $f) {
  global $MAGIC_METHODS;
  $clsname = $class['name'];
  foreach ($class['consts'] as $k) {
    $name = typename($k['type']);
    if ($name == 'String') {
      $name = 'StaticString';
    }
    fprintf($f, "extern const %s q_%s_%s;\n", $name, $clsname, $k['name']);
  }

  fprintf($f,
          <<<EOT

///////////////////////////////////////////////////////////////////////////////
// class ${class['name']}


EOT
          );

  fprintf($f, "FORWARD_DECLARE_CLASS(%s);\n", $clsname);
  foreach ($class['properties'] as $p) {
    generatePropertyCPPForwardDeclarations($p, $f);
  }

  fprintf($f, "class c_%s", $clsname);
  $flags = array();
  foreach ($class['methods'] as $m) {
    $name = $m['name'];
    if (isset($MAGIC_METHODS[$name]) && $MAGIC_METHODS[$name]) {
      $flags[$name] = $MAGIC_METHODS[$name];
    }
  }

  if ($class['parent']) {
    global $classes;
    $pclass = $class;
    while ($flags && $pclass['parent'] && isset($classes[$class['parent']])) {
      $pclass = $classes[$class['parent']];
      foreach ($pclass['methods'] as $m) {
        unset($flags[$m['name']]);
      }
    }
    fprintf($f, " : public c_" . $class['parent']);
  } else {
    fprintf($f, " : public ExtObjectData");
    if ($flags) {
      fprintf($f, "Flags<%s>", implode('|', $flags));
      $flags = false;
    }
  }
  foreach ($class['bases'] as $p) {
    fprintf($f, ", public $p");
  }
  $parents = array();
  fprintf($f, " {\n public:\n");
  fprintf($f, "  BEGIN_CLASS_MAP(%s)\n", $clsname);
  if ($class['parent']) {
    $p = $class['parent'];
    fprintf($f, "  RECURSIVE_PARENT_CLASS(%s)\n", $p);
  }
  foreach ($class['ifaces'] as $p) {
    fprintf($f, "  PARENT_CLASS(%s)\n", $p);
  }
  foreach ($parents as $p) {
    fprintf($f, "  RECURSIVE_PARENT_CLASS(%s)\n", $p);
  }
  fprintf($f, "  END_CLASS_MAP(%s)\n", $clsname);
  fprintf($f, "  DECLARE_CLASS(%s, %s, %s)\n", $clsname, $clsname,
          $class['parent'] ? $class['parent'] : 'ObjectData');

  fprintf($f, "\n");

  if (!empty($class['properties'])) {
    fprintf($f, "  // properties\n");
    foreach ($class['properties'] as $p) {
      generatePropertyCPPHeader($p, $f);
    }
    fprintf($f, "\n");
  }

  fprintf($f, "  // need to implement\n");
  if ($flags) {
    fprintf($f, "  // constructor must call setAttributes(%s)\n",
            implode('|', $flags));
  }
  fprintf($f, "  public: c_%s();\n", $class['name']);
  fprintf($f, "  public: ~c_%s();\n", $class['name']);
  foreach ($class['methods'] as $m) {
    generateMethodCPPHeader($m, $class, $f);
  }

  fprintf($f, "\n");
  fprintf($f, "  // implemented by HPHP\n");
  foreach ($class['methods'] as $m) {
    generatePreImplemented($m, $class, $f);
  }
  if (!empty($class['footer'])) {
    fprintf($f, $class['footer']);
  }
  fprintf($f, "\n};\n");
}

function generateMethodCPPHeader($method, $class, $f) {
  global $MAGIC_METHODS;
  if ($method['flags'] & IsPrivate) {
    $vis = "private";
  } else if ($method['flags'] & IsProtected) {
    $vis = "protected";
  } else {
    $vis = "public";
  }
  fprintf($f, "  %s: ", $vis);
  generateFuncCPPHeader($method, $f, true,
                        isset($MAGIC_METHODS[$method['name']]),
                        $method['flags'] & IsStatic, $class);
  fprintf($f, "  DECLARE_METHOD_INVOKE_HELPERS(%s);\n",
          strtolower($method['name']));
  if ($method['name'] == "__call") {
    fprintf($f, "  public: Variant doCall(Variant v_name, ");
    fprintf($f, "Variant v_arguments, bool fatal);\n");
  }
}

function generatePropertyCPPHeader($property, $f) {
  if ($property['flags'] & IsPrivate) {
    $vis = "private";
  } else if ($property['flags'] & IsProtected) {
    $vis = "protected";
  } else {
    $vis = "public";
  }
  fprintf($f, "  %s: ", $vis);
  fprintf($f, "%s m_%s;\n", typename($property['type']),
          $property['name']);
}

function generatePropertyCPPForwardDeclarations($property, $f) {
  if (is_string($property['type'])) {
    fprintf($f, "FORWARD_DECLARE_CLASS(%s);\n",
            typename($property['type'], false));
  }
}

function generatePreImplemented($method, $class, $f) {
  if ($method['name'] == '__construct') {
    fprintf($f, "  public: c_%s *create", $class['name']);
    generateFuncArgsCPPHeader($method, $f, true);
    fprintf($f, ";\n");
    fprintf($f, "  public: void dynConstruct(CArrRef Params);\n");
    fprintf($f, "  public: void getConstructor(MethodCallPackage &mcp);\n");
  } else if ($method['name'] == '__destruct') {
    fprintf($f, "  public: virtual void destruct();\n", $class['name']);
  }
}

function generateFuncCPPImplementation($func, $f) {
  $schema = "";
  $schema_no = 0;
  if ($func['return'] == Object || $func['return'] == Resource) {
    $schema .= '.set(' . ($schema_no++) . ', -1, "OO")';
  }
  $output = '';
  $need_ret = false;

  fprintf($f, '%s f_%s(', typename($func['return']), $func['name']);
  $var_arg = ($func['flags'] & VarArgsMask);
  if ($var_arg) fprintf($f, 'int _argc, ');
  $params = "";
  $params_no = 0;
  for ($i = 0; $i < count($func['args']); $i++) {
    $arg = $func['args'][$i];
    if ($i > 0) fprintf($f, ', ');
    fprintf($f, '%s %s', param_typename($arg),
            $arg['name']);
    if (isset($arg['default'])) {
      fprintf($f, ' /* = %s */', $arg['default']);
    }

    if ($arg['type'] == Object || $arg['type'] == Resource) {
      $params .= '.set(' . ($params_no++) .
                 ', (OpaqueObject::GetIndex(' . $arg['name'] . '))';
    } else {
      $params .= '.set(' . ($params_no++) . ', ' . $arg['name'] . ')';
    }

    if ($arg['type'] == Object || $arg['type'] == Resource) {
      if (idx($arg, 'ref')) {
        $schema .= '.set(' . ($schema_no++) . ', ' . $i . ', "OO")';
      } else {
        $schema .= '.set(' . ($schema_no++) . ', ' . $i . ', "O")';
      }
    } else if (idx($arg, 'ref')) {
      $schema .= '.set(' . ($schema_no++) . ', ' . $i . ', "R")';
    }

    if (idx($arg, 'ref')) {
      $need_ret = true;
      $output .= '  '.$arg['name'].' = ((Variant)_ret[1])['.$i.'];'."\n";
    }
  }
  if ($var_arg) {
    fprintf($f, ', CArrRef _argv /* = null_array */');
  }
  fprintf($f, ") {\n");
  fprintf($f, "  throw NotImplementedException(__func__);\n");
  fprintf($f, "}\n\n");
}

function replaceParams($filename, $header) {
  global $funcs;

  $orig = $file = file_get_contents($filename);
  foreach ($funcs as &$func) {
    $var_arg = ($func['flags'] & VarArgsMask);
    $args = $func['args'];

    $search = '(?!return\s)\b\w+\s+f_'.$func['name'].'\s*\(\s*';
    if ($var_arg) $search .= '\w+\s+\w+,\s*';
    for ($i = 0; $i < count($args); $i++) {
      $arg = $args[$i];
      $search .= '\w+\s+\w+\s*';
      if (isset($arg['default'])) {
        if ($header) {
          $search .= '=\s*(?:'.preg_quote($arg['default'], '/').'|\d+)\s*';
        } else {
          $search .= '(?:\/\*\s*=\s*(?:'.preg_quote($arg['default'], '/').
            '|\d+)\s*\*\/\s*)?';
        }
      }
      if ($i < count($args) - 1) {
        $search .= ',(\s*)';
      }
    }
    if ($var_arg) {
      if ($header) {
        $search .= ',\s*\w+\s+\w+\s*=\s*null_array\s*';
      } else {
        $search .= ',(\s*)\w+\s+\w+\s*(?:\/\*\s*=\s*null_array\s*\*\/\s*)?';
      }
    }
    $search .= '\)';

    $replace = typename($func['return']).' f_'.$func['name'].'(';
    if ($var_arg) $replace .= 'int _argc, ';
    for ($i = 0; $i < count($args); $i++) {
      $arg = $args[$i];
      $replace .= param_typename($arg).' '.$arg['name'];
      if (isset($arg['default'])) {
        if ($header) {
          $replace .= ' = '.addcslashes($arg['default'], '\\');
        } else {
          $replace .= ' /* = '.addcslashes($arg['default'], '\\').' */';
        }
      }
      if ($i < count($args) - 1) {
        $replace .= ',${'.($i+1).'}';
      }
    }
    if ($var_arg) {
      if ($header) {
        $replace .= ', CArrRef _argv = null_array';
      } else {
        $replace .= ',${'.($i).'}';
        $replace .= 'CArrRef _argv /* = null_array */';
      }
    }
    $replace .= ')';

    if ($header && preg_match("/inline\s+$search/ms", $file)) {
      $func['inlined'] = true;
    }

    //var_dump($search, $replace);
    $count = preg_match_all("/$search/ms", $file, $m);
    if ($count == 0) {
      if ($header || !isset($func['inlined'])) {
        var_dump($search, $replace);
        print $func['name']." not found in $filename\n";
      }
    } else if ($count == 1) {
      $file = preg_replace("/$search/ms", $replace, $file);
    } else {
      print "skipped ".$func['name']." in $filename\n";
    }
  }
  if ($orig != $file) {
    file_put_contents($filename, $file);
  }
}

///////////////////////////////////////////////////////////////////////////////
// helpers

function escape_php($val) {
  $val = preg_replace("/\\\\/", "\\\\\\\\", $val);
  $val = preg_replace("/\\\"/", "\\\\\"",   $val);
  $val = preg_replace("/\\$/",  "\\\\$",    $val);
  $val = preg_replace("/\n/",   "\\\\n",    $val); // optional
  return $val;
}

function escape_cpp($val) {
  $len = strlen($val);
  $ret = '';
  for ($i = 0; $i < $len; $i++) {
    $ch = $val[$i];
    switch ($ch) {
      case "\n": $ret .= "\\n";  break;
      case "\r": $ret .= "\\r";  break;
      case "\t": $ret .= "\\t";  break;
      case "\a": $ret .= "\\a";  break;
      case "\b": $ret .= "\\b";  break;
      case "\f": $ret .= "\\f";  break;
      case "\v": $ret .= "\\v";  break;
      case "\0": $ret .= "\\000";break;
      case "\"": $ret .= "\\\""; break;
      case "\\": $ret .= "\\\\"; break;
      case "?":  $ret .= "\\?";  break; // avoiding trigraph errors
      default:
        if (ord($ch) >= 0x20 && ord($ch) <= 0x7F) {
          $ret .= $ch;
        } else {
          $ret .= sprintf("\\%03o", ord($ch));
        }
        break;
    }
  }
  return $ret;
}

function idx($arr, $idx, $default=null) {
  if ($idx === null) {
    return $default;
  }
  if (isset($arr[$idx])) {
    return $arr[$idx];
  }
  return $default;
}

function format_doc_desc($arr, $clsname) {
  if ($arr['flags'] & HipHopSpecific) {
    $desc = "( HipHop specific )\n";
  } else {
    $clsname = preg_replace('#_#', '-', strtolower($clsname));
    $name = preg_replace('#_#', '-', strtolower($arr['name']));
    $url = "http://php.net/manual/en/$clsname.$name.php";
    $desc = "( excerpt from $url )\n";
  }
  $details = idx($arr, 'desc', '');
  if ($details) {
    $desc .= "\n$details";
  }
  return wordwrap($desc, 72)."\n\n";
}

function format_doc_arg($name, $type, $desc) {
  $width1 = 12;
  $width2 = 8;

  if (!$desc) $desc = ' ';
  $lines = explode("\n", wordwrap($desc, 72 - $width1 - $width2));

  $col = str_pad('@'.$name, $width1 - 1);
  $ret = $col;
  if (strlen($col) >= $width1) {
    $ret .= "\n".str_repeat(' ', $width1 - 1);
  }

  $col = str_pad(get_php_name($type), $width2 - 1);
  $ret .= ' '.$col;
  if (strlen($col) >= $width2) {
    $ret .= "\n".str_repeat(' ', $width1 + $width2 - 1);
  }

  $ret .= ' '.$lines[0]."\n";
  for ($i = 1; $i < count($lines); $i++) {
    $ret .= rtrim(str_repeat(' ', $width1 + $width2) . $lines[$i])."\n";
  }
  return $ret;
}

function format_doc_comment($text) {
  $lines = explode("\n", $text);
  $ret = "/**\n";
  for ($i = 0; $i < count($lines) - 1; $i++) {
    $line = $lines[$i];
    $ret .= rtrim(" * $line")."\n";
  }
  $ret .= " */";
  return $ret;
}

function get_function_doc_comments($func, $clsname) {
  $text = format_doc_desc($func, empty($clsname) ? 'function' : $clsname);

  if ($func['args']) {
    foreach ($func['args'] as $arg) {
      $desc = idx($arg, 'desc', '');
      if (idx($func, 'ref')) {
        $desc = '(output) ' . $desc;
      }
      $text .= format_doc_arg($arg['name'], idx($arg, 'type'), $desc);
    }
  }
  $ret = ($func['return'] !== null || !empty($func['ret_desc']));
  if ($func['args'] && $ret) {
    $text .= "\n";
  }
  if ($ret) {
    $text .= format_doc_arg('return', $func['return'], $func['ret_desc']);
  }

  return format_doc_comment($text);
}

function get_class_doc_comments($class) {
  return format_doc_comment(format_doc_desc($class, 'class'));
}

///////////////////////////////////////////////////////////////////////////////
// phpnet

function phpnet_clean($text) {
  $text = preg_replace('#<!--UdmComment.*?/UdmComment-->#s', '', $text);
  $text = preg_replace('#<div class="example-contents">.*?</div>#s',
                       '<>', $text);
  $text = preg_replace('#<p class="para">#', '<>', $text);
  $text = preg_replace('#<b class="note">Note</b>:#', '', $text);
  $text = preg_replace('#<.+?>#', '', $text);
  $text = preg_replace('#[ \t\n]+#s', ' ', $text);
  $text = preg_replace('# ?<> ?#', "\n\n", $text);
  $text = preg_replace('/&#039;/', "'", $text);
  $text = trim(html_entity_decode($text));
  return $text;
}

function phpnet_get_function_info($name, $clsname = 'function') {
  $clsname = preg_replace('#_#', '-', strtolower($clsname));
  $name = preg_replace('#_#', '-', strtolower($name));
  $doc = @file_get_contents("http://php.net/manual/en/$clsname.$name.php");
  if ($doc === false) {
    return array();
  }

  $ret = array();
  if (preg_match('#<div class="refsect1 description">(.*?)'.
                 '<div class="refsect1 #s', $doc, $m)) {
    $desc = $m[1];
    if (preg_match('#<p class="para rdfs-comment">(.*)</div>#s', $desc, $m)) {
      $ret['desc'] = phpnet_clean($m[1]);
    }
  }

  if (preg_match('#<div class="refsect1 parameters">(.*?)'.
                 '<div class="refsect1 #s', $doc, $m)) {
    $desc = $m[1];
    if (preg_match_all('#<tt class="parameter">(.*?)</tt>#s', $desc, $m)) {
      foreach ($m[1] as $param) {
        $ret['param_names'][] = phpnet_clean($param);
      }
    }
    if (preg_match_all('#<dd>(.*?)</dd>#s', $desc, $m)) {
      foreach ($m[1] as $param) {
        $ret['params'][] = phpnet_clean($param);
      }
    }
    $desc = preg_replace('#<h3.*</h3>#', '', $desc);
    $desc = preg_replace('#<dl>.*</dl>#s', '', $desc);
    $desc = phpnet_clean($desc);
    if (!empty($desc)) {
      $ret['desc'] .= "\n$desc";
    }
  }

  if (preg_match('#<div class="refsect1 returnvalues">(.*?)'.
                 '(<div class="refsect1 |<div id="usernotes">)#s', $doc, $m)) {
    $desc = $m[1];
    $desc = preg_replace('#<h3.*</h3>#', '', $desc);
    $ret['ret'] = phpnet_clean($desc);
  }

  return $ret;
}

function phpnet_get_class_desc($name) {
  $name = preg_replace('#_#', '-', strtolower($name));
  $doc = @file_get_contents("http://php.net/manual/en/class.$name.php");
  if ($doc !== false &&
      preg_match('#<h2 class="title">Introduction</h2>(.*?)'.
                 '<div class="section"#s', $doc, $m)) {
    return phpnet_clean($m[1]);
  }
  return false;
}

function phpnet_get_extension_functions($name) {
  $doc = @file_get_contents("http://www.php.net/manual/en/ref.$name.php");
  if ($doc === false) {
    return false;
  }

  preg_match_all('#<li><a href="function\..*?\.php">(.*?)</a>.*?</li>#',
                 $doc, $m);
  return $m[1];
}
