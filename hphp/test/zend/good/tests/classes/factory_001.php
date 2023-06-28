<?hh

class Circle {
    function draw() :mixed{
        echo "Circle\n";
    }
}

class Square {
    function draw() :mixed{
        print "Square\n";
    }
}

function ShapeFactoryMethod($shape) :mixed{
    switch ($shape) {
        case "Circle":
            return new Circle();
        case "Square":
            return new Square();
    }
}
<<__EntryPoint>> function main(): void {
ShapeFactoryMethod("Circle")->draw();
ShapeFactoryMethod("Square")->draw();
}
