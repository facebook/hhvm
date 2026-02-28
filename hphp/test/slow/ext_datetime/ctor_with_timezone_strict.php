<?hh

// Tests that no strict warning is issued

<<__EntryPoint>>
function main_ctor_with_timezone_strict() :mixed{
new DateTime("now", new DateTimeZone('Europe/Berlin'));
}
