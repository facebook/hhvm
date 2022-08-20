# Mustache Templates

A quick guide to understand our use of the
[mustache](https://mustache.github.io/mustache.5.html) template system.

## Delimiters

There are two kinds of delimiters for our mustache templates. For most languages
it's `{{` and `}}`. For C++ they were replaced with `<%` and `%>` to avoid
confusion with normal C++ brackets. These are the basic building blocks of
mustache templates. The examples here will use the cpp delimiters. You
should replace those with the double brackets if you are working on another
language.

## Special Delimiters

* Variables: `<%var%>`
* Existence: `<%#var%>content<%/var%>`
* Non Existence: `<%^var%>content<%/var%>`
* Evaluate: `<%#container:var%>the contents of <%var%> are now evaluated<%/container:var%>`
* Include Files: `<% > Path/File%>`
* Comments: Open `<%!` close `%>`
```
<%! This is a comment %>

<%!
This is a multi-line comment
%>
```

## Variables
Most variables are set in `thrift/compiler/generate/t_mstch_generator.cc`.
The pattern that we use throughout our mustache templates is:
`<%container:variable%>`. Where:  
`container`: A node in the AST that's defined in `thrift/parse`.  
`variable` A value within that node.  
To find if a variable already exists, look in `t_mstch_generator`. Find
the function `dump(const container& n)` for your AST node.  
In that function `prepend_prefix("container", result)` has the container
name and `mstch::map result` has the variable name that you can use
inside a mustache template.  

t_mstch_generator.cc
```
mstch::map results {
  {"myvalue", "foo"}
}
return this->prepend_prefix("mycontainer", results);
```

MyTemplate.mustache
```
<%mycontainer:myvalue%>
```

Output
```
foo
```

## Lazy Evaluation
Our implementation of `t_mstch_generator` uses lazy evaluation to not
evaluate the entire AST if it's not needed. That means that if
you want to access a value in a leaf node of the AST, you need to evaluate
everything that comes before it. Starting from the base node `program` you
need to evaluate from there until you read your node value.  
For example, if you want to access an integer `42` which is held in the
`t_const_value` node, you need to write in your mustache template:  

```
<%#program:constants%><%#constant:value%><%#value:integer?%>
  <%value:integer_value%>
<%/value:integer?%><%/constant:value%><%/program:constants%>
```
```
42
```
This evaluates t_program -> t_const -> t_const_value, makes a check to
confirm that the value is integer, and prints the integer variable to
the template.

Also note that `constant:value` is a vector. This means that if that
vector contains more than one element, it will iterate through all the
elements printing them to your output. Ensure proper formatting if you
are printing a list.

## Comments and Indenting
To facilitate the readability of the mustache templates a combination of
comments and indentation is used:
```
<%#program:constants%><%!
  %>These are your constant integers: <%!
  %><%#constant:value%><%!
    %><%#value:integer?%><%!
      %><%value:integer_value%><%!
    %><%/value:integer?%><%!
    %><%^value:integer?%><%!
      %>Not an integer<%!
    %><%/value:integer?%><%!
    %>, <%!
  %><%/constant:value%><%!
%><%/program:constants%>
```
Let's imagine you have various constants values:
`42, "foo", 1.23, 32, 40, false`.  
The output will be:
```
These are your constant integers: 42, Not an integer, Not an integer, 32, 40, Not an integer,
```

## File Paths and Recursion
Every language directory will be the root directory for those
templates. For example, in the `cpp2` directory, if you want to
include `cpp2/File.mustache`:
```
<% > File%>
```

To include a mustache file in a subdirectory `cpp2/Constants/File.mustache`:
```
<% > Constants/File%>
```  

Mustache templates are not aware of relative paths. The language path
will always be the root path even inside subdirectories. To include
a file in another subdirectory you need to specify the path from the
root path.  

A mustache template can also include itself:  
Constants/File.mustache
```
<% > Constants/File%>
```
This is helpful if you want to iterate through a list of elements. To
avoid infinite loops, make sure that you have a check to redirect your
File evaluation.  
Constants/File.mustache
```
<%#contant:value%><%!
  %><%#value:container?%><%!
    %><% > Constants/File%><%!
  %><%/value:container?%><%!
%><%/contant:value%>
```
Everytime that you evaluate `constant:value`, it will check if it's a
container. If it is, the recurse and use the value inside that
the new container. Otherwise, do not recurse.  

Notice that all the variables that have been evaluated before including
the file will also be available inside the file.

## Extending variables
To add a missing variable required by `t_mstch_generator`, have
`thrift/compiler/generate/t_mstch_<lang>_generator.cc` inherit from
`t_mstch_generator` and use the hooks: `extend_node(const node& n)` to
add extra variables (or variable formatting) that are specific to your
language.
