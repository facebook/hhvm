<?hh

class A {}

function ctx_cipp()[cipp]: void {
  ctx_cipp();
  ctx_cipp_of();
  ctx_cipp_global();
}

function ctx_cipp_of()[cipp_of<\A>]: void {
  ctx_cipp();
  ctx_cipp_of();
  ctx_cipp_global();
}

function ctx_cipp_global()[cipp_global]: void {
  ctx_cipp();
  ctx_cipp_of();
  ctx_cipp_global();
}
