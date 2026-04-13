<?hh

namespace HH_FIXME {
  type MISSING_TYPE_IN_HIERARCHY = mixed;
}

namespace {
  function foo(HH_FIXME\MISSING_TYPE_IN_HIERARCHY $any, bool $b): void {
    hh_log_level('show', 3); // make sure "_" is Tany not Tvar
    hh_show($any);
    $nullable_any = $b ? $any : null;
    hh_show($nullable_any);
    if ($nullable_any is null) { // expect no 12009 warning
    }
  }
}
