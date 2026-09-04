//// abstract_method_optional.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises declaration_parser.rs.
interface AbstractMethodOptional {
  public function test(optional int $x): void;
}

//// abstract_method_inout.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises declaration_parser.rs.
interface AbstractMethodInout {
  public function test(inout int $x): void;
}

//// abstract_method_named.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises declaration_parser.rs.
interface AbstractMethodNamed {
  public function test(named int $x): void;
}

//// abstract_method_readonly.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises declaration_parser.rs.
interface AbstractMethodReadonly {
  public function test(readonly int $x): void;
}

//// abstract_method_optional_named.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises declaration_parser.rs.
interface AbstractMethodOptionalNamed {
  public function test(optional named int $x): void;
}

//// abstract_method_optional_readonly.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises declaration_parser.rs.
interface AbstractMethodOptionalReadonly {
  public function test(optional readonly int $x): void;
}

//// abstract_method_named_readonly.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises declaration_parser.rs.
interface AbstractMethodNamedReadonly {
  public function test(named readonly int $x): void;
}

//// abstract_method_optional_named_readonly.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises declaration_parser.rs.
interface AbstractMethodOptionalNamedReadonly {
  public function test(optional named readonly int $x): void;
}

//// function_type_optional.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises type_parser.rs.
type FunctionTypeOptional = (function(optional int): void);

//// function_type_inout.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises type_parser.rs.
type FunctionTypeInout = (function(inout int): void);

//// function_type_named.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises type_parser.rs.
type FunctionTypeNamed = (function(named int $x): void);

//// function_type_readonly.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises type_parser.rs.
type FunctionTypeReadonly = (function(readonly int): void);

//// function_type_optional_named.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises type_parser.rs.
type FunctionTypeOptionalNamed = (function(optional named int $x): void);

//// function_type_optional_readonly.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises type_parser.rs.
type FunctionTypeOptionalReadonly = (function(optional readonly int): void);

//// function_type_inout_readonly.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises type_parser.rs.
type FunctionTypeInoutReadonly = (function(inout readonly int): void);

//// function_type_named_readonly.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises type_parser.rs.
type FunctionTypeNamedReadonly = (function(named readonly int $x): void);

//// function_type_optional_named_readonly.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises type_parser.rs.
type FunctionTypeOptionalNamedReadonly = (function(optional named readonly int $x): void);
