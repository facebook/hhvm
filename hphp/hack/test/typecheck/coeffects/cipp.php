<?hh

class A {}

function ctx_policied()[policied]: void {
  ctx_policied_shallow(); // fail
  ctx_policied_local(); // fail
  ctx_policied_of(); // fail
  defaults(); // fail
}

function ctx_policied_shallow()[policied_shallow]: void {
  ctx_policied();
  ctx_policied_local();
  ctx_policied_of(); // fail
  defaults(); // fail
}

function ctx_policied_local()[policied_local]: void {
  ctx_policied();
  ctx_policied_shallow();
  // TODO(cipp): these three should fail
  ctx_policied_of(); // fail
  defaults();
}

function ctx_policied_of()[policied_of<A>]: void {
  ctx_policied();
  ctx_policied_shallow(); // fail
  ctx_policied_local(); // fail
  defaults(); // fail
}

function defaults(): void {
  ctx_policied();
  ctx_policied_shallow();
  ctx_policied_local();
  // TODO(cipp): these three should fail
  ctx_policied_of();
}
