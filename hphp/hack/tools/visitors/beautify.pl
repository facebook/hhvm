#!/usr/bin/perl

my $s = "";

sub splitCond{
  $indent = $_[0];
  $cond = $_[1];
  $cond =~ s/ &&/\n${indent} &&/g;
  return $cond;
}

sub doEndo{
  $x = $_[0];
  $x =~ s/if true then this else (.*)/this/g;
  $x =~ s/ as this -> / -> /g;
  $x =~ s/ this =\n\s*this\n/ this = this\n/g;
  $x =~ s/Pervasives.\(==\) c([0-9]*) r\1/c$1 == r$1/g;
  $x =~ s/(Pervasives.\(==\) [\w\d_\.]+)\n\s+/$1 /g;
  $x =~ s/Pervasives.\(==\) (this\.\w*) (r[0-9]*)/$1 == $2/g;
  while ($x =~ s/Pervasives.\(&&\)(\s*)\(([^P][^)\n]*)\)\1\(([^P][^)\n]*)\)/$2 && $3/gm) {};
  while ($x =~ s/^(\s+)\(Pervasives.\(&&\) \(([^)]*)\)\n\s+\(([^)]*)\)\)/$1($2 && $3)/gm) {};
  $x =~ s/Pervasives\.\(&&\) \(([^)]*)\)\n\s+\(([^)]*)\)/$1 && $2/gm;
  $x =~ s/\s+(.*&&)/ $1/gm;
  $x =~ s/(^\s+)if (.{70,})/"$1if  ".splitCond($1,$2)/gme;
  return $x;
}

sub doIter{
  $x = $_[0];

  # In the case of iter-visitors, all the `let rX = term in` stuff generates
  # warnings about unused variables (because the result is always unit).
  $x =~ s/let r[0-9]* = (.*) in/\1;/g;
  $x =~ s/\n(\s+)let r\d+ =((\n\1  .*)+)\n\1in/\n$1begin$2\n$1end;/g;
  $x =~ s/^(\s+)let r[0-9]* =((\n\1  .*)*)(\n\1| )in/\2\1;/gm;
  $x =~ s/\s*;\n\s+\(\)\n/;\n/g;

  $x =~ s/(method.*) this =\n\s+match this with\n/$1 = function\n/g;
  $x =~ s/^(\s+)(method.*) env c0 =\n\1  self(.*) env c0;/\1\2 = self$3/gm;
  return $x;
}

sub doReduce{
    $x = $_[0];

    # For reduce; I use a different naming convention for monoids.
    my %replace = ( zero => "self#e",
                    plus => "self#add" );
    $x =~ s/self#(zero|plus)/$replace{$1}/eg;

    sub flatten{
      $s = $_[0];
      $s =~ s/\s+/ /gm;
      $s =~ s/\)//g;
      $s =~ s/ r/; r/g;
      return "self#sum [ $s]";
    }
    $x =~ s/^(\s+)  ((self#add\s*\()*self#add ((r\d+\)?\s*){4,}))/"$1  ".flatten($4)."\n$1"/gme;
    $x =~ s/(method.*) this =\n\s+match this with\n/$1 = function\n/g;
    $x =~ s/let r0 = (.*) in r0/$1/g;
    $x =~ s/^(\s+)(method.*) env c0 =\n\1  self(.*) env c0/\1\2 = self$3/gm;
    $x =~ s/ env = self#e\n/ _ = self#e\n/g;
    $x =~ s/^(\s+)(let .* in) self/$1$2\n$1self/gm;
    $x =~ s/ env\n\s+c/ env c/g;
    $x =~ s/\n+/\n/g;
    return $x;
}

# Per line of the input
while (<>) {
  # Rename the visitors calling "out of" the AST type
  s/self#visit_t /self#visit_Pos_t /g;
  s/self#visit_env /self#visit_Namespace_env /g;
  s/self#visit_mode /self#visit_FileInfo_mode /g;

  # Since the module will be Ast_visitors, I find `visit` too verbose.
  s/(self#|method )visit_/\1on_/g;

  # `fun env -> fun (c0, c1, c2) ->` doesn't read well; `fun env (c0, c1, c2) ->`
  s/fun env ->\n/fun env ->/g;

  # A little line breaking in the right place (`let` after `=` and `in ()`)
  s/^(\s+)(.* =) (let .*)/\1\2\n\1  \3/g;
  s/^(\s+)(let .*? in) \(\)\n/\1\2\n\1()\n/gm;
  $s .= $_;
}

$s =~ s/ +;/;/g;
$s =~ s/\n\n/\n/g;

# Some multi-line magic (or stuff that became a single line only after the block
# above)
$s =~ s/fun env ->\s+fun /fun env /g;
$s =~ s/^(\s+)(let.*(?!in))\n\1in/\1\2 in\n/gm;
$s =~ s/^\s*\n//g;

# Cleaning up record syntax
#$s =~ s/^(\s)\{\n(.*)^\1\}/&/gmse;

$s =~ s/(^class.* reduce =\n(\s+)object .*\n)((^\2.*\n|^ *\n)*?)\2end/$1.doReduce($3).$2."end"/me;
$s =~ s/(^class.* iter =\n(\s+)object .*\n)((^\2.*\n|^ *\n)*?)\2end/$1.doIter($3).$2."end"/me;
$s =~ s/(^class.* endo =\n(\s+)object .*\n)((^\2.*\n|^ *\n)*?)\2end/$1.doEndo($3).$2."end"/me;


print $s;
