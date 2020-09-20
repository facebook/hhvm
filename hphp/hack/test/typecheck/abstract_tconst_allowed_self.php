<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

interface ISiteVariable {
  abstract const type TVal;
}

trait SiteVariable implements ISiteVariable {
  private static ?self::TVal $data = null;
}
