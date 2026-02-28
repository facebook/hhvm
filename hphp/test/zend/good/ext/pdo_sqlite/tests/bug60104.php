<?hh
function setUp()
:mixed{
    $handler = new PDO( "sqlite::memory:" );
    $handler->sqliteCreateFunction( "md5", "md5", 1 );
    unset( $handler );
}
<<__EntryPoint>> function main(): void {
setUp();
setUp();
echo "done";
}
