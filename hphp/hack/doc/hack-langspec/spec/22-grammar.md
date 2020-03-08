<style type = "text/css">
  header {
    border-color: red;
    border-width: .25em;
    border-style: solid;
    padding: .25em;
  }
</style>
<header>
NOTICE: This spec is currently very out of date and does not reflect the current version of Hack.
</header>

# Grammar

## General

The grammar notation is described in [§§](09-lexical-structure.md#grammars).

## Lexical Grammar

### General

<pre>
  <i>input-file::
    <i>input-element
    <i>input-file   input-element
  <i>input-element::</i>
    <i>comment</i>
    <i>white-space</i>
    <i>token</i>
</pre>

### Comments

<pre>
  <i>comment::</i>
    <i>single-line-comment</i>
    <i>delimited-comment</i>

  <i>single-line-comment::</i>
    //  <i>input-characters<sub>opt</sub></i>
    #  <i>input-characters<sub>opt</sub></i>

  <i>input-characters::</i>
    <i>input-character</i>
    <i>input-characters   input-character</i>

  <i>input-character::</i>
    Any source character except new-line

  <i>new-line::</i>
    Carriage-return character (U+000D)
    Line-feed character (U+000A)
    Carriage-return character (U+000D) followed by line-feed character (U+000A)

  <i>delimited-comment::</i>
    /*   No characters or any source character sequence except /*   */
</pre>

### White Space

<pre>
  <i>white-space::</i>
    <i>white-space-character</i>
    <i>white-space   white-space-character</i>

  <i>white-space-character::</i>
    <i>new-line</i>
    Space character (U+0020)
    Horizontal-tab character (U+0009)
</pre>

### Tokens

#### General

<pre>
  <i>token::</i>
    <i>variable-name</i>
    <i>name</i>
    <i>keyword</i>
    <i>literal</i>
    <i>operator-or-punctuator</i>
</pre>

#### Names

<pre>
  <i>variable-name::</i>
    $   <i>name</i>

  <i>name::</i>
    <i>name-nondigit</i>
    <i>name   name-nondigit</i>
    <i>name   digit</i>

  <i>name-nondigit::</i>
    <i>nondigit</i>
    one of the characters U+007f–U+00ff

  <i>nondigit::</i> one of
    _
    a   b   c   d   e   f   g   h   i   j   k   l   m
    n   o   p   q   r   s   t   u   v   w   x   y   z
    A   B   C   D   E   F   G   H   I   J   K   L   M
    N   O   P   Q   R   S   T   U   V   W   X   Y   Z
</pre>

### Keywords

<pre>
  <i>keyword::</i> one of
    abstract  arraykey  as  async  break  case  catch  class  classname clone  const  continue  default  do
    echo  else  elseif  enum  extends  final  finally  for  foreach  function  if  implements
    instanceof  insteadof  interface  mixed  namespace  new  newtype  noreturn   num  parent  private
    protected  public  require  require_once  return  self  shape  static  switch  throw  trait  try
    tuple  type  use  while  yield
</pre>

### Literals

#### General

<pre>
  <i>literal::
    <i>boolean-literal</i>
    <i>integer-literal</i>
    <i>floating-literal</i>
    <i>string-literal</i>
    <i>null-literal</i>
</pre>

#### Boolean Literals

<pre>
  <i>boolean-literal::</i>
    true
    false
</pre>

#### Integer Literals

<pre>
  <i>integer-literal::</i>
    <i>decimal-literal</i>
    <i>octal-literal</i>
    <i>hexadecimal-literal</i>
    <i>binary-literal</i>

    <i>decimal-literal::</i>
      <i>nonzero-digit</i>
      <i>decimal-literal   digit</i>

    <i>octal-literal::</i>
      0
      <i>octal-literal   octal-digit</i>

    <i>hexadecimal-literal::</i>
      <i>hexadecimal-prefix   hexadecimal-digit</i>
      <i>hexadecimal-literal   hexadecimal-digit</i>

    <i>hexadecimal-prefix:: one of</i>
      0x  0X

    <i>binary-literal::</i>
      <i>binary-prefix   binary-digit</i>
      <i>binary-literal   binary-digit</i>

    <i>binary-prefix:: one of</i>
      0b  0B

    <i>digit:: one of</i>
      0  1  2  3  4  5  6  7  8  9

    <i>nonzero-digit:: one of</i>
      1  2  3  4  5  6  7  8  9

    <i>octal-digit:: one of</i>
      0  1  2  3  4  5  6  7

    <i>hexadecimal-digit:: one of</i>
      0  1  2  3  4  5  6  7  8  9
            a  b  c  d  e  f
            A  B  C  D  E  F

    <i>binary-digit:: one of</i>
        0  1
</pre>

#### Floating-Point Literals

<pre>
  <i>ﬂoating-literal::</i>
    <i>fractional-literal   exponent-part<sub>opt</sub></i>
    <i>digit-sequence   exponent-part</i>

  <i>fractional-literal::</i>
    <i>digit-sequence<sub>opt</sub></i> . <i>digit-sequence</i>
    <i>digit-sequence</i> .

  <i>exponent-part::</i>
    e  <i>sign<sub>opt</sub>   digit-sequence</i>
    E  <i>sign<sub>opt</sub>   digit-sequence</i>

  <i>sign:: one of</i>
    +  -

  <i>digit-sequence::</i>
    <i>digit</i>
    <i>digit-sequence   digit</i>
</pre>

#### String Literals

<pre>
  <i>string-literal::</i>
    <i>single-quoted-string-literal</i>
    <i>double-quoted-string-literal</i>
    <i>heredoc-string-literal</i>
    <i>nowdoc-string-literal</i>

  <i>single-quoted-string-literal::</i>
    b<i><sub>opt</sub></i>  ' <i>sq-char-sequence<sub>opt</sub></i>  '

  <i>sq-char-sequence::</i>
    <i>sq-char</i>
    <i>sq-char-sequence   sq-char</i>

  <i>sq-char::</i>
    <i>sq-escape-sequence</i>
    \<i><sub>opt</sub></i>   any member of the source character set except single-quote (') or backslash (\)

  <i>sq-escape-sequence:: one of</i>
    \'  \\

  <i>double-quoted-string-literal::</i>
    b<i><sub>opt</sub></i>  " <i>dq-char-sequence<sub>opt</sub></i>  "

  <i>dq-char-sequence::</i>
    <i>dq-char</i>
    <i>dq-char-sequence   dq-char</i>

  <i>dq-char::</i>
    <i>dq-escape-sequence</i>
    any member of the source character set except double-quote (") or backslash (\)
    \  any member of the source character set except "\$efnrtvxX or
octal-digit

  <i>dq-escape-sequence::</i>
    <i>dq-simple-escape-sequence</i>
    <i>dq-octal-escape-sequence</i>
    <i>dq-hexadecimal-escape-sequence</i>

  <i>dq-simple-escape-sequence:: one of</i>
    \"   \\   \$   \e   \f   \n   \r   \t   \v

  <i>dq-octal-escape-sequence::</i>
    \   <i>octal-digit</i>
    \   <i>octal-digit   octal-digit</i>
    \   <i>octal-digit   octal-digit   octal-digit</i>

  <i>dq-hexadecimal-escape-sequence::</i>
    \x  <i>hexadecimal-digit   hexadecimal-digit<sub>opt</sub></i>
    \X  <i>hexadecimal-digit   hexadecimal-digit<sub>opt</sub></i>

  <i>heredoc-string-literal::</i>
    &lt;&lt;&lt; <i>hd-start-identifier   new-line   hd-char-sequence<sub>opt</sub>  new-line hd-end-identifier</i>  ;<i><sub>opt</sub>   new-line</i>

  <i>hd-start-identifier::</i>
    <i>name</i>

  <i>hd-end-identifier::</i>
    <i>name</i>

  <i>hd-char-sequence::</i>
    <i>hd-char</i>
    <i>hd-char-sequence   hd-char</i>

  <i>hd-char::</i>
    <i>hd-escape-sequence</i>
    any member of the source character set except backslash (\)
    \  any member of the source character set except \$efnrtvxX or
octal-digit

  <i>hd-escape-sequence::</i>
    <i>hd-simple-escape-sequence</i>
    <i>dq-octal-escape-sequence</i>
    <i>dq-hexadecimal-escape-sequence</i>

  <i>hd-simple-escape-sequence:: one of</i>
    \\   \$   \e   \f   \n   \r   \t   \v


  <i>nowdoc-string-literal::</i>
    &lt;&lt;&lt; '  <i>hd-start-identifier</i>  '  <i>new-line  hd-char-sequence<sub>opt</sub>   new-line hd-end-identifier</i>  ;<i><sub>opt</sub>   new-line</i>     
</pre>

#### The Null Literal

<pre>
  <i>null-literal::</i>
    null
</pre>

### Operators and Punctuators

<pre>
  <i>operator-or-punctuator:: one of</i>
    [   ]    (   )   {    }   .   -&gt;   ++   --   **   *   +   -   ~   !
    $   /   %   &lt;&lt;   &gt;&gt;   &lt;   &gt;   &lt;=   &gt;=   ==   ===   !=   !==   ^   |
    &amp;   &amp;&amp;   ||   ?   ??   :   ; =   **=   *=   /=   %=   +=   -=   .=   &lt;&lt;=
    &gt;&gt;=   &amp;=   ^=   |=   ,   @   ::   =>   ==>   ?->   \   ...   |>   $$
</pre>

## Syntactic Grammar

### Program Structure

<pre>
<i>script:
  </i>&lt;?hh // strict
  <i>declaration-list<sub>opt</sub></i>

<i>declaration-list:</i>
  <i>declaration</i>
  <i>declaration-list</i> <i>declaration</i>

<i>declaration:</i>
  <i>inclusion-directive</i>
  <i>enum-declaration</i>
  <i>function-definition</i>
  <i>class-declaration</i>
  <i>interface-declaration</i>
  <i>trait-declaration</i>
  <i>namespace-definition</i>
  <i>namespace-use-declaration</i>
  <i>alias-declaration</i>
</pre>

### Types

#### General

<pre>
<i>type-specifier:</i>
  bool
  int
  float
  num
  string
  arraykey
  void
  resource
  <i>alias-type-specifier</i>
  <i>vector-like-array-type-specifier</i>
  <i>map-like-array-type-specifier</i>
  <i>enum-specifier</i>
  <i>class-interface-trait-specifier</i>
  <i>tuple-type-specifier</i>
  <i>shape-specifier</i>
  <i>closure-type-specifier</i>
  <i>nullable-type-specifier</i>
  <i>generic-type-parameter-name</i>
  this
  <i>classname-type-specifier</i>
  <i>type-constant-type-name</i>

<i>alias-type-specifier:</i>
  <i>qualified-name</i>

<i>enum-specifier:</i>
  <i>qualified-name

<i>class-interface-trait-specifier:</i>
  <i>qualified-name generic-type-argument-list<sub>opt</sub></i>

<i>type-specifier-list:</i>
  <i>type-specifiers</i>  ,<sub>opt</sub>

<i>type-specifiers:</i>
  <i>type-specifier</i>
  <i>type-specifiers</i>  ,  <i>type-specifier</i>

<i>type-constraint:</i>
  as  <i>type-specifier</i>

<i>type-constant-type-name:</i>
  <i>name</i>  ::  <i>name</i>
  self  ::  <i>name</i>
  this  ::  <i>name</i>
  <i>type-constant-type-name</i>  ::  <i>name</i>
</pre>

#### Array Types

<pre>
<i>vector-like-array-type-specifier:</i>
  array &lt; <i>array-value-type-specifier</i> &gt;

<i>map-like-array-type-specifier:</i>
  array &lt; <i>array-value-type-specifier</i> , <i>array-value-type-specifier</i> &gt;

<i>array-value-type-specifier:</i>
  <i>type-specifier</i>

<i>array-key-type-specifier:</i>
  <i>type-specifier</i>
</pre>

#### Tuple Types

<pre>
<i>tuple-type-specifier:</i>
  ( <i>type-specifier</i>  ,  <i>type-specifier-list</i>  )
</pre>

#### Shape Types

<pre>
<i>shape-specifier:</i>
  shape ( <i>field-specifier-list<sub>opt</sub></i> )

<i>field-specifier-list:</i>
  <i>field-specifier</i>
  <i>field-specifier-list</i>  ,  <i>field-specifier</i>

<i>field-specifier:</i>
  <i>single-quoted-string-literal</i>  =>  <i>type-specifier</i>
  <i>qualified-name</i>  =>  <i>type-specifier</i>
  <i>scope-resolution-expression</i>  =>  <i>type-specifier</i>
</pre>

#### Closure Types

<pre>
<i>closure-type-specifier:</i>
( function ( <i>type-specifier-list<sub>opt</sub></i> ) : <i>type-specifier</i> )
</pre>

#### Nullable Types

<pre>
<i>nullable-type-specifier:</i>
  ? <i>type-specifier</i>
  mixed
</pre>

#### The Classname Type

<pre>
<i>classname-type-specifier:</i>
  classname   <   <i>qualified-name<i>  <i>generic-type-argument-list<sub>opt</sub></i>  >
</pre>

#### Type Aliases

<pre>
<i>alias-declaration:</i>
  <i>attribute-specification<sub>opt</sub>  type  <i>name</i>  <i>generic-type-parameter-list</i><sub>opt</sub>  =  <i>type-specifier</i>  ;
  <i>attribute-specification<sub>opt</sub>  newtype  <i>name</i>  <i>generic-type-parameter-list</i><sub>opt</sub>  <i>type-constraint<sub>opt</sub></i>  =  <i>type-specifier</i>  ;
</pre>

### Variables

<pre>
  <i>function-static-declaration:</i>
    static <i>static-declarator-list</i>  ;
  <i>static-declarator-list:</i>
    <i>static-declarator</i>  
    <i>static-declarator-list</i>  ,  <i>static-declarator</i>
  <i>static-declarator:</i>
    <i>variable-name</i>  <i>function-static-initializer<sub>opt</sub></i>
  <i>function-static-initializer:</i>
    = <i>const-expression</i>
</pre>

### Expressions

#### Primary Expressions

<pre>
  <i>primary-expression:</i>
    <i>variable-name</i>
    <i>qualified-name</i>
    <i>literal</i>
    <i>const-expression</i>
    <i>intrinsic</i>
    <i>collection-literal</i>
    <i>tuple-literal</i>
    <i>shape-literal</i>
    <i>anonymous-function-creation-expression</i>
    <i>awaitable-creation-expression</i>
    (  <i>expression</i>  )
    $this
    $$

  <i>intrinsic:</i>
    <i>array-intrinsic</i>
    <i>echo-intrinsic</i>
    <i>exit-intrinsic</i>
    <i>invariant-intrinsic</i>
    <i>list-intrinsic</i>

  <i>array-intrinsic:</i>
    array ( <i>array-initializer<sub>opt</sub></i>  )

  <i>echo-intrinsic:</i>
    echo  <i>expression</i>
    echo  (  <i>expression</i>  )
    echo  <i>expression-list-two-or-more</i>

  <i>expression-list-two-or-more:</i>
    <i>expression</i>  ,  <i>expression</i>
    <i>expression-list-two-or-more</i>  ,  <i>expression</i>

  <i>exit-intrinsic:</i>
    exit  <i>expression<sub>opt</sub></i>
    exit  (  <i>expression<sub>opt</sub></i>  )

  <i>invariant-intrinsic:</i>
    invariant  (  <i>condition</i>  ,  <i>format</i>  )
    invariant  (  <i>condition</i>  ,  <i>format</i>  ,  <i>values</i>  )

  <i>list-intrinsic:</i>
    list  (  <i>list-expression-list<sub>opt</sub></i>  )

  <i>list-expression-list:</i>
    <i>expression</i>
    ,
    <i>list-expression-list</i>  ,  <i>expression<sub>opt</sub></i>

  <i>collection-literal:</i>
    <i>non-key-collection-class-type</i>  {  <i>cl-initializer-list-without-keys<sub>opt</sub></i>  }
    <i>key-collection-class-type</i>  {  <i>cl-initializer-list-with-keys<sub>opt</sub></i>  }
    <i>pair-type</i>  {  <i>cl-element-value</i>  ,  <i>cl-element-value</i>  }

  <i>non-key-collection-class-type:</i>
    <i>qualified-name non-key-collection-class-type-hint<sub>opt</sub></i>

  <i>key-collection-class-type:</i>
    <i>qualified-name key-collection-class-type-hint<sub>opt</sub></i>

  <i>non-key-collection-class-type-hint:</i>
    <i>&lt; type-specifier &rt;</i>
  
  <i>key-collection-class-type-hint:</i>
    <i>&lt; type-specifier , type-specifier &rt;</i>

  <i>pair-type:</i>
    <i>qualified-name</i>

  <i>cl-initializer-list-without-keys:</i>
    <i>cl-element-value</i>
    <i>cl-initializer-list-without-keys</i>  ,  <i>cl-element-value</i>

  <i>cl-initializer-list-with-keys:</i>
    <i>cl-element-key</i>  =>  <i>cl-element-value</i>
    <i>cl-initializer-list-with-keys</i>  ,  <i>cl-element-key</i>  =>  <i>cl-element-value</i>

  <i>cl-element-key:</i>
    <i>expression</i>

  <i>cl-element-value:</i>
    <i>expression</i>

  <i>tuple-literal:</i>
    tuple  (  <i>expression-list-one-or-more</i>  )

  <i>expression-list-one-or-more:</i>
    <i>expression</i>
    <i>expression-list-one-or-more</i>  ,  <i>expression</i>

  <i>shape-literal:</i>
    <i>shape</i>  (  <i>field-initializer-list<sub>opt</sub></i>  )

  <i>field-initializer-list:</i>
    <i>field-initializers</i>  ,<sub>opt</sub>

  <i>field-initializers</i>:
    <i>field-initializer</i>
    <i>field-initializers</i>  ,  <i>field-initializer</i>

  <i>field-initializer:</i>
    <i>single-quoted-string-literal</i>  =>  <i>expression</i>
    <i>integer-literal</i>  =>  <i>expression</i>
    <i>qualified-name</i>  =>  <i>expression</i>
    <i>scope-resolution-expression</i>  =>  <i>expression</i>

  <i>anonymous-function-creation-expression:</i>
    async<sub>opt</sub>  function  (  <i>anonymous-function-parameter-list<sub>opt<sub></i>  )  <i>anonymous-function-return<sub>opt</sub></i>  <i>anonymous-function-use-clause<sub>opt</sub></i>  <i>compound-statement</i>

  <i>anonymous-function-parameter-list:</i>
    ...
    <i>anonymous-function-parameter-declaration-list</i>
    <i>anonymous-function-parameter-declaration-list</i>  ,
    <i>anonymous-function-parameter-declaration-list</i>  ,  ...
  
  <i>anonymous-function-parameter-declaration-list:</i>
    <i>anonymous-function-parameter-declaration</i>
    <i>anonymous-function-parameter-declaration-list</i>  ,  <i>anonymous-function-parameter-declaration</i>

  <i>anonymous-function-parameter-declaration:</i>
    <i>attribute-specification<sub>opt</sub>  type-specifier<sub>opt</sub> variable-name  default-argument-specifier<sub>opt</sub></i>

  <i>anonymous-function-return:</i>
    : <i>return-type</i>

  <i>anonymous-function-use-clause:</i>
    use  (  <i>use-variable-name-list</i>  ,<sub>opt</sub> )

  <i>use-variable-name-list:</i>
    <i>variable-name</i>
    <i>use-variable-name-list</i>  ,  <i>variable-name</i>             

  <i>awaitable-creation-expression:</i>
    async   {   <i>async-statement-list<sub>opt</sub></i>   }

  <i>async-statement-list:</i>
    <i>statement</i>
    <i>async-statement-list   statement</i>
</pre>

#### Postfix Operators

<pre>
  <i>postfix-expression:</i>
    <i>-expression</i>
    <i>clone-expression</i>
    <i>object-creation-expression</i>
    <i>array-creation-expression</i>
    <i>subscript-expression</i>
    <i>function-call-expression</i>
    <i>member-selection-expression</i>
    <i>null-safe-member-selection-expression</i>
    <i>postfix-increment-expression</i>
    <i>postfix-decrement-expression</i>
    <i>scope-resolution-expression</i>
    <i>exponentiation-expression</i>


  <i>clone-expression:</i>
    clone  <i>expression</i>

  <i>object-creation-expression:</i>
    new  <i>class-type-designator</i>  (  <i>argument-expression-list<sub>opt</sub></i>  )

  <i>class-type-designator:</i>
    parent
    self
    static
    <i>member-selection-expression</i>
    <i>null-safe-member-selection-expression</i>
    <i>qualified-name</i>
    <i>scope-resolution-expression</i>
    <i>subscript-expression</i>
    <i>variable-name</i>

  <i>array-creation-expression:</i>
    array  (  <i>array-initializer<sub>opt</sub></i>  )
    [ <i>array-initializer<sub>opt</sub></i> ]

  <i>array-initializer:</i>
    <i>array-initializer-list</i>  ,<sub>opt</sub>

  <i>array-initializer-list:</i>
    <i>array-element-initializer</i>
    <i>array-element-initializer  ,  array-initializer-list</i>

  <i>array-element-initializer:</i>
    <i>element-value</i>
    element-key  =>  <i>element-value</i>

  <i>element-key:</i>
    <i>expression</i>

  <i>element-value</i>
    <i>expression</i>

  <i>subscript-expression:</i>
    <i>postfix-expression</i>  [  <i>expression<sub>opt</sub></i>  ]
    <i>postfix-expression</i>  {  <i>expression<sub>opt</sub></i>  }   <b>[Deprecated form]</b>

  <i>function-call-expression:</i>
    <i>postfix-expression</i>  (  <i>argument-expression-list<sub>opt</sub></i>  )

  <i>argument-expression-list:</i>
    <i>argument-expressions</i>  ,<sub>opt</sub>

  <i>argument-expressions:</i>
    <i>expression</i>
    <i>argument-expressions</i>  ,  <i>expression</i>

  <i>member-selection-expression:</i>
    <i>postfix-expression</i>  ->  <i>name</i>
    <i>postfix-expression</i>  ->  <i>variable-name</i>

  <i>null-safe-member-selection-expression:</i>
    <i>postfix-expression</i>  ?->  <i>name</i>
    <i>postfix-expression</i>  ?->  <i>variable-name</i>

  <i>postfix-increment-expression:</i>
    <i>unary-expression</i>  ++

  <i>postfix-decrement-expression:</i>
    <i>unary-expression</i>  --

  <i>scope-resolution-expression:</i>
    <i>scope-resolution-qualifier</i>  ::  <i>name</i>
    <i>scope-resolution-qualifier</i>  ::  <i>variable-name</i>
    <i>scope-resolution-qualifier</i>  ::  class

  <i>scope-resolution-qualifier:</i>
    <i>qualified-name</i>
    <i>variable-name</i>
    self
    parent
    static

  <i>exponentiation-expression:</i>
    <i>expression  **  expression</i>                   
</pre>

#### Unary Operators

<pre>
  <i>unary-expression:</i>
    <i>postfix-expression</i>
    <i>prefix-increment-expression</i>
    <i>prefix-decrement-expression</i>
    <i>unary-op-expression</i>
    <i>error-control-expression</i>
    <i>cast-expression</i>
    <i>await-expression</i>

  <i>prefix-increment-expression:</i>
    ++ <i>unary-expression</i>

  <i>prefix-decrement-expression:</i>
    -- <i>unary-expression</i>

  <i>unary-op-expression:</i>
    <i>unary-operator cast-expression</i>

  <i>unary-operator: one of</i>
    +  -  !  ~

  <i>error-control-expression:</i>
    @  <i>expression</i>

  <i>cast-expression:</i>
    (  <i>cast-type</i>  ) <i>unary-expression</i>

  <i>cast-type: one of</i>
    bool  int  float  string

  <i>await-expression:</i>
    await  <i>expression</i>

</pre>

#### instanceof Operator

<pre>
  <i>instanceof-expression:</i>
    <i>unary-expression</i>
    <i>instanceof-subject</i>  instanceof   <i>instanceof-type-designator</i>

  <i>instanceof-subject:</i>
    <i>expression</i>

  <i>instanceof-type-designator:</i>
    <i>qualified-name</i>
    <i>variable-name</i>
</pre>

#### Multiplicative Operators

<pre>
  <i>multiplicative-expression:</i>
    <i>instanceof-expression</i>
    <i>multiplicative-expression</i>  *  <i>instanceof-expression</i>
    <i>multiplicative-expression</i>  /  <i>instanceof-expression</i>
    <i>multiplicative-expression</i>  %  <i>instanceof-expression</i>
</pre>

#### Additive Operators

<pre>
  <i>additive-expression:</i>
    <i>multiplicative-expression</i>
    <i>additive-expression</i>  +  <i>multiplicative-expression</i>
    <i>additive-expression</i>  -  <i>multiplicative-expression</i>
    <i>additive-expression</i>  .  <i>multiplicative-expression</i>
</pre>

#### Bitwise Shift Operators

<pre>
  <i>shift-expression:</i>
    <i>additive-expression</i>
    <i>shift-expression</i>  &lt;&lt;  <i>additive-expression</i>
    <i>shift-expression</i>  &gt;&gt;  <i>additive-expression</i>
</pre>

#### Relational Operators

<pre>
  <i>relational-expression:</i>
    <i>shift-expression</i>
    <i>relational-expression</i>  &lt;   <i>shift-expression</i>
    <i>relational-expression</i>  &gt;   <i>shift-expression</i>
    <i>relational-expression</i>  &lt;=  <i>shift-expression</i>
    <i>relational-expression</i>  &gt;=  <i>shift-expression</i>
</pre>

#### Equality Operators

<pre>
  <i>equality-expression:</i>
    <i>relational-expression</i>
    <i>equality-expression</i>  ==  <i>relational-expression</i>
    <i>equality-expression</i>  !=  <i>relational-expression</i>
    <i>equality-expression</i>  ===  <i>relational-expression</i>
    <i>equality-expression</i>  !==  <i>relational-expression</i>
</pre>

#### Bitwise Logical Operators

<pre>
  <i>bitwise-AND-expression:</i>
    <i>equality-expression</i>
    <i>bit-wise-AND-expression</i>  &amp;  <i>equality-expression</i>

  <i>bitwise-exc-OR-expression:</i>
    <i>bitwise-AND-expression</i>
    <i>bitwise-exc-OR-expression</i>  ^   <i>bitwise-AND-expression</i>

  <i>bitwise-inc-OR-expression:</i>
    <i>bitwise-exc-OR-expression</i>
    <i>bitwise-inc-OR-expression</i>  |  <i>bitwise-exc-OR-expression</i>
</pre>

#### Logical Operators

<pre>
  <i>logical-AND-expression:</i>
    <i>bitwise-inc-OR-expression</i>
    <i>logical-AND-expression</i>  &amp;&amp;  <i>bitwise-inc-OR-expression</i>

  <i>logical-inc-OR-expression:</i>
    <i>logical-AND-expression</i>
    <i>logical-inc-OR-expression</i>  ||  <i>logical-AND-expression</i>
</pre>

#### Conditional Operator

<pre>
  <i>conditional-expression:</i>
    <i>logical-inc-OR-expression</i>
    <i>logical-inc-OR-expression</i>  ?  <i>expression<sub>opt</sub></i>  :  <i>conditional-expression</i>
</pre>

#### Pipe Operator

<pre>
  <i>piped-expression:</i>
    <i>coalesce-expression</i>
    <i>piped-expression</i>   |>   <i>coalesce-expression</i>
</pre>

#### Lambda Expressions

<pre>
<i>lambda-expression:</i>
  <i>piped-expression</i>
  async<sub>opt</sub>  <i>lambda-function-signature</i>  ==>  <i>lambda-body</i>

<i>lambda-function-signature:</i>
  <i>variable-name</i>
  (  <i>anonymous-function-parameter-declaration-list<sub>opt</sub></i>  )  <i>anonymous-function-return<sub>opt</sub></i>

<i>lambda-body:</i>
  <i>expression</i>
  <i>compound-statement</i>
</pre>

#### Assignment Operators

<pre>
  <i>assignment-expression:</i>
    <i>lambda-expression</i>
    <i>simple-assignment-expression</i>
    <i>compound-assignment-expression</i>

  <i>simple-assignment-expression:</i>
    <i>unary-expression</i>  =  <i>assignment-expression</i>

  <i>byref-assignment-expression:</i>
    <i>unary-expression</i>  =  &  <i>assignment-expression</i>

  <i>compound-assignment-expression:</i>
    <i>unary-expression   compound-assignment-operator   assignment-expression</i>

  <i>compound-assignment-operator: one of</i>
    **=  *=  /=  %=  +=  -=  .=  &lt;&lt;=  >>=  &amp;=  ^=  |=
</pre>

#### yield Operator

<pre>
  <i>expression:</i>
    <i>assignment-expression</i>
    yield  <i>array-element-initializer</i>
</pre>

#### Constant Expressions

<pre>
  <i>constant-expression:</i>
    <i>array-creation-expression</i>
    <i>collection-literal</i>
    <i>tuple-literal</i>
    <i>shape-literal</i>
    <i>const-expression</i>

  <i>const-expression:</i>
    <i>expression</i>
</pre>

### Statements

#### General

<pre>
  <i>statement:</i>
    <i>function-static-declaration</i>
    <i>compound-statement</i>
    <i>labeled-statement</i>
    <i>expression-statement</i>
    <i>selection-statement</i>
    <i>iteration-statement</i>
    <i>jump-statement</i>
    <i>try-statement</i>
</pre>

#### Compound Statements

<pre>
  <i>compound-statement:</i>
    {  <i>statement-list<sub>opt</sub></i>  }

  <i>statement-list:</i>
    <i>statement</i>
    <i>statement-list   statement</i>
</pre>

#### Labeled Statements

<pre>
  <i>labeled-statement:</i>
    <i>case-label</i>
    <i>default-label</i>

  <i>case-label:</i>
    case   <i>expression</i>  :  <i>statement</i>

  <i>default-label:</i>
    default  :  <i>statement</i>
</pre>

#### Expression Statements

<pre>
   <i>expression-statement:</i>
     <i>expression<sub>opt</sub></i>  ;

  <i>selection-statement:</i>
    <i>if-statement</i>
    <i>switch-statement</i>

  <i>if-statement:</i>
    if   (   <i>expression</i>   )   <i>statement   elseif-clauses-opt   else-clause-opt</i>

  <i>elseif-clauses:</i>
    <i>elseif-clause</i>
    <i>elseif-clauses   elseif-clause</i>

  <i>elseif-clause:</i>
    elseif   (   <i>expression</i>   )   <i>statement</i>

  <i>else-clause:</i>
    else   <i>statement</i>

  <i>switch-statement:</i>
    switch  (  <i>expression</i>  )  <i>compound-statement</i>     
</pre>

#### Iteration Statements

<pre>
  <i>iteration-statement:</i>
    <i>while-statement</i>
    <i>do-statement</i>
    <i>for-statement</i>
    <i>foreach-statement</i>

  <i>while-statement:</i>
    while  (  <i>expression</i>  )  <i>statement</i>

  <i>do-statement:</i>
    do  <i>statement</i>  while  (  <i>expression</i>  )  ;

  <i>for-statement:</i>
    for   (   <i>for-initializeropt</i>   ;   <i>for-controlopt</i>   ;   <i>for-end-of-loopopt</i>   )   <i>statement</i>

  <i>for-initializer:</i>
    <i>for-expression-group</i>

  <i>for-control:</i>
    <i>for-expression-group</i>

  <i>for-end-of-loop:</i>
    <i>for-expression-group</i>

  <i>for-expression-group:</i>
    <i>expression</i>
    <i>for-expression-group</i>   ,   <i>expression</i>

  <i>foreach-statement:</i>
    foreach  (  <i>foreach-collection-name</i>  as  <i>foreach-key<sub>opt</sub>  foreach-value</i>  )   statement
    foreach  (  <i>foreach-collection-name</i>  await  as  <i>foreach-key<sub>opt</sub>  foreach-value</i>  )   statement

  <i>foreach-collection-name</i>:
    <i>expression</i>

  <i>foreach-key:</i>
    <i>expression</i>  =>

  <i>foreach-value:<i>
    <i>expression</i>
    <i>list-intrinsic</i>

</pre>

#### Jump Statements

<pre>
  <i>jump-statement:</i>
    <i>continue-statement</i>
    <i>break-statement</i>
    <i>return-statement</i>
    <i>throw-statement</i>

  <i>continue-statement:</i>
    continue  ;

  <i>break-statement:</i>
    break  ;

  <i>return-statement:</i>
    return  <i>expression<sub>opt</sub></i>  ;

  <i>throw-statement:</i>
    throw  <i>expression</i>  ;
</pre>

#### The try Statement

<pre>
  <i>try-statement:</i>
    try  <i>compound-statement   catch-clauses</i>
    try  <i>compound-statement   finally-clause</i>
    try  <i>compound-statement   catch-clauses   finally-clause</i>

  <i>catch-clauses:</i>
    <i>catch-clause</i>
    <i>catch-clauses   catch-clause</i>

  <i>catch-clause:</i>
    catch  (  <i>type-specifier</i>  <i>variable-name</i>  )  <i>compound-statement</i>

  <i>finally-clause:</i>
    finally   <i>compound-statement</i>
</pre>

### Script Inclusion

<pre>
  <i>inclusion-directive:</i>
    <i>require-multiple-directive</i>
    <i>require-once-directive</i>

  <i>require-multiple-directive:</i>
    require  <i>include-filename</i>  ;

  <i>include-filename:</i>
    <i>expression</i>  

  <i>require-once-directive:</i>
    require_once  <i>include-filename</i>  ;
</pre>  

### Enums

<pre>
  <i>enum-declaration:</i>
  enum  <i>name</i>  <i>enum-base</i>  <i>type-constraint<sub>opt</sub></i>  {  <i>enumerator-list<sub>opt</sub></i>  }

  <i>enum-base:</i>
    :  int
    :  string

  <i>enumerator-list:</i>
    <i>enumerator</i>
    <i>enumerator-list</i>  <i>enumerator</i>

  <i>enumerator:</i>
    <i>enumerator-constant</i>  =  <i>constant-expression</i>  ;

  <i>enumerator-constant:</i>
    <i>name</i>
</pre>

### Generic Types, Methods and Functions

<pre>
  <i>generic-type-parameter-list:</i>
    &lt;  <i>generic-type-parameters</i>  ,<sub>opt</sub>  &gt;

  <i>generic-type-parameters:</i>
    <i>generic-type-parameter</i>
    <i>generic-type-parameters</i>  ,  <i>generic-type-parameter</i>

  <i>generic-type-parameter:</i>
    <i>generic-type-parameter-variance<sub>opt</sub></i>  <i>generic-type-parameter-name</i>  <i>type-constraint<sub>opt</sub></i>

  <i>generic-type-parameter-name:</i>
    <i>name</i>

  <i>generic-type-parameter-variance:</i>
    +
    -

  <i>generic-type-argument-list:</i>
    &lt;  <i>generic-type-arguments</i>  ,<sub>opt</sub>  &gt;

  <i>generic-type-arguments:</i>
    <i>generic-type-argument</i>
    <i>generic-type-arguments</i>  ,  <i>generic-type-argument</i>

  <i>generic-type-argument:</i>
    <i>type-specifier</i>
    <i>name</i>
</pre>

### Functions

<pre>
  <i>function-definition:</i>
    <i>attribute-specification<sub>opt</sub>   function-definition-no-attribute</i>

  <i>function-definition-no-attribute:</i>
    <i>function-definition-header  compound-statement</i>

  <i>function-definition-header:</i>
    async<sub>opt</sub>  function <i>name</i>  <i>generic-type-parameter-list<sub>opt</sub></i>  (  <i>parameter-list<sub>opt</sub></i>  ) :  <i>return-type</i>

  <i>parameter-list:</i>
    ...
    <i>parameter-declaration-list</i>  ,<sub>opt</sub>
    <i>parameter-declaration-list</i>  ,  ...

  <i>parameter-declaration-list:</i>
    <i>parameter-declaration</i>
    <i>parameter-declaration-list</i>  ,  <i>parameter-declaration</i>

  <i>parameter-declaration:</i>
    <i>attribute-specification<sub>opt</sub></i>  <i>type-specifier</i>  <i>variable-name  default-argument-specifier<sub>opt</sub></i>

  <i>default-argument-specifier:</i>
    =  <i>const-expression</i>

  <i>return-type:</i>
    <i>type-specifier</i>
    noreturn
</pre>

### Classes

<pre>
  <i>class-declaration:</i>
    <i>attribute-specification<sub>opt</sub></i>  <i>class-modifier<sub>opt</sub></i>  class  <i>name  generic-type-parameter-list<sub>opt</sub></i>  <i>class-base-clause<sub>opt</sub></i>
      <i>class-interface-clause<sub>opt</sub></i>  {  <i>trait-use-clauses<sub>opt</sub>  class-member-declarations<sub>opt</sub></i>  }

  <i>class-modifier:</i>
    abstract
    final
    abstract final

  <i>class-base-clause:</i>
    extends  <i>qualified-name</i>  <i>generic-type-argument-list<sub>opt</sub></i>

  <i>class-interface-clause:</i>
    implements  <i>qualified-name</i>  <i>generic-type-argument-list<sub>opt</sub></i>
    <i>class-interface-clause</i>  ,  <i>qualified-name</i>  <i>generic-type-argument-list<sub>opt</sub></i>

  <i>class-member-declarations:</i>
    <i>class-member-declaration</i>
    <i>class-member-declarations   class-member-declaration</i>

   <i>class-member-declaration:</i>
     <i>const-declaration</i>
     <i>property-declaration</i>
     <i>method-declaration</i>
     <i>constructor-declaration</i>
     <i>destructor-declaration</i>
     <i>type-constant-declaration</i>

  <i>const-declaration:</i>
    abstract<i><sub>opt</sub></i>  const  <i>type-specifier<sub>opt</sub></i>  <i>constant-declarator-list</i>  ;

  <i>constant-declarator-list:</i>
    <i>constant-declarator</i>
    <i>constant-declarator-list</i>  ,  <i>constant-declarator</i>

  <i>constant-declarator:</i>
    <i>name</i>  <i>constant-initializer<sub>opt</sub></i>

  <i>constant-initializer:</i>
    =  <i>const-expression</i>

  <i>property-declaration:</i>
    <i>property-modifier</i>  <i>type-specifier</i>  <i>property-declarator-list</i>  ;

  <i>property-declarator-list:</i>
    <i>property-declarator</i>
    <i>property-declarator-list</i>  ,  <i>property-declarator</i>

  <i>property-declarator:</i>
    <i>variable-name</i>  <i>property-initializer<sub>opt</sub></i>

  <i>property-modifier:</i>
    <i>visibility-modifier</i>  <i>static-modifier<sub>opt</sub></i>
    <i>static-modifier</i>  <i>visibility-modifier</i>

  <i>visibility-modifier:</i>
    public
    protected
    private

  <i>static-modifier:</i>
    static

  <i>property-initializer:</i>
    =  <i>expression</i>

  <i>method-declaration:</i>
    <i>attribute-specification<sub>opt</sub></i> <i>method-modifiers</i>  <i>function-definition-no-attribute</i>
    <i>attribute-specification<sub>opt</sub></i> <i>method-modifiers</i>  <i>function-definition-header</i>  ;

  <i>method-modifiers:</i>
    <i>method-modifier</i>
    <i>method-modifiers</i>  <i>method-modifier</i>

  <i>method-modifier:</i>
    <i>visibility-modifier</i>
    <i>static-modifier</i>
    abstract
    final

  <i>constructor-declaration:</i>
  <i>attribute-specification<sub>opt</sub></i>  <i>constructor-modifiers</i>  function  __construct  (
    <i>constructor-parameter-declaration-list<sub>opt</sub></i>  )  <i>void-return<sub>opt</sub></i>  <i>compound-statement</i>

  <i>constructor-parameter-declaration-list:</i>
    <i>constructor-parameter-declaration</i>
    <i>constructor-parameter-declaration-list</i>  ,  <i>constructor-parameter-declaration</i>

  <i>constructor-parameter-declaration:</i>
    <i>visibility-modifier<sub>opt</sub></i>  <i>type-specifier</i>  <i>variable-name</i> <i>default-argument-specifier<sub>opt</sub></i>

  <i>constructor-modifiers:</i>
    <i>constructor-modifier</i>
    <i>constructor-modifiers</i>  <i>constructor-modifier</i>

  <i>constructor-modifier:</i>
    <i>visibility-modifier</i>
    abstract
    final

  <i>destructor-declaration:</i>
    <i>attribute-specification<sub>opt</sub></i>  <i>visibility-modifier</i>  function  __destruct  ( )  <i>void-return<sub>opt</sub></i>  <i>compound-statement</i>

  <i>void-return</i>:
    : void 

  <i>type-constant-declaration:</i>
    <i>abstract-type-constant-declaration</i>
    <i>concrete-type-constant-declaration</i>
  <i>abstract-type-constant-declaration:</i>
    abstract  const  type  name  <i>type-constraint<sub>opt</sub></i>  ;
  <i>concrete-type-constant-declaration:</i>
    const  type  name  <i>type-constraint<sub>opt</sub><i>  =  <i>type-specifier</i>  ;
</pre>

### Interfaces

<pre>
  <i>interface-declaration:</i>
    <i>attribute-specification<sub>opt</sub></i>  interface  <i>name</i>  <i>generic-type-parameter-list<sub>opt</sub></i>  <i>interface-base-clause<sub>opt</sub></i> {
      <i>interface-member-declarations<sub>opt</sub></i>  }

  <i>interface-base-clause:</i>
    extends  <i>qualified-name</i>  <i>generic-type-argument-list<sub>opt</sub></i>
    <i>interface-base-clause</i>  ,  <i>qualified-name</i>  <i>generic-type-argument-list<sub>opt</sub></i>

  <i>interface-member-declarations:</i>
    <i>interface-member-declaration</i>
    <i>interface-member-declarations   interface-member-declaration</i>

  <i>interface-member-declaration:</i>
    <i>require-extends-clause</i>
    <i>const-declaration</i>
    <i>method-declaration</i>
    <i>type-constant-declaration</i>
</pre>

### Traits

<pre>
  <i>trait-declaration:</i>
   <i>attribute-specification<sub>opt</sub></i>  trait  <i>name</i>  <i>generic-type-parameter-list<sub>opt</sub></i>  <i>class-interface-clause<sub>opt</sub></i>  {
     <i>trait-use-clauses<sub>opt</sub>  trait-member-declarations<sub>opt</sub></i>  }

  <i>trait-use-clauses:</i>
    <i>trait-use-clause</i>
    <i>trait-use-clauses</i>  <i>trait-use-clause</i>

  <i>trait-use-clause:</i>
    use  <i>trait-name-list</i>  ;

  <i>trait-name-list:</i>
    <i>qualified-name</i>  <i>generic-type-argument-list<sub>opt</sub></i>
    <i>trait-name-list</i>  ,  <i>qualified-name</i>  <i>generic-type-argument-list<sub>opt</sub></i>

  <i>trait-member-declarations:</i>
    <i>trait-member-declaration</i>
    <i>trait-member-declarations   trait-member-declaration</i>

  <i>trait-member-declaration:</i>
    <i>require-extends-clause</i>
    <i>require-implements-clause</i>
    <i>property-declaration</i>
    <i>method-declaration</i>
    <i>constructor-declaration</i>
    <i>destructor-declaration</i>

  <i>require-extends-clause:</i>
    require  extends  <i>qualified-name</i>  ;

  <i>require-implements-clause:</i>
    require  implements  <i>qualified-name</i>  ;
</pre>

### Namespaces

<pre>
  <i>namespace-definition:</i>
    namespace  <i>namespace-name</i>  ;
    namespace  <i>namespace-name<sub>opt</sub></i> { <i>declaration-list<sub>opt</sub></i> }

  <i>namespace-use-declaration:</i>
    use <i>namespace-use-kind<sub>opt</sub></i>  <i>namespace-use-clauses</i>  ;
    use <i>namespace-use-kind</i>  <i>namespace-name-as-a-prefix</i>  { <i>namespace-use-clauses</i>  }  ;
    use <i>namespace-name-as-a-prefix</i>  { <i>namespace-use-kind-clauses</i>  }  ;

  <i>namespace-use-clauses:</i>
    <i>namespace-use-clause</i>
    <i>namespace-use-clauses</i>  ,  <i>namespace-use-clause</i>

  <i>namespace-use-clause:</i>
    <i>qualified-name  namespace-aliasing-clause<sub>opt</sub></i>

  <i>namespace-use-kind-clauses:</i>
    <i>namespace-use-kind-clause</i>
    <i>namespace-use-kind-clauses</i>  ,  <i>namespace-use-kind-clause</i>

  <i>namespace-use-kind-clause:</i>
    <i>namespace-use-kind<sub>opt</sub></i>  <i>qualified-name  namespace-aliasing-clause<sub>opt</sub></i>

  <i>namespace-aliasing-clause:</i>
    as  <i>name</i>

  <i>namespace-use-kind</i>:
    function
    const

  <i>namespace-name:</i>
    <i>name </i>
    <i>namespace-name   \   name</i>

  <i>namespace-name-as-a-prefix:</i>
    \
    \<sub>opt</sub>   <i>namespace-name</i>   \
    namespace   \
    namespace   \   <i>namespace-name</i>   \

  <i>qualified-name:</i>
    <i>namespace-name-as-a-prefix<sub>opt</sub>   name</i>
</pre>

### Attributes
<pre>

<i>attribute-specification:</i>
  &lt;&lt;  <i>attribute-list</i>  &gt;&gt;

<i>attribute-list:</i>
  <i>attribute</i>
  <i>attribute-list</i>  ,  <i>attribute</i>

<i>attribute:</i>
  <i>attribute-name</i>  <i>attribute-value-list<sub>opt</sub></i>

<i>attribute-name:</i>
  <i>name</i>

<i>attribute-value-list:</i>
  (  <i>attribute-values<sub>opt</sub></i>  )

<i>attribute-values</i>
  <i>attribute-value</i>
  <i>attribute-values</i>  ,  <i>attribute-value</i>

<i>attribute-value:</i>
  <i>expression</i>
</pre>
