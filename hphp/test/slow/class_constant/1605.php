<?php

class FooConstants {
  const ZERO        = 0;
  const TWENTY_FOUR3 = FooConstants::TWENTY_FOUR2;
  const TWENTY_FOUR2 = FooConstants::TWENTY_FOUR;
  const TWENTY_FOUR = 24;
  const FORTY_EIGHT = 48;
}
class BarConstants {
  const ZERO        = FooConstants::ZERO;
  const TWENTY_FOUR = FooConstants::TWENTY_FOUR;
  const FORTY_EIGHT = FooConstants::FORTY_EIGHT;
}
class GooConstants {
  const ZERO        = BarConstants::ZERO;
  const TWENTY_FOUR = BarConstants::TWENTY_FOUR;
  const FORTY_EIGHT = BarConstants::FORTY_EIGHT;
}
function a_better_pickle() {
  return FooConstants::ZERO;
}
a_better_pickle();
print GooConstants::ZERO;
print FooConstants::TWENTY_FOUR2;
print FooConstants::TWENTY_FOUR3;
