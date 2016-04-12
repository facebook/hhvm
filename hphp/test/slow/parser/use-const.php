<?php

namespace N1 {
  const C = 42;
}

namespace N2\N3 {
  const C = 43;
}

namespace N2 {
  use const \N1\C as CC;
  var_dump(CC);
  var_dump(N3\C);
}
