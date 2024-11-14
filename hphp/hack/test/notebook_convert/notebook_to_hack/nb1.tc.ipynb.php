<?hh
// copyright header here
//@bento-notebook:{name: "n1234"}
//@bento-cell:{"id": 2, "cell_type": "markdown"}
/*
# Check it out

I am a *markdown* **cell**
*/

//@bento-cell:{"id": 4, "cell_type": "code"}
/* Very classy */
class MyClass {}

function notebook_main_n1234(): void {
  //@bento-cell:{"id": 1, "cell_type": "code"}
  new MyClass();
  echo "hi3";
  //@bento-cell:{"id": 4, "cell_type": "code"}
  echo "hi2";
  $x = 3;
  echo $x + 1;
  echo "hi0";
  //@bento-cell:{"id": 4, "cell_type": "code"}
  // I am a comment
  echo "hi1";
}
