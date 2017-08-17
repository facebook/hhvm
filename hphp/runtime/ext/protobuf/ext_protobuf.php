<?hh

//die("This is a stub file for IDEs, don't use it directly!");

abstract class ProtobufMessage
{
    const PB_TYPE_DOUBLE     = 1;
    const PB_TYPE_FIXED32    = 2;
    const PB_TYPE_FIXED64    = 3;
    const PB_TYPE_FLOAT      = 4;
    const PB_TYPE_INT        = 5;
    const PB_TYPE_SIGNED_INT = 6;
    const PB_TYPE_STRING     = 7;
    const PB_TYPE_BOOL       = 8;
    const PB_TYPE_UINT64     = 9;
    
    public $values = array();

    /**
     * @return null
     */
    abstract public function reset():void;

    /**
     * @param int   $position
     * @param mixed $value
     * 
     * @return null
     */
    <<__Native>>
    public function append(int $position, mixed $value):void;

    /**
     * @param int $position
     * 
     * @return null
     */
    <<__Native>>
    public function clear(int $position):void;

    /**
     * @param bool $onlySet
     * @param int  $indentation
     * 
     * @return null
     */
    <<__Native>>
    public function dump(bool $onlySet = true, int $indentation = 0):void;

    /**
     * @param int $position
     * 
     * @return int
     */
    <<__Native>>
    public function count(int $position):int;

    /**
     * @param int $position
     * 
     * @return mixed
     */
    <<__Native>>
    public function get(int $position, int $offset = -1):mixed;

    /**
     * @param string $packed
     * 
     * @throws Exception
     * 
     * @return bool
     */
    <<__Native>>
    public function parseFromString(string $packed):bool;

    /**
     * @throws Exception
     *
     * @return string
     */
    <<__Native>>
    public function serializeToString():string;

    /**
     * @param int   $position
     * @param mixed $value
     * 
     * @return null
     */
    <<__Native>>
    public function set(int $position, mixed $value):void;
}
