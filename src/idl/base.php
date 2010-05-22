<?php

///////////////////////////////////////////////////////////////////////////////
// constants

define('Reference',   0x0100);
define('Optional',    0x0200);
define('TypeMask',    0x00FF);

// Defining over public/protected/private/abstract is not only not a good idea, but is also not actually possible.
define('PublicMethod',      0);
define('ProtectedMethod',   1);
define('PrivateMethod',     2);
define('AbstractMethod',    0x0100);
define('StaticMethod',      0x0200);
define('VisibilityMask',    0x00FF);

define('Boolean',      1);
//define('Byte',         2);
//define('Int16',        3);
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
   //Byte        => array('name' => 'char',        'enum' => 'Byte',       ),
   //Int16       => array('name' => 'short',       'enum' => 'Int16',      ),
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
function p($p) {
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
function ck($name, $type) {
  return array('type' => $type,
               'name' => $name);
}
$classes = array();
function c($name, $parent = null, $interfaces = array(),
           $methods = array(), $consts = array(), $footer = "") {
  global $classes;

  $have_ctor = false;
  $have_dtor = false;
  foreach ($methods as $method) {
    if ($method['name'] == '__construct') $have_ctor = true;
    if ($method['name'] == '__destruct') $have_dtor = true;
  }

  // We don't have the information to autogenerate a ctor def, so make the user do it.
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
      if (empty($clsname)) {
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

function typename($type) {
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
  global $REFNAMES;
  $name = typename($type);
  if ($ref || !isset($REFNAMES[$name])) {
    return $name;
  }
  return $REFNAMES[$name];
}

function typeenum($type) {
  global $TYPENAMES;
  $type = $type & TypeMask;
  if ($type !== 0) {
    return $TYPENAMES[$type]['enum'];
  }
  return 'Void';
}

function generateFuncCPPInclude($func, $f, $newline = true) {
  fprintf($f, '"%s", T(%s), ', $func['name'],
          typeenum($func['return']));
  fprintf($f, 'S(%d), ', idx($func, 'ref') ? 1 : 0);
  for ($i = 0; $i < count($func['args']); $i++) {
    $arg = $func['args'][$i];
    fprintf($f, '"%s", T(%s), ', $arg['name'], typeenum($arg['type']));
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
    $vis = "public";
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
    fprintf($f, "  public: Variant doCall(Variant v_name, Variant v_arguments,".
            " bool fatal);\n");
  } else if ($method['name'] == "__get") {
    fprintf($f, "  public: Variant doGet(Variant v_name, bool error);\n");
  }
}

function generatePreImplemented($method, $class, $f) {
  if ($method['name'] == '__construct') {
    fprintf($f, "  public: c_%s *create", strtolower($class['name']));
    generateFuncArgsCPPHeader($method, $f, true);
    fprintf($f, ";\n");
    fprintf($f, "  public: void dynConstruct(CArrRef Params);\n");
    fprintf($f, "  public: void dynConstructFromEval(Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *call);\n");
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

function generateZValToHPHPConversion($arg, $argrval) {
  global $TYPENAMES;
  if (idx($arg, 'ref')) {
    return 'ref(zval_to_variant('.$argrval.'))';
  }

  return 'zval_to_'.strtolower($TYPENAMES[($arg['type'] & TypeMask)]['name']).'('.$argrval.')';
}

function generateHPHPToZValConversion($argtype, $zvalp, $argrval) {
  global $TYPENAMES;
  $conv_fn = strtolower($TYPENAMES[$argtype]['name']).'_to_zval';
  if ($conv_fn == 'variant_to_zval' || $conv_fn == 'array_to_zval') return sprintf('%s(%s, %s);', $conv_fn, $argrval, $zvalp);
  return sprintf('%s(%s, *%s);', $conv_fn, $argrval, $zvalp);
}

function generatePHPBridgeImplementation($func, $f, $classname = NULL, $static = false) {
  if ($classname) {
    fprintf($f, "PHP_METHOD(%s, %s) {\n", $classname, $func['name']);
    if (!$static) {
      if ($func['name'] == '__construct') {
        // create the object, and store objptr in the newly constructed class's properties
        fprintf($f, "  c_%s* objptr = new c_%s();\n", $classname, $classname);
        fprintf($f, "  objptr->incRefCount();\n");
        fprintf($f, "  set_objptr_for_zobject(getThis(), objptr, %s_objptr_propname, %s_objptr_propname_len, %s_objptr_propname_hash);\n", $classname, $classname, $classname);
      } else {
        // retrieve the objptr from the class properties
        fprintf($f, "  c_%s* objptr = reinterpret_cast<c_%s*>(get_objptr_for_zobject(getThis(), %s_objptr_propname, %s_objptr_propname_len, %s_objptr_propname_hash));\n", $classname, $classname, $classname, $classname, $classname);
      }
    }
  } else {
    fprintf($f, "PHP_FUNCTION(%s) {\n", $func['name']);
  }
  fprintf($f, "  if (ht < %d) ZEND_WRONG_PARAM_COUNT();\n", $func['required_args']);
  fprintf($f, "  try {\n");
  fprintf($f, "    resolve_zval_references();\n");
  fprintf($f, "    boost::scoped_array<zval**> args(new zval**[ht]);\n");
  fprintf($f, "    zend_get_parameters_array_ex(ht, args.get());\n");

  $argc = count($func['args']);
  $argnames = array();
  for ($i = 0; $i < $argc; ++$i) {
    $arg = $func['args'][$i];
    $argname = 'a_'.$arg['name'];
    $arg_decl = typename($arg['type']).' '.$argname;
    if ($arg['type'] & Optional) {
      fprintf($f, "    %s;\n", $arg_decl);
      fprintf($f, "    if (ht >= %d) %s = %s;\n", $i+1, $argname, generateZValToHPHPConversion($arg, '*args['.$i.']'), $i);
    } else {
      fprintf($f, "    %s = %s;\n", $arg_decl, generateZValToHPHPConversion($arg, '*args['.$i.']'), $i);
    }
    if (!empty($arg['ref'])) fprintf($f, "  %s.setContagious();\n", $argname);
    $argnames[] = $argname;
  }
  $var_arg = ($func['flags'] & VarArgsMask);
  if ($var_arg) {
    fprintf($f,
      "    Array va_params;\n".
      "    for (int i = %d; i < ht; ++i) {\n".
      "      va_params.append(zval_to_variant(*(args.get()[i])));\n".
      "    }\n", $argc);
    $argnames[] = 'ht'; // num_args
    $argnames[] = 'va_params';
  }

  if ($func['name'] == '__destruct') {
    // The __destruct function only gets called via bridged_object_del_ref, which checks the c++ refcount too
    // We are guaranteed to go into this with refcount == 0.
    fprintf($f, "    assert(objptr->getCount() == 0);\n");
    fprintf($f, "    objptr->t___destruct();\n");
    fprintf($f, "    delete objptr;\n");
  } else {
    if ($func['return'] !== null) {
      fprintf($f, "    %s rv;\n", typename($func['return']));
    }

    if (count($func['args']) == $func['required_args']) {
      fprintf($f, "    ");
      if ($func['return'] !== null) {
        fprintf($f, "rv = ");
      }
      if ($classname && !$static) {
        fprintf($f, "objptr->");
      }
      fprintf($f, "%s(%s);\n", ($classname ? 't_' : 'f_') . $func['name'], implode(',',$argnames));
    } else {
      fprintf($f, "    switch (ht) {\n");
      for ($i = $func['required_args']; $i <= count($func['args']); ++$i) {
        fprintf($f, "      case %d:", $i);
        if ($i == count($func['args'])) fprintf($f, "\n      default:");
        if ($func['return'] !== null) fprintf($f, ' rv =');
        if ($classname && !$static) {
          fprintf($f, ' objptr->');
        }
        fprintf($f, " %s(%s); break;\n", ($classname ? 't_' : 'f_') . $func['name'], implode(',',array_slice($argnames, 0, $i)));
      }
      fprintf($f, "    };\n");
    }
  }
  if ($func['return'] !== null) {
    fprintf($f, "    if (!return_value_ptr) return_value_ptr = &return_value;\n");
    fprintf($f, "    if (return_value_used) { %s }\n", generateHPHPToZValConversion($func['return'], 'return_value_ptr', 'rv'));
    fprintf($f, "    else ZVAL_NULL(return_value);\n");
  } else {
    fprintf($f, "    ZVAL_NULL(return_value);\n");
  }

  fprintf($f, <<<EOT
    resolve_zval_references();

  } catch (const HPHP::Exception& ex) {
    EG(exception) = exception_to_zval(ex);
    ZVAL_NULL(return_value);
  }
}

EOT
          );
}

function generatePHPArgInfo($struct_name, $func_args, $f) {
  fprintf($f, "static ZEND_BEGIN_ARG_INFO_EX(%s, 0, ZEND_RETURN_REFERENCE, -1)\n", $struct_name);
  foreach ($func_args as $arg) {
    fprintf($f, "  ZEND_ARG_INFO(%d, %s)\n", empty($arg['ref']) ? 0 : 1, $arg['name']);
  }
  fprintf($f, "ZEND_END_ARG_INFO()\n");
}

function generatePHPBridgeModuleHeader($module_name, $f) {
  global $funcs, $classes, $global_bindings;
  // Generate PHP_FUNCTION and PHP_METHOD forward decls
  foreach ($classes as $class) {
    foreach ($class['methods'] as $method) {
      fprintf($f, "PHP_METHOD(%s, %s);\n", $class['name'], $method['name']);
      generatePHPArgInfo(sprintf("method_%s_%s_arginfo", $class['name'], $method['name']), $method['args'], $f);
    }
  }

  foreach ($funcs as $func) {
    fprintf($f, "PHP_FUNCTION(%s);\n", $func['name'], $func['name']);
    generatePHPArgInfo($func['name'] . '_arginfo', $func['args'], $f);
  }

  foreach ($classes as $class) {
    fprintf($f, "static function_entry %s_me_table[] = {\n", $class['name']);
    foreach ($class['methods'] as $method) {
      $vis = '';
      switch ($method['visibility'] & VisibilityMask) {
        case PrivateMethod: $vis = 'ZEND_ACC_PRIVATE'; break;
        case ProtectedMethod: $vis = 'ZEND_ACC_PROTECTED'; break;
        case PublicMethod: default: $vis = 'ZEND_ACC_PUBLIC'; break;
      }
      if ($method['name'] == '__construct') $vis .= '|ZEND_ACC_CTOR';
      if ($method['name'] == '__destruct') $vis .= '|ZEND_ACC_DTOR';
      if ($method['name'] == '__clone') $vis .= '|ZEND_ACC_CLONE';
      fprintf($f, "  PHP_ME(%s, %s, method_%s_%s_arginfo, %s)\n", $class['name'], $method['name'], $class['name'], $method['name'], $vis);
    }
    fprintf($f, "  {NULL, NULL, NULL} };\n");
    fprintf($f, "zend_class_entry* %s_ce = NULL;\n", $class['name']);
    fprintf($f, "char* %s_objptr_propname = \"\\0%s\\0__hphp_objptr\";\n", $class['name'], $class['name']);
    fprintf($f, "const size_t %s_objptr_propname_len = %d;\n", $class['name'], strlen($class['name']) + strlen('__hphp_objptr') + 2); // length includes the 2 internal NULLs, but not the NULL terminator.
    fprintf($f, "const ulong %s_objptr_propname_hash = zend_get_hash_value(%s_objptr_propname, %s_objptr_propname_len + 1);\n", $class['name'], $class['name'], $class['name']);
  }
  fprintf($f, "\n\n");

  fprintf($f, "static function_entry %s_fe_table[] = {\n", $module_name);
  foreach ($funcs as $func) {
    fprintf($f, "  PHP_FE(%s, %s_arginfo)\n", $func['name'], $func['name']);
  }
  fprintf($f, "  {NULL, NULL, NULL} };\n");

  fprintf($f, "\n\n");
  fprintf($f, "PHP_MINIT_FUNCTION(%s);\n", $module_name);
  fprintf($f, "PHP_RINIT_FUNCTION(%s);\n", $module_name);
  fprintf($f, "PHP_RSHUTDOWN_FUNCTION(%s);\n", $module_name);

  fprintf($f, "zend_module_entry %s_module_entry = {\n", $module_name);
  fprintf($f, "  STANDARD_MODULE_HEADER, \"%s\", NULL,\n", $module_name);
  fprintf($f, "  PHP_MINIT(%s), NULL, PHP_RINIT(%s), PHP_RSHUTDOWN(%s), NULL, \"1.0\", STANDARD_MODULE_PROPERTIES };\n", $module_name, $module_name, $module_name);
  fprintf($f, "ZEND_GET_MODULE(%s)\n", $module_name);

  fprintf($f, "\n\n");
  fprintf($f, "PHP_MINIT_FUNCTION(%s) {\n", $module_name);
  fprintf($f, "  register_invocation_handler(zend_invoke_system_handler);\n");
  foreach ($classes as $class) {
    fprintf($f, "  zend_class_entry %s_tmp_ce;\n", $class['name']);
    fprintf($f, "  init_bridged_class_entry(&%s_tmp_ce, \"%s\", %s_me_table);\n", $class['name'], $class['name'], $class['name']);
    fprintf($f, "  %s_ce = zend_register_internal_class(&%s_tmp_ce);\n", $class['name'], $class['name']);
  }
  fprintf($f, "  return SUCCESS;\n");
  fprintf($f, "}\n");
  fprintf($f, "namespace HPHP {\n");
  foreach ($global_bindings as $name) {
    fprintf($f, "extern Variant gv_%s;\n", $name);
  }
  fprintf($f, "}\n");
  fprintf($f, "PHP_RINIT_FUNCTION(%s) {\n", $module_name);
  fprintf($f, "  override_zend_function_entries(%s_fe_table);\n", $module_name);
  foreach ($global_bindings as $name) {
    fprintf($f, "  bind_zend_global(HPHP::gv_%s.getVariantData(), \"%s\");\n", $name, $name);
  }
  fprintf($f, "  return SUCCESS;\n");
  fprintf($f, "}\n\n");
  fprintf($f, "PHP_RSHUTDOWN_FUNCTION(%s) {\n", $module_name);
  fprintf($f, "  clear_zval_references();\n");
  fprintf($f, "  return SUCCESS;\n");
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
