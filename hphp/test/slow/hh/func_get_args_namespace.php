<?hh

namespace A;

function func_get_args() {
  \var_dump('func_get_args');
}

<<__EntryPoint>>
function main_func_get_args_namespace() {
\A\func_get_args();
}
