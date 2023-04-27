type a = {
  x: x; [@opaque] [@visitors.opaque]
  y: y; [@visitors.opaque]
}
[@@deriving eq, show, visitors { variety = "iter" }]
