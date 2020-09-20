<?hh <<__EntryPoint>> function main(): void {
try
{
    $rdi = new recursiveDirectoryIterator(dirname(__FILE__),  FilesystemIterator::SKIP_DOTS | FilesystemIterator::UNIX_PATHS);
    $it = new recursiveIteratorIterator( $rdi );
    $it->getInnerIterator()->seek(1);
    while( $it->valid())
    {
        if( $it->getInnerIterator()->isFile() )
        {
            $it->current();
        }

        $it->next();
    }

    $it->current();
}
catch(Exception $e)
{
}
echo "okey";
}
