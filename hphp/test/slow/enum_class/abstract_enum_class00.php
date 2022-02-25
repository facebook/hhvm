<?hh

abstract enum class E: mixed {
  abstract string Y;
}


enum class F: mixed extends E {
  string Y = 'yolo';
}

abstract enum class G: mixed extends E {
  abstract string Z;
}

enum class H: mixed extends F, G {
  string Z = 'yolo';
}

<<__EntryPoint>>
function main(): void {
  echo H::Y;
  echo "\n";
  echo H::Z;
  echo "\n";
}
