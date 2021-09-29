<?hh

function expr(supportdynamic $sd): void {
  $sd[0];
  $sd[] = 0;
  $sd::f();
  $sd->m();
  $sd?->m();
  $sd->p;
  $sd?->p;
  'str'.$sd;
  $sd + 1;
}

async function stmt(supportdynamic $sd): Awaitable<void> {
  foreach ($sd as $_) {}
  foreach ($sd as $_k => $_v) {}
  await $sd;
}

type N = supportdynamic;

function access_typeconst(N::C $_): void {}
function cname(classname<supportdynamic> $_): void {}
enum E: supportdynamic {}
function targs(supportdynamic<int> $_): void {}
