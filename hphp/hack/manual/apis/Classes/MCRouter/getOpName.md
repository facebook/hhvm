
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Translate an mc_op_* numeric code to something human-readable







``` Hack
public static function getOpName(
  int $op,
): string;
```




## Parameters




+ ` int $op `




## Returns




* ` string ` - - The name of the op




## Examples




The following example shows how to use [` MCRouter::getOpName `](/apis/Classes/MCRouter/getOpName/) to get the English readable name for an MCRouter operation given as an integer.




Here is the list of the current mappings:




| Integer | Constant | Returned String |
| - | - | - |
| 0 | MCRouter::mc_op_unknown | unknown |
| 1 | MCRouter::mc_op_echo | echo |
| 2 | MCRouter::mc_op_quit | quit |
| 3 | MCRouter::mc_op_version | version |
| 4 | MCRouter::mc_op_servererr | servererr |
| 5 | MCRouter::mc_op_get | get |
| 6 | MCRouter::mc_op_set | set |
| 7 | MCRouter::mc_op_add | add |
| 8 | MCRouter::mc_op_replace | replace |
| 9 | MCRouter::mc_op_append | append |
| 10 | MCRouter::mc_op_prepend | prepend |
| 11 | MCRouter::mc_op_cas | cas |
| 12 | MCRouter::mc_op_delete | delete |
| 13 | MCRouter::mc_op_nops | nops |
| 14 | MCRouter::mc_op_incr | incr |
| 15 | MCRouter::mc_op_decr | decr |
| 16 | MCRouter::mc_op_flushall | flushall |
| 17 | MCRouter::mc_op_flushre | flushre |
| 18 | MCRouter::mc_op_stats | stats |
| 19 | MCRouter::mc_op_verbosity | verbosity |
| 20 | MCRouter::mc_op_lease_get | lease-get |
| 21 | MCRouter::mc_op_lease_set | lease-set |
| 22 | MCRouter::mc_op_shutdown | shutdown |
| 23 | MCRouter::mc_op_end | end |
| 24 | MCRouter::mc_op_metaget | metaget |
| 25 | MCRouter::mc_op_exec | exec |
| 26 | MCRouter::mc_op_gets | gets |
| 27 | MCRouter::mc_op_get_service_info | get-service-info |




~~~ basic-usage.hack
function get_simple_mcrouter(): \MCRouter {
  $servers = Vector {\getenv('HHVM_TEST_MCROUTER')};
  $mc = \MCRouter::createSimple($servers);
  return $mc;
}

function get_op_name(int $op_num): string {
  return \MCRouter::getOpName($op_num);
}

<<__EntryPoint>>
async function run(): Awaitable<void> {
  $mc = get_simple_mcrouter();

  // You can pass raw integers
  \var_dump(get_op_name(3));
  \var_dump(get_op_name(9));
  \var_dump(get_op_name(-1));
  \var_dump(get_op_name(0));
  \var_dump(get_op_name(100));

  // You can pass MCRouter constants
  \var_dump(get_op_name(\MCRouter::mc_op_servererr));
  \var_dump(get_op_name(\MCRouter::mc_op_exec));
  \var_dump(get_op_name(\MCRouter::mc_op_unknown));

  // You can pass something from an exception too
  try {
    $val = await $mc->get('KEYDOESNOTEXISTIHOPEREALLY');
  } catch (\MCRouterException $ex) {
    \var_dump($ex->getOp());
    \var_dump(get_op_name($ex->getOp()));
  }
}
```.hhvm.expectf
string(7) "version"
string(6) "append"
string(7) "unknown"
string(7) "unknown"
string(7) "unknown"
string(9) "servererr"
string(4) "exec"
string(7) "unknown"
int(5)
string(3) "get"
```.example.hhvm.out
string(7) "version"
string(6) "append"
string(7) "unknown"
string(7) "unknown"
string(7) "unknown"
string(9) "servererr"
string(4) "exec"
string(7) "unknown"
int(5)
string(3) "get"
```.skipif
\Hack\UserDocumentation\API\Examples\MCRouter\skipif();
~~~
<!-- HHAPIDOC -->
