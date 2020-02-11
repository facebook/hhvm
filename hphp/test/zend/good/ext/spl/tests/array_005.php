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
    {
        return $this->id . ', ' . $this->name;
    }

    public function getId()
    {
        return $this->id;
    }
}

class StudentList implements IteratorAggregate
{
    private $students;

    public function __construct()
    {
        $this->students = varray[];
    }

    public function add(Student $student)
    {
        if (!$this->contains($student)) {
            $this->students[] = $student;
        }
    }

    public function contains(Student $student)
    {
        foreach ($this->students as $s)
        {
            if ($s->getId() == $student->getId()) {
                return true;
            }
        }
        return false;
    }

    public function getIterator() {
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
