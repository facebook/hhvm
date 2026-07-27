<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete')>>

// Test all field descriptor merge cases from HIP §4.1.3

// Req + Req = Req (rightmost type wins)
type TReqReq = shape(
  ...shape('x' => int),
  ...shape('x' => bool),
);

// Req + Opt = Req (union of types)
type TReqOpt = shape(
  ...shape('x' => int),
  ...shape(?'x' => bool),
);

// Opt + Req = Req (rightmost type wins)
type TOptReq = shape(
  ...shape(?'x' => int),
  ...shape('x' => bool),
);

// Opt + Opt = Opt (union of types)
type TOptOpt = shape(
  ...shape(?'x' => int),
  ...shape(?'x' => bool),
);

function test_req_req(TReqReq $s): void {
  hh_show($s);
  hh_expect<bool>($s['x']);
}

function test_req_opt(TReqOpt $s): void {
  hh_show($s);
  // Required because left has Req; type is (int | bool) because right is Opt
  hh_expect<(int | bool)>($s['x']);
}

function test_opt_req(TOptReq $s): void {
  hh_show($s);
  hh_expect<bool>($s['x']);
}

function test_opt_opt(TOptOpt $s): void {
  hh_show($s);
}
