<?php

include_once 'base.php';

pre('#include <runtime/eval/debugger/debugger_client.h>');
pre('#include <runtime/eval/debugger/debugger_proxy.h>');

///////////////////////////////////////////////////////////////////////////////

f('hphpd_install_user_command', Boolean,
  array('cmd' => String,
        'clsname' => String));

f('hphpd_get_user_commands', StringMap);

c('DebuggerProxy', null, array(),
  array(
    m(PublicMethod, '__construct', null),
    m(PublicMethod, 'isLocal', Boolean),
    m(PublicMethod, 'send', Variant,
      array('cmd' => 'DebuggerCommand')),
  ),
  Array(),
  "\n".
  " public:\n".
  "  Eval::DebuggerProxy *m_proxy;"
 );

c('DebuggerClient', null, array(),
  array(
    m(PublicMethod, '__construct', null),
    m(PublicMethod, 'quit', NULL),

    m(PublicMethod, 'print', NULL,
      array('format' => String), VariableArguments),
    m(PublicMethod, 'help', NULL,
      array('format' => String), VariableArguments),
    m(PublicMethod, 'info', NULL,
      array('format' => String), VariableArguments),
    m(PublicMethod, 'output', NULL,
      array('format' => String), VariableArguments),
    m(PublicMethod, 'error', NULL,
      array('format' => String), VariableArguments),
    m(PublicMethod, 'comment', NULL,
      array('format' => String), VariableArguments),
    m(PublicMethod, 'code', NULL,
      array('format' => String), VariableArguments),
    m(PublicMethod, 'ask', Variant,
      array('format' => String), VariableArguments),

    m(PublicMethod, 'wrap', String,
      array('str' => String)),
    m(PublicMethod, 'helpTitle', NULL,
      array('str' => String)),
    m(PublicMethod, 'helpBody', NULL,
      array('str' => String)),
    m(PublicMethod, 'helpSection', NULL,
      array('str' => String)),
    m(PublicMethod, 'tutorial', NULL,
      array('str' => String)),

    m(PublicMethod, 'getCode', String),
    m(PublicMethod, 'getCommand', String),

    m(PublicMethod, 'arg', Boolean,
      array('index' => Int32, 'str' => String)),
    m(PublicMethod, 'argCount', Int32),
    m(PublicMethod, 'argValue', String,
      array('index' => Int32)),
    m(PublicMethod, 'argRest', String,
      array('index' => Int32)),
    m(PublicMethod, 'args', StringVec),

    m(PublicMethod, 'send', Variant,
      array('cmd' => 'DebuggerCommand')),
    m(PublicMethod, 'xend', Variant,
      array('cmd' => 'DebuggerCommand')),

    m(PublicMethod, 'getCurrentLocation', Variant),
    m(PublicMethod, 'getStackTrace', Variant),
    m(PublicMethod, 'getFrame', Int32),
    m(PublicMethod, 'printFrame', NULL,
      array('index' => Int32)),

    m(PublicMethod, 'addCompletion', NULL,
      array('list' => Variant)),

    ),
  array(
    ck("AUTO_COMPLETE_FILENAMES", Int64),
    ck("AUTO_COMPLETE_VARIABLES", Int64),
    ck("AUTO_COMPLETE_CONSTANTS", Int64),
    ck("AUTO_COMPLETE_CLASSES",   Int64),
    ck("AUTO_COMPLETE_FUNCTIONS", Int64),
  ),
  "\n".
  " public:\n".
  "  Eval::DebuggerClient *m_client;"
 );
