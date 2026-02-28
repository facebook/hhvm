<?hh

function expr(supportdyn<nonnull> $sd): void {
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

async function stmt(supportdyn<nonnull> $sd): Awaitable<void> {
  foreach ($sd as $_) {}
  foreach ($sd as $_k => $_v) {}
  await $sd;
}

type N = supportdyn<nonnull>;

function access_typeconst(N::C $_): void {}
function cname(classname<supportdyn<nonnull>> $_): void {}
enum E: supportdyn<nonnull> {}
function targs(supportdyn<nonnull, int> $_): void {}
