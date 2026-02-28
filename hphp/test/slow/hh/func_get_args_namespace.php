<?hh

namespace A;

function func_get_args() :mixed{
  \var_dump('func_get_args');
}

<<__EntryPoint>>
function main_func_get_args_namespace() :mixed{
\A\func_get_args();
}
