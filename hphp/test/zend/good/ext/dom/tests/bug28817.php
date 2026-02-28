<?hh

class z extends DOMDocument{
    /** variable can have name */
    public $p_array;
    public $p_variable;

    function __construct(){
        $this->p_array = vec[];
        $this->p_array[] = 'bonus';
        $this->p_array[] = 'vir';
        $this->p_array[] = 'semper';
        $this->p_array[] = 'tiro';

        $this->p_variable = 'Cessante causa cessat effectus';
    }
}
<<__EntryPoint>> function main(): void {
$z=new z();
var_dump($z->p_array);
var_dump($z->p_variable);
}
