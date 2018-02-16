<?hh // strict

function is_bool_(mixed $x): void {
  if ($x is ?bool) {
    echo "bool\n";
  }
}

function is_int_(mixed $x): void {
  if ($x is ?int) {
    echo "int\n";
  }
}

function is_float_(mixed $x): void {
  if ($x is ?float) {
    echo "float\n";
  }
}

function is_string_(mixed $x): void {
  if ($x is ?string) {
    echo "string\n";
  }
}

function is_resource_(mixed $x): void {
  if ($x is ?resource) {
    echo "resource\n";
  }
}

function is_vec_(mixed $x): void {
  if ($x is ?vec) {
    echo "vec\n";
  }
}

function is_dict_(mixed $x): void {
  if ($x is ?dict) {
    echo "dict\n";
  }
}

function is_keyset_(mixed $x): void {
  if ($x is ?keyset) {
    echo "keyset\n";
  }
}

function is_varray_(mixed $x): void {
  if ($x is ?varray) {
    echo "varray\n";
  }
}

function is_darray_(mixed $x): void {
  if ($x is ?darray) {
    echo "darray\n";
  }
}

is_bool_(null);
is_bool_(true);

echo "---\n";

is_int_(null);
is_int_(0);

echo "---\n";

is_float_(null);
is_float_(1.5);

echo "---\n";

is_string_(null);
is_string_("foo");

echo "---\n";

is_resource_(null);
is_resource_(STDIN);

echo "---\n";

is_vec_(null);
is_vec_(vec[]);

echo "---\n";

is_dict_(null);
is_dict_(dict[]);

echo "---\n";

is_keyset_(null);
is_keyset_(keyset[]);

echo "---\n";

is_varray_(null);
is_varray_(varray[]);

echo "---\n";

is_darray_(null);
is_darray_(darray[]);
