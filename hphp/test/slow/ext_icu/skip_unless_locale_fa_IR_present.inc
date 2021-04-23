<?hh

function skip_unless_locale_fa_IR_present(): void {
  if (!setlocale(LC_ALL, 'fa_IR')) {
    die('skip since fa_IR locale is not available');
  }
}

