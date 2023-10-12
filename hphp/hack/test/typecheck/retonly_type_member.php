<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class C {
  /* HH_FIXME[2063]: this is return-only too */
  public void $void_member;
  /* HH_FIXME[4055]: cascading errors */
  public static noreturn $noreturn_member;
}
