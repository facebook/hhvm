<?hh

class ÜberKööliäå
{
	public $åäöÅÄÖüÜber = 'åäöÅÄÖ';
}

<<__EntryPoint>> function main(): void {
$åäöÅÄÖ = darray['åäöÅÄÖ' => 'åäöÅÄÖ'];

$foo = new Überkööliäå();

var_dump(serialize($foo));
var_dump(unserialize(serialize($foo)));
var_dump(serialize($åäöÅÄÖ));
var_dump(unserialize(serialize($åäöÅÄÖ)));
}
