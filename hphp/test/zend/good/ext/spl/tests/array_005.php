<?hh

class Student
{
    private $id;
    private $name;

    public function __construct($id, $name)
    {
        $this->id = $id;
        $this->name = $name;
    }

    public function __toString()
:mixed    {
        return $this->id . ', ' . $this->name;
    }

    public function getID()
:mixed    {
        return $this->id;
    }
}

class StudentList implements IteratorAggregate
{
    private $students;

    public function __construct()
    {
        $this->students = vec[];
    }

    public function add(Student $student)
:mixed    {
        if (!$this->contains($student)) {
            $this->students[] = $student;
        }
    }

    public function contains(Student $student)
:mixed    {
        foreach ($this->students as $s)
        {
            if ($s->getID() == $student->getID()) {
                return true;
            }
        }
        return false;
    }

    public function getIterator() :mixed{
        return new ArrayIterator($this->students);
    }
}
<<__EntryPoint>> function main(): void {
$students = new StudentList();
$students->add(new Student('01234123', 'Joe'));
$students->add(new Student('00000014', 'Bob'));
$students->add(new Student('00000014', 'Foo'));

foreach ($students as $student) {
    echo $student, "\n";
}
echo "===DONE===\n";
}
