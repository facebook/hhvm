<?hh

function nullsafe_on_null(null $null, ?HH\Ref<int> $nullable): void {
  $_ = $null?->value; // expect lint
  $_ = $nullable?->value; // expect OK
}
