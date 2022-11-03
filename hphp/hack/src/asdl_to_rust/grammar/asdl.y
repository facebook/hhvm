%start Module
%avoid_insert "IDENTIFIER"
%avoid_insert "STRING_LITERAL"
%%
Module -> Result<ast::Module, ()>:
  'MODULE' Identifier Version_opt 'LBRACE' Definitions 'RBRACE' {
      let id = $2?;
      let version = $3?;
      let definitions = $5?;
      Ok(ast::Module{id, version, definitions})
  }
  ;

Identifier -> Result<String, ()>:
   'IDENTIFIER' {
      let v = $1.map_err(|_| ())?;
      Ok($lexer.span_str(v.span()).to_owned())
    }
  ;

Version_opt -> Result<Option<String>, ()>:
  'VERSION' String_ {
     let v = $2.map_err(|_| ())?;
     Ok(Some(v))
    }
  | { Ok(None) }
  ;

Definition -> Result<ast::Definition, ()>:
  Identifier 'EQUAL' Desc Attributes_opt {
    let type_id = $1?;
    let desc = $3?;
    let attributes = $4?.unwrap_or_default();
    Ok(ast::Definition{type_id, desc, attributes})
  }
  ;

Definitions -> Result<Vec<ast::Definition>, ()>:
    { Ok(vec![]) }
  | Definitions Definition {
      let mut defs = $1?;
      defs.push($2?);
      Ok(defs)
    }
  ;

Desc -> Result<ast::Desc, ()>:
    Product { Ok(ast::Desc::Product($1?)) }
  | Sum { Ok(ast::Desc::Sum($1?)) }
  ;

Product -> Result<ast::Product, ()>:
  'LPAREN' Fields 'RPAREN' {
    Ok(ast::Product($2?))
  }
  ;

Sum -> Result<ast::Sum, ()>:
   Constructors {
     $1
   }
  ;

Constructors -> Result<Vec<ast::Constructor>, ()>:
    Constructor { Ok(vec![$1?]) }
  | Constructors 'PIPE' Constructor {
      let mut ctors = $1?;
      ctors.push($3?);
      Ok(ctors)
    }
  ;

Constructor -> Result<ast::Constructor, ()>:
    Identifier Fields_opt {
      let constructor_id = $1?;
      let fields = $2?.unwrap_or_default();
      Ok(ast::Constructor{constructor_id, fields})
    }
  ;

Fields_opt -> Result< Option<Vec<ast::Field>> , ()>:
    { Ok(None) }
  | 'LPAREN' Fields 'RPAREN' {
      Ok(Some($2?))
    }
  ;

Fields -> Result< Vec<ast::Field>, ()>:
    Field {
      Ok(vec![$1?])
    }
  | Fields 'COMMA' Field {
      let mut fields = $1?;
      fields.push($3?);
      Ok(fields)
    }
  ;

Field -> Result <ast::Field, ()>:
    Identifier Opt_modifier FieldId {
        let type_id = $1?;
        let modifier = $2?;
        let id = $3?;
        Ok(ast::Field{type_id, modifier, id})
    }
  ;

FieldId -> Result<String, ()>:
    'MODULE' { Ok("module".to_string()) }
  | 'ATTRIBUTES' { Ok("attributes".to_string()) }
  | Identifier { $1 }
  ;

Attributes_opt -> Result<Option<ast::Attributes>, ()>:
    { Ok(None) }
  | 'ATTRIBUTES' 'LPAREN' Fields 'RPAREN' {
      Ok(Some(ast::Attributes($3?)))
    }
  ;

Opt_modifier -> Result<Option<ast::Modifier>, ()>:
                     { Ok(None) }
  |  'QUESTION_MARK' {
      Ok(Some(ast::Modifier::Question))
    }
  | 'STAR' {
      Ok(Some(ast::Modifier::Star))
    }
  ;

String_ -> Result<String, ()>:
    'STRING_LITERAL' {
       let v = $1.map_err(|_| ())?;
       let s = $lexer.span_str(v.span());
       Ok(s[1..s.len()-1].to_owned())
    }
  ;

%%

pub mod ast {

  #[derive(Debug)]
  pub enum Modifier {
        Question,
        Star,
    }

  #[derive(Debug)]
  pub struct Field {
      pub type_id: String,
      pub modifier: Option<Modifier>,
      pub id: String,
  }

  #[derive(Debug)]
  pub struct Constructor {
      pub constructor_id: String,
      pub fields: Vec<Field>,
  }

  #[derive(Debug)]
  pub struct Product(pub Vec<Field>);

  pub type Sum = Vec<Constructor>;

  #[derive(Debug)]
  pub enum Desc {
    Product(Product),
    Sum(Sum)
  }

  #[derive(Debug, Default)]
  pub struct Attributes(pub Vec<Field>);

  #[derive(Debug)]
  pub struct Definition {
    pub type_id: String,
    pub desc: Desc,
    pub attributes: Attributes,
  }

  #[derive(Debug)]
  pub struct Module {
    pub id: String,
    pub version: Option<String>,
    pub definitions: Vec<Definition>,
  }
}
