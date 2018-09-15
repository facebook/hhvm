(* this is a simple model of exceptions *)

type ex = string
type handler = FH of int | CH of int

(* the exception, was the original raise in a funclet, depth *)
type exinfo = ex * bool * handler list

type instr =
 | Print
 | Int of int
 | Exn of string
 | Throw
 | Unwind
 | Label of int
 | Try
 | CatchMid
 | TryFault of int
 | EndTryFault
 | Jmp of int (* label *)
 | Ret
 | Pop

type exnstack = exinfo list

type value = I of int | E of string
type stack = value list

type program = instr list

type config = exnstack * stack * int


(* given a list of instructions, a current index and a nest count
   return index of matching catch *)
let rec find_catch idx depth l =
match l with
| [] -> failwith "findcatch"
| Try :: rest -> find_catch (idx+1) (depth+1) rest
| CatchMid :: rest -> if depth = 0 then idx else find_catch (idx+1) (depth-1) rest
| _ :: rest -> find_catch (idx+1) depth rest

(* return index of a given label *)
let rec find_label idx n l =
match l with
| [] -> failwith "find_label"
| Label n' :: _ when n=n' -> idx
| _ :: rest -> find_label (idx+1) n rest

(* produce list of static handler lists *)
let rec statics idx current l =
match l with
| [] -> []
| Try :: rest -> let matching_catch = find_catch (idx+1) 0 rest in
                 let newcurrent = CH matching_catch :: current in
                 newcurrent :: (statics (idx+1) newcurrent rest)
| CatchMid :: rest -> begin
  match current with
   | [] -> failwith "statics no matching try"
   | (CH _h) :: others -> current :: (statics (idx+1) others rest)
   | (FH _h) :: _others -> failwith "statics try matched with FH"
  end
| TryFault n :: rest ->
  let epc = find_label (idx+1) n rest (* must be later, this is rubbish but I'm lazy *)
  in let newcurrent = FH epc :: current in
  newcurrent :: (statics (idx+1) newcurrent rest)
| EndTryFault :: rest ->
   let newcurrent =
     match current with
     | FH _ :: others -> others
     | _ -> failwith "statics expected tryfault"
   in newcurrent :: (statics (idx+1) newcurrent rest)
| _ :: rest -> current :: (statics (idx+1) current rest)


let test1 = ([
 TryFault 1;
 Exn "1";
 Throw;
 EndTryFault], [
Label 1;
 Int 42;
 Print;
 Unwind
])


let stepfun (p, faultsection) =
  let infaultfunclet pc = pc >= List.length p in
  let p = p @ faultsection in
  let staticsp = statics 0 [] p in
  let rec steps ((es,s,pc) : config) =
   match List.nth p pc with
   | Print -> let str, news = begin match s with
              | I n :: s' -> string_of_int n, s'
              | E s :: s' -> s, s'
              | _ -> failwith "print underflow"
              end in begin
                Printf.printf "%s\n" str;
                (es,news,pc+1)
              end
   | Int n -> (es, I n :: s, pc+1)
   | Exn str -> (es, E str :: s, pc+1)
   | Label _ -> (es,s,pc+1) (* nop for running *)
   | Throw -> begin match s with
              | E str :: s' ->
               let mystatic = List.nth staticsp pc in
               (* let depth = List.length mystatic in *)
               let newinfo = (str, infaultfunclet pc, mystatic) in
                unwind (newinfo::es, s')
              | _ -> failwith "throw bad stack"
              end
   | Unwind -> unwind (es,s)
   | Try
   | CatchMid
   | TryFault _
   | EndTryFault -> (es,s,pc+1) (* all nops too *)
   | Jmp n -> (es,s, find_label 0 n p)
   | Ret -> failwith "returned"
   | Pop -> begin match s with
      | [] -> failwith "pop underflow"
      | _ :: s' -> (es,s',pc+1)
     end
 and unwind (es,s) =
   match es with
   | [] -> failwith "empty exn stack in unwind"
   | (str,infunc,statics) :: rest ->
      begin match statics with
       | h :: remainingstatics ->
         let newes = (str,infunc,remainingstatics) :: rest in
         begin match h with
          | FH n -> (newes,s,n) (* transfer to fault funclet after
                                        rewriting top exn info *)
          | CH n -> (rest, E str :: s, n) (* pop exn from info stack to normal one
                                            and transfer control *)
         end
       | [] -> (* step 5, our depth has decreased all the way to zero *)
         if infunc then
          begin match rest with
            | (ystr,yinfunc,ystatics) :: restrest ->
               let xupdstr = str ^ "(" ^ ystr ^ ")" in
                unwind ((xupdstr,yinfunc,ystatics)::restrest, s)
            | _ -> failwith "infunc no enclosing handler"
          end
         else (* primary function body, so we exit *)
          failwith ("thrown out: " ^ str)
      end
  in steps

let bar = ([
 Try;
  TryFault 2;
   Exn "1";
   Throw;
  EndTryFault;
  Exn "in finally";
  Print;
  Jmp 0;
  CatchMid;
   Pop;
   Exn "in catch";
   Print;
   Jmp 0;
Label 0;
 Int 0;
 Ret], [
Label 2;
 Exn "in finally";
 Print;
 Unwind
])

let brokenbar = ([
 Try;
  Exn "1";
  Throw;
 CatchMid;
 Exn "in finally";
 Print;
 Pop; (* throw away exn object *)
 Exn "in catch";
 Print;
 Jmp 0;
Label 0;
 Int 0;
 Ret;
],[])

let brokenbar2 =([
 Try;
  Exn "1";
  Throw;
 CatchMid;
  TryFault 1;
   Exn "in finally";
   Print;
  EndTryFault;
  Pop; (* discard exn *)
  Exn "in catch";
  Print;
  Jmp 0;
 Label 0;
 Int 0;
 Ret
],[
Label 1;
Pop; (* discard *)
Exn "in catch at F1";
Print;
Int 0;
Ret
])

let shortlop = ([
 TryFault 0;
 TryFault 1;
 Exn "one";
 Throw;
 EndTryFault;
 EndTryFault], [
 Label 1;
 Exn "two";
 Throw;
 Label 0;
 Exn "outer finally";
 Print;
 Unwind
])

let goo = ([

])
let init p =
 let step = stepfun p in
 let r = ref (([],[],0) : config) in
 fun () -> let rv = step (!r) in
           (r := rv; rv)

let smash (l,l') = l @ l'
