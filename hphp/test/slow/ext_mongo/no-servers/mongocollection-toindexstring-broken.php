<?hh
class MyCollection extends MongoCollection
{
    static public function toIndexString($a)
    {
        return parent::toIndexString($a);
    }
}
<<__EntryPoint>> function main(): void {
var_dump(MyCollection::toIndexString(null));
}
