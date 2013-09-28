<?php

include_once __DIR__ . '/base.php';

/**
 * Possible values for 'format':
 *
 *   cpp:     generating .h and .cpp for implementing the functions.
 *   inc:     generating .inc for system/builtin_symbols.cpp
 *   test:    generating .h and .cpp for unit tests
 *   param:   synchronize parameter types and names
 *   extmap:  sep extension's prototype map
 */
$format = $argv[1];
$input = $argv[2];
$mode = '';

// format-mode: for "sep" detachable extensions
if (strpos($format, '-') > 0) {
  list($format, $mode) = explode('-', $format);
}

if (preg_match('/\.idl\.json/', $input)) {
  $full_name = preg_replace('/\.idl\.json/', '', $input);
  $name = preg_replace('%^.*/%', '', $full_name);
  if ($full_name != $name) $mode = 'remote';
} else {
  throw new Exception("wrong IDL or schema file $input");
}
ReadIDLFile($input);

$name = preg_replace('|[/\.]|', '_', $name);
$NAME = strtoupper($name);
$Name = ucfirst($name);

$PREFIX = ($mode == 'sep') ? 'SEPEXT' : 'EXT';

$format_function = 'idl_format_' . $format;
if (function_exists($format_function)) {
  call_user_func_array($format_function, array_slice($argv, 3));
}

/*****************************************************************************/
function idl_format_cpp($header, $impl) {
  idl_format_cpp_header($header);
  idl_format_cpp_impl($impl);
}

/*****************************************************************************/
function idl_format_cpp_header($header) {
  global $preamble, $funcs, $constants, $classes, $PREFIX, $NAME;
  ($f = fopen($header, 'w')) || die("cannot open $header");

  fprintf($f,
          <<<EOT

#ifndef incl_${PREFIX}_${NAME}_H_
#define incl_${PREFIX}_${NAME}_H_

#include "hphp/runtime/base/base-includes.h"

EOT
          );

  if (isset($preamble)) {
    fprintf($f, $preamble);
  }

  fprintf($f,
          <<<EOT

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////


EOT
          );

  foreach ($funcs as $func) {
    generateFuncCPPHeader($func, $f);
  }
  foreach ($constants as $const) {
    generateConstCPPHeader($const, $f);
  }
  foreach ($classes as $class) {
    generateClassCPPHeader($class, $f);
  }
  fprintf($f,
          <<<EOT

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_${PREFIX}_${NAME}_H_

EOT
          );
  fclose($f);
}

/*****************************************************************************/
function idl_format_cpp_impl($impl) {
  global $funcs, $name, $mode, $constants, $classes;
  ($f = fopen($impl, 'w')) || die("cannot open $impl");

  if ($mode) {
    $header_file = "\"ext_${name}.h\"";
  } else {
    $header_file = "<runtime/ext/ext_${name}.h>";
  }

  fprintf($f,
          <<<EOT

#include $header_file

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////


EOT
          );

  foreach ($constants as $const) {
    generateConstCPPImplementation($const, $f);
  }
  foreach ($funcs as $func) {
    generateFuncCPPImplementation($func, $f);
  }
  foreach ($classes as $class) {
    generateClassCPPImplementation($class, $f);
  }
  fprintf($f,
          <<<EOT

///////////////////////////////////////////////////////////////////////////////
}

EOT
          );
}

/*****************************************************************************/
function idl_format_test($header, $impl) {
  idl_format_test_header($header);
  idl_format_test_impl($header,$impl);
}

/*****************************************************************************/
function idl_format_test_header($test_header) {
  global $funcs, $classes, $PREFIX, $NAME, $Name;
  $ext_header = substr($test_header, 5);
  ($f = fopen($test_header, 'w')) || die("cannot open $test_header");

  fprintf($f,
          <<<EOT

#ifndef incl_TEST_${PREFIX}_${NAME}_H_
#define incl_TEST_${PREFIX}_${NAME}_H_

#include "hphp/test/test_cpp_ext.h"

///////////////////////////////////////////////////////////////////////////////

class TestExt${Name} : public TestCppExt {
 public:
  virtual bool RunTests(const std::string &which);


EOT
          );
  foreach ($funcs as $func) {
    fprintf($f, "  bool test_".$func['name']."();\n");
  }
  foreach ($classes as $class) {
    fprintf($f, "  bool test_".$class['name']."();\n");
  }
  fprintf($f,
          <<<EOT
};

///////////////////////////////////////////////////////////////////////////////

#endif // incl_TEST_${PREFIX}_${NAME}_H_

EOT
          );
  fclose($f);
}

/*****************************************************************************/
function idl_format_test_impl($test_header, $test_impl) {
  global $funcs, $classes, $Name, $name, $mode;

  ($f = fopen($test_impl, 'w')) || die("cannot open $test_impl");
  if ($mode == 'sep' || $mode == 'remote') {
    $inc_file1 = "\"$test_header\"";
    $inc_file2 = "\"ext_${name}.h\"";
  } else {
    $inc_file1 = "<test/$test_header>";
    $inc_file2 = "<runtime/ext/ext_${name}.h>";
  }
  fprintf($f,
          <<<EOT

#include $inc_file1
#include $inc_file2

IMPLEMENT_SEP_EXTENSION_TEST($Name);
///////////////////////////////////////////////////////////////////////////////

bool TestExt${Name}::RunTests(const std::string &which) {
  bool ret = true;


EOT
  );

  foreach ($funcs as $func) {
    fprintf($f, "  RUN_TEST(test_".$func['name'].");\n");
  }
  foreach ($classes as $class) {
    fprintf($f, "  RUN_TEST(test_".$class['name'].");\n");
  }
  fprintf($f, <<<EOT

  return ret;
}

///////////////////////////////////////////////////////////////////////////////

EOT
          );

  foreach ($funcs + $classes as $item) {
    $item_name = $item['name'];
    fprintf($f, <<<EOT

bool TestExt${Name}::test_$item_name() {
  return Count(true);
}

EOT
          );
  }

  fclose($f);
}

/*****************************************************************************/
function idl_format_param($header, $impl) {
  replaceParams($header, true);
  replaceParams($impl,   false);
}

/*****************************************************************************/
function idl_format_extmap($header, $impl) {
  idl_format_extmap_header($header);
  idl_format_extmap_impl($impl);
}

/*****************************************************************************/
function idl_format_extmap_header($map_header) {
  global $name;

  ($f = fopen($map_header, 'w')) || die("cannot open $map_header");
  fprintf($f,
          <<<EOT

#include <util/base.h>
///////////////////////////////////////////////////////////////////////////////

extern "C" {
  extern const char **${name}_map[];
}

EOT
          );
  fclose($f);
}

/*****************************************************************************/
function idl_format_extmap_impl($map_impl) {
  global $name;
  ($f = fopen($map_impl, 'w')) || die("cannot open $map_impl");
  fprintf($f,
          <<<EOT

#include "extmap_${name}.h"
#include <compiler/analysis/type.h>

///////////////////////////////////////////////////////////////////////////////

static const char *${name}_extension_functions[] = {
#define S(n) (const char *)n
#define T(t) (const char *)HPHP::Type::KindOf ## t
#define EXT_TYPE 0
#include "${name}.inc"
  NULL,
};
#undef EXT_TYPE

static const char *${name}_extension_constants[] = {
#define EXT_TYPE 1
#include "${name}.inc"
  NULL,
};
#undef EXT_TYPE

static const char *${name}_extension_classes[] = {
#define EXT_TYPE 2
#include "${name}.inc"
  NULL,
};
#undef EXT_TYPE

///////////////////////////////////////////////////////////////////////////////

const char **${name}_map[] = {
  ${name}_extension_functions,
  ${name}_extension_constants,
  ${name}_extension_classes,
};

EOT
          );
  fclose($f);
}
