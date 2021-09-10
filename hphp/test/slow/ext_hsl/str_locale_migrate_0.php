<?hh // strict
use namespace HH\Lib\_Private\{_Locale, _Str};

<<__EntryPoint>>
function main(): void {
  _Locale\set_request_locale(_Locale\newlocale_all('fr_FR'));
  var_dump(_Str\vsprintf_l(null, '%f', vec[123]));
}
