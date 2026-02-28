<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class BigNumbersRUs {
  const float fp = 1_2_3.4_5_6;
  const float fpexp = 1_2_3e-4_5_6;
  const float fpdot = .1_2_3e-4_5_6;
  const float fpleading = 0.1_2_3_4e+2_5;
  const float fpleadingexp = 0e-1_5;
  const float fpoct = 03_41.51_36;

  const int dec = 1_234_567_890;
  const int hex = 0xff_cc_123_4;
  const int bin = 0b11_00_11_00;
  const int oct = 0666_555_444;
  const int neg_dec = -1_234_567_890;
  const int neg_hex = -0xff_cc_123_4;
  const int neg_bin = -0b11_00_11_00;
  const int neg_oct = -0666_555_444;
}
