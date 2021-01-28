<?hh

class A {}

function ctx_policied()[policied]: void {
  ctx_policied_shallow(); // fail
  ctx_policied_local(); // fail
  ctx_policied_of(); // fail
  ctx_policied_of_shallow(); // fail
  ctx_policied_of_local(); // fail
  defaults(); // fail
}

function ctx_policied_shallow()[policied_shallow]: void {
  ctx_policied();
  ctx_policied_local();
  ctx_policied_of(); // fail
  ctx_policied_of_shallow(); // fail
  ctx_policied_of_local(); // fail
  defaults(); // fail
}

function ctx_policied_local()[policied_local]: void {
  ctx_policied();
  ctx_policied_shallow();
  // TODO(cipp): these three should fail
  ctx_policied_of(); // fail
  ctx_policied_of_shallow(); // fail
  ctx_policied_of_local(); // fail
  defaults();
}

function ctx_policied_of()[policied_of<A>]: void {
  ctx_policied();
  ctx_policied_shallow(); // fail
  ctx_policied_local(); // fail
  ctx_policied_of_shallow(); // fail
  ctx_policied_of_local(); // fail
  defaults(); // fail
}

function ctx_policied_of_shallow()[policied_of_shallow<A>]: void {
  ctx_policied();
  ctx_policied_shallow();
  ctx_policied_local();
  ctx_policied_of();
  ctx_policied_of_local();
  defaults(); // fail
}

function ctx_policied_of_local()[policied_of_local<A>]: void {
  ctx_policied();
  ctx_policied_shallow();
  ctx_policied_local();
  ctx_policied_of();
  ctx_policied_of_shallow();
  defaults();
}

function defaults(): void {
  ctx_policied();
  ctx_policied_shallow();
  ctx_policied_local();
  // TODO(cipp): these three should fail
  ctx_policied_of();
  ctx_policied_of_shallow();
  ctx_policied_of_local();
}
