<?hh
class MyCollection extends MongoCollection
{
    static public function toIndexString($a)
    {
        return parent::toIndexString($a);
    }
}
<<__EntryPoint>> function main(): void {
var_dump(MyCollection::toIndexString('x'));
var_dump(MyCollection::toIndexString('x.y.z'));
var_dump(MyCollection::toIndexString('x_y.z'));
var_dump(MyCollection::toIndexString(darray['x' => 1]));
var_dump(MyCollection::toIndexString(darray['x' => -1]));
var_dump(MyCollection::toIndexString(darray['x' => 1, 'y' => -1]));
}
