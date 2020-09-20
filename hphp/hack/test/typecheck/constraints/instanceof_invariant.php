<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class GeneralWidget<T> {}
class Widget<T> extends GeneralWidget<T> {}
function expect_int_widget(Widget<int> $w): void {}
function not_broken(GeneralWidget<int> $m): void {
  if ($m is Widget<_>) {
    //      hh_show_env();
    expect_int_widget($m);
  }
}
