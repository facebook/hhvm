//// abstract_method_optional_inout.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises declaration_parser.rs.
interface AbstractMethodOptionalInout {
  public function test(optional inout int $x): void;
}

//// abstract_method_inout_optional.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises declaration_parser.rs.
interface AbstractMethodInoutOptional {
  public function test(inout optional int $x): void;
}

//// abstract_method_inout_named.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises declaration_parser.rs.
interface AbstractMethodInoutNamed {
  public function test(inout named int $x): void;
}

//// abstract_method_inout_readonly.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises declaration_parser.rs.
interface AbstractMethodInoutReadonly {
  public function test(inout readonly int $x): void;
}

//// abstract_method_named_optional.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises declaration_parser.rs.
interface AbstractMethodNamedOptional {
  public function test(named optional int $x): void;
}

//// abstract_method_named_inout.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises declaration_parser.rs.
interface AbstractMethodNamedInout {
  public function test(named inout int $x): void;
}

//// abstract_method_readonly_optional.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises declaration_parser.rs.
interface AbstractMethodReadonlyOptional {
  public function test(readonly optional int $x): void;
}

//// abstract_method_readonly_inout.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises declaration_parser.rs.
interface AbstractMethodReadonlyInout {
  public function test(readonly inout int $x): void;
}

//// abstract_method_readonly_named.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises declaration_parser.rs.
interface AbstractMethodReadonlyNamed {
  public function test(readonly named int $x): void;
}

//// abstract_method_optional_inout_named.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises declaration_parser.rs.
interface AbstractMethodOptionalInoutNamed {
  public function test(optional inout named int $x): void;
}

//// abstract_method_optional_inout_readonly.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises declaration_parser.rs.
interface AbstractMethodOptionalInoutReadonly {
  public function test(optional inout readonly int $x): void;
}

//// abstract_method_optional_named_inout.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises declaration_parser.rs.
interface AbstractMethodOptionalNamedInout {
  public function test(optional named inout int $x): void;
}

//// abstract_method_optional_readonly_inout.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises declaration_parser.rs.
interface AbstractMethodOptionalReadonlyInout {
  public function test(optional readonly inout int $x): void;
}

//// abstract_method_optional_readonly_named.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises declaration_parser.rs.
interface AbstractMethodOptionalReadonlyNamed {
  public function test(optional readonly named int $x): void;
}

//// abstract_method_inout_optional_named.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises declaration_parser.rs.
interface AbstractMethodInoutOptionalNamed {
  public function test(inout optional named int $x): void;
}

//// abstract_method_inout_optional_readonly.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises declaration_parser.rs.
interface AbstractMethodInoutOptionalReadonly {
  public function test(inout optional readonly int $x): void;
}

//// abstract_method_inout_named_optional.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises declaration_parser.rs.
interface AbstractMethodInoutNamedOptional {
  public function test(inout named optional int $x): void;
}

//// abstract_method_inout_named_readonly.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises declaration_parser.rs.
interface AbstractMethodInoutNamedReadonly {
  public function test(inout named readonly int $x): void;
}

//// abstract_method_inout_readonly_optional.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises declaration_parser.rs.
interface AbstractMethodInoutReadonlyOptional {
  public function test(inout readonly optional int $x): void;
}

//// abstract_method_inout_readonly_named.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises declaration_parser.rs.
interface AbstractMethodInoutReadonlyNamed {
  public function test(inout readonly named int $x): void;
}

//// abstract_method_named_optional_inout.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises declaration_parser.rs.
interface AbstractMethodNamedOptionalInout {
  public function test(named optional inout int $x): void;
}

//// abstract_method_named_optional_readonly.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises declaration_parser.rs.
interface AbstractMethodNamedOptionalReadonly {
  public function test(named optional readonly int $x): void;
}

//// abstract_method_named_inout_optional.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises declaration_parser.rs.
interface AbstractMethodNamedInoutOptional {
  public function test(named inout optional int $x): void;
}

//// abstract_method_named_inout_readonly.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises declaration_parser.rs.
interface AbstractMethodNamedInoutReadonly {
  public function test(named inout readonly int $x): void;
}

//// abstract_method_named_readonly_optional.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises declaration_parser.rs.
interface AbstractMethodNamedReadonlyOptional {
  public function test(named readonly optional int $x): void;
}

//// abstract_method_named_readonly_inout.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises declaration_parser.rs.
interface AbstractMethodNamedReadonlyInout {
  public function test(named readonly inout int $x): void;
}

//// abstract_method_readonly_optional_inout.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises declaration_parser.rs.
interface AbstractMethodReadonlyOptionalInout {
  public function test(readonly optional inout int $x): void;
}

//// abstract_method_readonly_optional_named.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises declaration_parser.rs.
interface AbstractMethodReadonlyOptionalNamed {
  public function test(readonly optional named int $x): void;
}

//// abstract_method_readonly_inout_optional.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises declaration_parser.rs.
interface AbstractMethodReadonlyInoutOptional {
  public function test(readonly inout optional int $x): void;
}

//// abstract_method_readonly_inout_named.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises declaration_parser.rs.
interface AbstractMethodReadonlyInoutNamed {
  public function test(readonly inout named int $x): void;
}

//// abstract_method_readonly_named_optional.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises declaration_parser.rs.
interface AbstractMethodReadonlyNamedOptional {
  public function test(readonly named optional int $x): void;
}

//// abstract_method_readonly_named_inout.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises declaration_parser.rs.
interface AbstractMethodReadonlyNamedInout {
  public function test(readonly named inout int $x): void;
}

//// abstract_method_optional_inout_named_readonly.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises declaration_parser.rs.
interface AbstractMethodOptionalInoutNamedReadonly {
  public function test(optional inout named readonly int $x): void;
}

//// abstract_method_optional_inout_readonly_named.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises declaration_parser.rs.
interface AbstractMethodOptionalInoutReadonlyNamed {
  public function test(optional inout readonly named int $x): void;
}

//// abstract_method_optional_named_inout_readonly.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises declaration_parser.rs.
interface AbstractMethodOptionalNamedInoutReadonly {
  public function test(optional named inout readonly int $x): void;
}

//// abstract_method_optional_named_readonly_inout.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises declaration_parser.rs.
interface AbstractMethodOptionalNamedReadonlyInout {
  public function test(optional named readonly inout int $x): void;
}

//// abstract_method_optional_readonly_inout_named.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises declaration_parser.rs.
interface AbstractMethodOptionalReadonlyInoutNamed {
  public function test(optional readonly inout named int $x): void;
}

//// abstract_method_optional_readonly_named_inout.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises declaration_parser.rs.
interface AbstractMethodOptionalReadonlyNamedInout {
  public function test(optional readonly named inout int $x): void;
}

//// abstract_method_inout_optional_named_readonly.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises declaration_parser.rs.
interface AbstractMethodInoutOptionalNamedReadonly {
  public function test(inout optional named readonly int $x): void;
}

//// abstract_method_inout_optional_readonly_named.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises declaration_parser.rs.
interface AbstractMethodInoutOptionalReadonlyNamed {
  public function test(inout optional readonly named int $x): void;
}

//// abstract_method_inout_named_optional_readonly.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises declaration_parser.rs.
interface AbstractMethodInoutNamedOptionalReadonly {
  public function test(inout named optional readonly int $x): void;
}

//// abstract_method_inout_named_readonly_optional.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises declaration_parser.rs.
interface AbstractMethodInoutNamedReadonlyOptional {
  public function test(inout named readonly optional int $x): void;
}

//// abstract_method_inout_readonly_optional_named.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises declaration_parser.rs.
interface AbstractMethodInoutReadonlyOptionalNamed {
  public function test(inout readonly optional named int $x): void;
}

//// abstract_method_inout_readonly_named_optional.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises declaration_parser.rs.
interface AbstractMethodInoutReadonlyNamedOptional {
  public function test(inout readonly named optional int $x): void;
}

//// abstract_method_named_optional_inout_readonly.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises declaration_parser.rs.
interface AbstractMethodNamedOptionalInoutReadonly {
  public function test(named optional inout readonly int $x): void;
}

//// abstract_method_named_optional_readonly_inout.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises declaration_parser.rs.
interface AbstractMethodNamedOptionalReadonlyInout {
  public function test(named optional readonly inout int $x): void;
}

//// abstract_method_named_inout_optional_readonly.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises declaration_parser.rs.
interface AbstractMethodNamedInoutOptionalReadonly {
  public function test(named inout optional readonly int $x): void;
}

//// abstract_method_named_inout_readonly_optional.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises declaration_parser.rs.
interface AbstractMethodNamedInoutReadonlyOptional {
  public function test(named inout readonly optional int $x): void;
}

//// abstract_method_named_readonly_optional_inout.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises declaration_parser.rs.
interface AbstractMethodNamedReadonlyOptionalInout {
  public function test(named readonly optional inout int $x): void;
}

//// abstract_method_named_readonly_inout_optional.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises declaration_parser.rs.
interface AbstractMethodNamedReadonlyInoutOptional {
  public function test(named readonly inout optional int $x): void;
}

//// abstract_method_readonly_optional_inout_named.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises declaration_parser.rs.
interface AbstractMethodReadonlyOptionalInoutNamed {
  public function test(readonly optional inout named int $x): void;
}

//// abstract_method_readonly_optional_named_inout.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises declaration_parser.rs.
interface AbstractMethodReadonlyOptionalNamedInout {
  public function test(readonly optional named inout int $x): void;
}

//// abstract_method_readonly_inout_optional_named.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises declaration_parser.rs.
interface AbstractMethodReadonlyInoutOptionalNamed {
  public function test(readonly inout optional named int $x): void;
}

//// abstract_method_readonly_inout_named_optional.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises declaration_parser.rs.
interface AbstractMethodReadonlyInoutNamedOptional {
  public function test(readonly inout named optional int $x): void;
}

//// abstract_method_readonly_named_optional_inout.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises declaration_parser.rs.
interface AbstractMethodReadonlyNamedOptionalInout {
  public function test(readonly named optional inout int $x): void;
}

//// abstract_method_readonly_named_inout_optional.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises declaration_parser.rs.
interface AbstractMethodReadonlyNamedInoutOptional {
  public function test(readonly named inout optional int $x): void;
}

//// abstract_method_optional_optional.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises declaration_parser.rs.
interface AbstractMethodOptionalOptional {
  public function test(optional optional int $x): void;
}

//// abstract_method_inout_inout.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises declaration_parser.rs.
interface AbstractMethodInoutInout {
  public function test(inout inout int $x): void;
}

//// abstract_method_named_named.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises declaration_parser.rs.
interface AbstractMethodNamedNamed {
  public function test(named named int $x): void;
}

//// abstract_method_readonly_readonly.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises declaration_parser.rs.
interface AbstractMethodReadonlyReadonly {
  public function test(readonly readonly int $x): void;
}

//// function_type_optional_inout.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises type_parser.rs.
type FunctionTypeOptionalInout = (function(optional inout int): void);

//// function_type_inout_optional.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises type_parser.rs.
type FunctionTypeInoutOptional = (function(inout optional int): void);

//// function_type_inout_named.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises type_parser.rs.
type FunctionTypeInoutNamed = (function(inout named int $x): void);

//// function_type_named_optional.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises type_parser.rs.
type FunctionTypeNamedOptional = (function(named optional int $x): void);

//// function_type_named_inout.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises type_parser.rs.
type FunctionTypeNamedInout = (function(named inout int $x): void);

//// function_type_readonly_optional.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises type_parser.rs.
type FunctionTypeReadonlyOptional = (function(readonly optional int): void);

//// function_type_readonly_inout.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises type_parser.rs.
type FunctionTypeReadonlyInout = (function(readonly inout int): void);

//// function_type_readonly_named.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises type_parser.rs.
type FunctionTypeReadonlyNamed = (function(readonly named int $x): void);

//// function_type_optional_inout_named.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises type_parser.rs.
type FunctionTypeOptionalInoutNamed = (function(optional inout named int $x): void);

//// function_type_optional_inout_readonly.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises type_parser.rs.
type FunctionTypeOptionalInoutReadonly = (function(optional inout readonly int): void);

//// function_type_optional_named_inout.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises type_parser.rs.
type FunctionTypeOptionalNamedInout = (function(optional named inout int $x): void);

//// function_type_optional_readonly_inout.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises type_parser.rs.
type FunctionTypeOptionalReadonlyInout = (function(optional readonly inout int): void);

//// function_type_optional_readonly_named.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises type_parser.rs.
type FunctionTypeOptionalReadonlyNamed = (function(optional readonly named int $x): void);

//// function_type_inout_optional_named.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises type_parser.rs.
type FunctionTypeInoutOptionalNamed = (function(inout optional named int $x): void);

//// function_type_inout_optional_readonly.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises type_parser.rs.
type FunctionTypeInoutOptionalReadonly = (function(inout optional readonly int): void);

//// function_type_inout_named_optional.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises type_parser.rs.
type FunctionTypeInoutNamedOptional = (function(inout named optional int $x): void);

//// function_type_inout_named_readonly.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises type_parser.rs.
type FunctionTypeInoutNamedReadonly = (function(inout named readonly int $x): void);

//// function_type_inout_readonly_optional.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises type_parser.rs.
type FunctionTypeInoutReadonlyOptional = (function(inout readonly optional int): void);

//// function_type_inout_readonly_named.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises type_parser.rs.
type FunctionTypeInoutReadonlyNamed = (function(inout readonly named int $x): void);

//// function_type_named_optional_inout.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises type_parser.rs.
type FunctionTypeNamedOptionalInout = (function(named optional inout int $x): void);

//// function_type_named_optional_readonly.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises type_parser.rs.
type FunctionTypeNamedOptionalReadonly = (function(named optional readonly int $x): void);

//// function_type_named_inout_optional.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises type_parser.rs.
type FunctionTypeNamedInoutOptional = (function(named inout optional int $x): void);

//// function_type_named_inout_readonly.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises type_parser.rs.
type FunctionTypeNamedInoutReadonly = (function(named inout readonly int $x): void);

//// function_type_named_readonly_optional.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises type_parser.rs.
type FunctionTypeNamedReadonlyOptional = (function(named readonly optional int $x): void);

//// function_type_named_readonly_inout.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises type_parser.rs.
type FunctionTypeNamedReadonlyInout = (function(named readonly inout int $x): void);

//// function_type_readonly_optional_inout.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises type_parser.rs.
type FunctionTypeReadonlyOptionalInout = (function(readonly optional inout int): void);

//// function_type_readonly_optional_named.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises type_parser.rs.
type FunctionTypeReadonlyOptionalNamed = (function(readonly optional named int $x): void);

//// function_type_readonly_inout_optional.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises type_parser.rs.
type FunctionTypeReadonlyInoutOptional = (function(readonly inout optional int): void);

//// function_type_readonly_inout_named.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises type_parser.rs.
type FunctionTypeReadonlyInoutNamed = (function(readonly inout named int $x): void);

//// function_type_readonly_named_optional.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises type_parser.rs.
type FunctionTypeReadonlyNamedOptional = (function(readonly named optional int $x): void);

//// function_type_readonly_named_inout.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises type_parser.rs.
type FunctionTypeReadonlyNamedInout = (function(readonly named inout int $x): void);

//// function_type_optional_inout_named_readonly.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises type_parser.rs.
type FunctionTypeOptionalInoutNamedReadonly = (function(optional inout named readonly int $x): void);

//// function_type_optional_inout_readonly_named.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises type_parser.rs.
type FunctionTypeOptionalInoutReadonlyNamed = (function(optional inout readonly named int $x): void);

//// function_type_optional_named_inout_readonly.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises type_parser.rs.
type FunctionTypeOptionalNamedInoutReadonly = (function(optional named inout readonly int $x): void);

//// function_type_optional_named_readonly_inout.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises type_parser.rs.
type FunctionTypeOptionalNamedReadonlyInout = (function(optional named readonly inout int $x): void);

//// function_type_optional_readonly_inout_named.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises type_parser.rs.
type FunctionTypeOptionalReadonlyInoutNamed = (function(optional readonly inout named int $x): void);

//// function_type_optional_readonly_named_inout.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises type_parser.rs.
type FunctionTypeOptionalReadonlyNamedInout = (function(optional readonly named inout int $x): void);

//// function_type_inout_optional_named_readonly.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises type_parser.rs.
type FunctionTypeInoutOptionalNamedReadonly = (function(inout optional named readonly int $x): void);

//// function_type_inout_optional_readonly_named.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises type_parser.rs.
type FunctionTypeInoutOptionalReadonlyNamed = (function(inout optional readonly named int $x): void);

//// function_type_inout_named_optional_readonly.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises type_parser.rs.
type FunctionTypeInoutNamedOptionalReadonly = (function(inout named optional readonly int $x): void);

//// function_type_inout_named_readonly_optional.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises type_parser.rs.
type FunctionTypeInoutNamedReadonlyOptional = (function(inout named readonly optional int $x): void);

//// function_type_inout_readonly_optional_named.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises type_parser.rs.
type FunctionTypeInoutReadonlyOptionalNamed = (function(inout readonly optional named int $x): void);

//// function_type_inout_readonly_named_optional.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises type_parser.rs.
type FunctionTypeInoutReadonlyNamedOptional = (function(inout readonly named optional int $x): void);

//// function_type_named_optional_inout_readonly.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises type_parser.rs.
type FunctionTypeNamedOptionalInoutReadonly = (function(named optional inout readonly int $x): void);

//// function_type_named_optional_readonly_inout.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises type_parser.rs.
type FunctionTypeNamedOptionalReadonlyInout = (function(named optional readonly inout int $x): void);

//// function_type_named_inout_optional_readonly.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises type_parser.rs.
type FunctionTypeNamedInoutOptionalReadonly = (function(named inout optional readonly int $x): void);

//// function_type_named_inout_readonly_optional.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises type_parser.rs.
type FunctionTypeNamedInoutReadonlyOptional = (function(named inout readonly optional int $x): void);

//// function_type_named_readonly_optional_inout.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises type_parser.rs.
type FunctionTypeNamedReadonlyOptionalInout = (function(named readonly optional inout int $x): void);

//// function_type_named_readonly_inout_optional.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises type_parser.rs.
type FunctionTypeNamedReadonlyInoutOptional = (function(named readonly inout optional int $x): void);

//// function_type_readonly_optional_inout_named.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises type_parser.rs.
type FunctionTypeReadonlyOptionalInoutNamed = (function(readonly optional inout named int $x): void);

//// function_type_readonly_optional_named_inout.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises type_parser.rs.
type FunctionTypeReadonlyOptionalNamedInout = (function(readonly optional named inout int $x): void);

//// function_type_readonly_inout_optional_named.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises type_parser.rs.
type FunctionTypeReadonlyInoutOptionalNamed = (function(readonly inout optional named int $x): void);

//// function_type_readonly_inout_named_optional.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises type_parser.rs.
type FunctionTypeReadonlyInoutNamedOptional = (function(readonly inout named optional int $x): void);

//// function_type_readonly_named_optional_inout.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises type_parser.rs.
type FunctionTypeReadonlyNamedOptionalInout = (function(readonly named optional inout int $x): void);

//// function_type_readonly_named_inout_optional.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises type_parser.rs.
type FunctionTypeReadonlyNamedInoutOptional = (function(readonly named inout optional int $x): void);

//// function_type_optional_optional.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises type_parser.rs.
type FunctionTypeOptionalOptional = (function(optional optional int): void);

//// function_type_inout_inout.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises type_parser.rs.
type FunctionTypeInoutInout = (function(inout inout int): void);

//// function_type_named_named.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises type_parser.rs.
type FunctionTypeNamedNamed = (function(named named int $x): void);

//// function_type_readonly_readonly.php
<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// Exercises type_parser.rs.
type FunctionTypeReadonlyReadonly = (function(readonly readonly int): void);
