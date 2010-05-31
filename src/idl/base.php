<?php

///////////////////////////////////////////////////////////////////////////////
// constants

define('Reference',   0x0100);
define('Optional',    0x0200);
define('TypeMask',    0x00FF);

// Defining over public/protected/private/abstract is not only not a good idea, but is also not actually possible.
define('AbstractMethod',    0x0100);
define('StaticMethod',      0x0200);
define('VisibilityMask',    0x00FF);
define('PublicMethod',      0);
define('ProtectedMethod',   1);
define('PrivateMethod',     2);
define('PublicProperty',    4);
define('ProtectedProperty', 5);
define('PrivateProperty',   6);

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

$TYPENAMES = array
  (Boolean     => array('name' => 'bool',        'enum' => 'Boolean',    ),
   Int32       => array('name' => 'int',         'enum' => 'Int32',      ),
   Int64       => array('name' => 'int64',       'enum' => 'Int64',      ),
   Double      => array('name' => 'double',      'enum' => 'Double',     ),
   String      => array('name' => 'String',      'enum' => 'String',     ),
   Int64Vec    => array('name' => 'Array',       'enum' => 'Array',      ),
   StringVec   => array('name' => 'Array',       'enum' => 'Array',      ),
   VariantVec  => array('name' => 'Array',       'enum' => 'Array',      ),
   Int64Map    => array('name' => 'Array',       'enum' => 'Array',      ),
   StringMap   => array('name' => 'Array',       'enum' => 'Array',      ),
   VariantMap  => array('name' => 'Array',       'enum' => 'Array',      ),
   Object      => array('name' => 'Object',      'enum' => 'Object',     ),
   Resource    => array('name' => 'Object',      'enum' => 'Object',     ),
   Variant     => array('name' => 'Variant',     'enum' => 'Variant',    ),
   Numeric     => array('name' => 'Numeric',     'enum' => 'Numeric',    ),
   Primitive   => array('name' => 'Primitive',   'enum' => 'Primitive',  ),
   PlusOperand => array('name' => 'PlusOperand', 'enum' => 'PlusOperand',),
   Sequence    => array('name' => 'Sequence',    'enum' => 'Sequence',   ),
   Any         => array('name' => 'Variant',     'enum' => 'Some'        ),
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

// Flags for functions (used in "system/builtin_symbols.cpp")
define('DefaultFlags', 0);
define('VariableArguments', 1);
define('ReferenceVariableArguments', 2);
define('NoEffect', 4);
define('NoInjection', 8);

// Mask for checking the flags related to variable arguments
define('VarArgsMask', (VariableArguments | ReferenceVariableArguments));

function escape_value($val) {
  $val = preg_replace("/\\\\/", "\\\\\\\\", $val);
  $val = preg_replace("/\\\"/", "\\\\\"", $val);
  return $val;
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

///////////////////////////////////////////////////////////////////////////////
// CPP header preamble
function pre($p) {
  global $preamble;
  if (!isset($preamble)) $preamble = "";
  $preamble .= $p . "\n";
}

///////////////////////////////////////////////////////////////////////////////
// dynamic declarations
function dyn() {
  global $dyns;
  if (!isset($dyns)) $dyns = array();
  $dyns += func_get_args();
}

///////////////////////////////////////////////////////////////////////////////
// function definition

function build_function_def($name,
                            $return,
                            $args,
                            $flags = DefaultFlags) {
  $fargs = array();
  $have_first_optional = false;
  $required_arg_count = 0;
  foreach ($args as $arg_name => $arg) {
    $farg = array('name' => strtolower($arg_name));
    if (is_array($arg)) {
      $farg['type'] = $arg[0];
      if (!is_string($arg[1]) || $arg[1] === '') {
        die('default value needs to be a non-empty string for '.$name.'(..'.
            $arg_name.'..)');
      }
      $default = $arg[1];
      $farg['default'] = $default;
      $default = escape_value($default);
      $farg['default_escaped'] = $default;
    } else {
      $farg['type'] = $arg;
    }
    if ($farg['type'] & Optional) {
      $have_first_optional = true;
    } else {
      ++$required_arg_count;
      if ($have_first_optional) {
        die('Required parameters cannot follow optional parameters (function '.$name.', param '.$arg_name.')');
      }
    }
    if ($farg['type'] & Reference) {
      $farg['ref'] = true;
      $farg['type'] = Variant | ($farg['type'] & (~TypeMask));
    }
    $fargs[] = $farg;
  }
  $func = array('name' => strtolower($name),
                'return' => $return,
                'args' => $fargs,
                'required_args' => $required_arg_count,
                'flags' => $flags);
  if ($return & Reference) {
    $func['ref'] = true;
    $func['return'] = Variant;
  }
  return $func;
}

$funcs = array();
function f($name,
           $return = null,
           $args = array(),
           $flags = DefaultFlags) {
  global $funcs;
  $funcs[] = build_function_def($name, $return, $args, $flags);
}

///////////////////////////////////////////////////////////////////////////////
// class definition

function m($flags,
           $name,
           $return = null,
           $args = array(),
           $func_flags = DefaultFlags) {
  $ret = build_function_def($name, $return, $args, $func_flags);
  if ($flags & AbstractMethod) $ret['abstract'] = true;
  $ret['visibility'] = $flags & VisibilityMask;
  $ret['static'] = $flags & StaticMethod;
  return $ret; //build_function_def($name, $return, $args, $func_flags);
}
function p($flags, $name, $type) {
  return array('name' => $name, 'type' => $type, 'flags' => $flags);
}
function ck($name, $type) {
  return array('type' => $type,
               'name' => $name);
}
$classes = array();
function c($name, $parent = null, $interfaces = array(),
           $methods = array(), $consts = array(), $footer = "",
           $properties = array()) {
  global $classes;

  $have_ctor = false;
  $have_dtor = false;
  foreach ($methods as $method) {
    if ($method['name'] == '__construct') $have_ctor = true;
    if ($method['name'] == '__destruct') $have_dtor = true;
  }

  // We don't have the information to autogenerate a ctor def,
  // so make the user do it.
  if (! $have_ctor) {
    printf("ERROR: No constructor defined in IDL for class %s.\n", $name);
    exit(-1);
  }
  // Generate the appropriate IDL for the dtor, if it doesn't exist.
  if (!$have_dtor) {
    $methods[] = m(PublicMethod, '__destruct', Variant);
  }

  $internal_bases = array();
  $interface_bases = array();
  if ($interfaces) {
    foreach ($interfaces as $clsname => $attr) {
      if (!is_string($clsname)) {
        $interface_bases[] = $attr;
      } else if ($attr == 'internal') {
        $internal_bases[] = $clsname;
      }
    }
  }

  $classes[] = array('name'       => $name,
                     'parent'     => $parent,
                     'interfaces' => $interface_bases,
                     'extrabases' => $internal_bases,
                     'consts'     => $consts,
                     'methods'    => $methods,
                     'properties' => $properties,
                     'footer'     => $footer);

}

///////////////////////////////////////////////////////////////////////////////
// constant definition

$constants = array();
function k($name, $type) {
  global $constants;
  $constants[] = array('type' => $type,
                       'name' => $name);
}

///////////////////////////////////////////////////////////////////////////////
// global variable binding (bridge module only)
$global_bindings = array();
function g($name) {
  global $global_bindings;
  $global_bindings[] = $name;
}

///////////////////////////////////////////////////////////////////////////////
// code generation

function typename($type, $prefix = true) {
  if (is_string($type)) {
    if ($prefix) return 'sp_' . strtolower($type);
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

function param_typename($type, $ref) {
  if (is_string($type)) {
    return 'sp_' . strtolower($type);
  }

  global $REFNAMES;
  $name = typename($type);
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
    fprintf($f, 'S(999), "%s"', strtolower($type));
  } else {
    fprintf($f, 'T(%s)', typeenum($type));
  }
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
      fprintf($f, '"%s", ', $arg['default_escaped']);
    } else {
      fprintf($f, 'NULL, ');
    }
    fprintf($f, 'S(%d), ', idx($arg, 'ref') ? 1 : 0);
  }
  fprintf($f, "NULL, ");
  fprintf($f, 'S(%d), ', $func['flags']);
  if ($newline) fprintf($f, "\n");
}

function generateConstCPPInclude($const, $f) {
  fprintf($f, '"%s", T(%s),'. "\n", $const['name'], typeenum($const['type']));
}

function generateClassCPPInclude($class, $f) {
  fprintf($f, '"%s", "%s", ', strtolower($class['name']),
          strtolower($class['parent']));
  foreach ($class['interfaces'] as $if) {
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
}

function generateMethodCPPInclude($method, $f) {
  generateFuncCPPInclude($method, $f, false, 'G');
  fprintf($f, "S(%d), S(%d), S(%d)",
          intval(idx($method, 'abstract') == AbstractMethod),
          intval($method['visibility']),
          intval($method['static'] == StaticMethod));
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
    fprintf($f, '%s %s', param_typename($arg['type'],
                                        idx($arg, 'ref') || $forceRef),
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

function generateFuncCPPHeader($func, $f, $method = false, $forceref = false,
                               $static = false) {
  fprintf($f, '%s%s %s_%s', $static ? 'static ' : '',
          typename($func['return']), $method ? ($static ? "ti" : "t") : "f",
          $func['name']);
  generateFuncArgsCPPHeader($func, $f, $forceref, $static);
  fprintf($f, ";\n");
}

function generateFuncProfileHeader($func, $f) {
  $var_arg = ($func['flags'] & VarArgsMask);
  $args = $func['args'];

  fprintf($f, 'inline %s x_%s', typename($func['return']), $func['name']);
  generateFuncArgsCPPHeader($func, $f, false);
  fprintf($f, " {\n");

  if (!($func['flags'] & NoInjection)) {
    fprintf($f, "  FUNCTION_INJECTION_BUILTIN(%s);\n", $func['name']);
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
    fprintf($f, idx($arg, 'ref') ? 'ref(%s)' : '%s', $arg['name']);
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
  $lowername = strtolower($class['name']);
  foreach ($class['consts'] as $k) {
    $name = typename($k['type']);
    if ($name == 'String') {
      $name = 'StaticString';
    }
    fprintf($f, "extern const %s q_%s_%s;\n", $name, $lowername, $k['name']);
  }

  fprintf($f,
          <<<EOT

///////////////////////////////////////////////////////////////////////////////
// class ${class['name']}


EOT
          );

  fprintf($f, "FORWARD_DECLARE_CLASS(%s);\n", $lowername);
  foreach ($class['properties'] as $p) {
    generatePropertyCPPForwardDeclarations($p, $f);
  }
  fprintf($f, "class c_%s", $lowername);
  if ($class['parent']) {
    fprintf($f, " : public c_" . strtolower($class['parent']));
  } else {
    fprintf($f, " : public ExtObjectData");
  }
  foreach ($class['extrabases'] as $p) {
    fprintf($f, ", public $p");
  }
  $parents = array();
  fprintf($f, " {\n public:\n");
  fprintf($f, "  BEGIN_CLASS_MAP(%s)\n", $lowername);
  if ($class['parent']) {
    $p = $class['parent'];
    fprintf($f, "  RECURSIVE_PARENT_CLASS(%s)\n", strtolower($p));
  }
  if ($class['interfaces']) {
    foreach ($class['interfaces'] as $p) {
      fprintf($f, "  PARENT_CLASS(%s)\n", strtolower($p));
    }
  }
  foreach ($parents as $p) {
    fprintf($f, "  RECURSIVE_PARENT_CLASS(%s)\n", strtolower($p));
  }
  fprintf($f, "  END_CLASS_MAP(%s)\n", $lowername);
  fprintf($f, "  DECLARE_CLASS(%s, %s, %s)\n", $lowername, $class['name'],
          $class['parent'] ? strtolower($class['parent']) : 'ObjectData');
  fprintf($f, "  DECLARE_INVOKES_FROM_EVAL\n");
  fprintf($f, "  ObjectData* dynCreate(CArrRef params, bool init = true);\n");

  fprintf($f, "\n");

  if (!empty($class['properties'])) {
    fprintf($f, "  // properties\n");
    foreach ($class['properties'] as $p) {
      generatePropertyCPPHeader($p, $f);
    }
    fprintf($f, "\n");
  }

  fprintf($f, "  // need to implement\n");
  fprintf($f, "  public: c_%s();\n", strtolower($class['name']));
  fprintf($f, "  public: ~c_%s();\n", strtolower($class['name']));
  foreach ($class['methods'] as $m) {
    generateMethodCPPHeader($m, $class, $f);
  }

  fprintf($f, "\n");
  fprintf($f, "  // implemented by HPHP\n");
  foreach ($class['methods'] as $m) {
    generatePreImplemented($m, $class, $f);
  }
  fprintf($f, $class['footer']);
  fprintf($f, "\n};\n");
}

function generateMethodCPPHeader($method, $class, $f) {
  switch ($method['visibility']) {
  case PrivateMethod:
    $vis = "private";
    break;
  case ProtectedMethod:
    $vis = "protected";
    break;
  default:
    $vis = "public";
  }
  fprintf($f, "  %s: ", $vis);
  generateFuncCPPHeader($method, $f, true,
                        $method['name'] != "__construct" &&
                        strpos($method['name'], "__") === 0,
                        $method['static']);
  if ($method['name'] == "__call") {
    fprintf($f, "  public: Variant doCall(Variant v_name, ");
    fprintf($f, "Variant v_arguments, bool fatal);\n");
  } else if ($method['name'] == "__get") {
    fprintf($f, "  public: Variant doGet(Variant v_name, bool error);\n");
  }
}

function generatePropertyCPPHeader($property, $f) {
  switch ($property['flags']) {
  case PrivateProperty:
    $vis = "private";
    break;
  case ProtectedProperty:
    $vis = "protected";
    break;
  default:
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
    fprintf($f, "  public: c_%s *create", strtolower($class['name']));
    generateFuncArgsCPPHeader($method, $f, true);
    fprintf($f, ";\n");
    fprintf($f, "  public: void dynConstruct(CArrRef Params);\n");
    fprintf($f, "  public: void dynConstructFromEval");
    fprintf($f, "(Eval::VariableEnvironment &env,\n");
    fprintf($f, "                                    ");
    fprintf($f, "const Eval::FunctionCallExpression *call);\n");
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
    fprintf($f, '%s %s', param_typename($arg['type'], idx($arg, 'ref')),
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

  /** commenting out Crutch code generation
  if ($schema_no == 0) {
    fprintf($f, "  Array _schema(ArrayData::Create());\n");
  } else {
    fprintf($f, "  Array _schema(ArrayInit(%d, false)%s.create());\n",
            $schema_no, $schema);
  }
  if ($params_no == 0) {
    fprintf($f, "  Array _params(ArrayData::Create());\n");
  } else {
    fprintf($f, "  Array _params(ArrayInit(%d, true)%s.create());\n",
            $params_no, $params);
  }

  fprintf($f, "  ");
  if ($func['return'] !== null || $need_ret) {
    fprintf($f, "Array _ret = ");
  }
  fprintf($f, "Crutch::Invoke(\"%s\", _schema, _params);\n", $func['name']);
  if ($output) {
    fprintf($f, $output);
  }
  if ($func['return'] !== null) {
    if ($func['return'] == Object || $func['return'] == Resource) {
      fprintf($f, "  return OpaqueObject::GetObject((Variant)_ret[0]);\n");
    } else {
      fprintf($f, "  return (Variant)_ret[0];\n");
    }
  }
  */
  fprintf($f, "  throw NotImplementedException(__func__);\n");
  fprintf($f, "}\n\n");
}

function replaceParams($filename, $header) {
  global $funcs;

  $file = file_get_contents($filename);
  foreach ($funcs as &$func) {
    $var_arg = ($func['flags'] & VarArgsMask);
    $args = $func['args'];

    $search = '\w+\s+f_'.$func['name'].'\s*\(\s*';
    if ($var_arg) $search .= '\w+\s+\w+,\s*';
    for ($i = 0; $i < count($args); $i++) {
      $arg = $args[$i];
      $search .= '\w+\s+\w+\s*';
      if (isset($arg['default'])) {
        if ($header) {
          $search .= '=\s*'.preg_quote($arg['default'], '/').'\s*';
        } else {
          $search .= '\/\*\s*=\s*'.preg_quote($arg['default'], '/').
            '\s*\*\/\s*';
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
        $search .= ',(\s*)\w+\s+\w+\s*\/\*\s*=\s*null_array\s*\*\/\s*';
      }
    }
    $search .= '\)';

    $replace = typename($func['return']).' f_'.$func['name'].'(';
    if ($var_arg) $replace .= 'int _argc, ';
    for ($i = 0; $i < count($args); $i++) {
      $arg = $args[$i];
      $replace .= param_typename($arg['type'], idx($arg, 'ref')).' '.$arg['name'];
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

    if ($header && preg_match("/inline\s+$search/s", $file)) {
      $func['inlined'] = true;
    }

    //var_dump($search, $replace);
    $count = preg_match_all("/$search/s", $file, $m);
    if ($count == 0) {
      if ($header || !isset($func['inlined'])) {
        print $func['name']." not found in $filename\n";
      }
    } else if ($count == 1) {
      $file = preg_replace("/$search/s", $replace, $file);
    } else {
      print "skipped ".$func['name']." in $filename\n";
    }
  }
  file_put_contents($filename, $file);
}
