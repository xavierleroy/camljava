open Jni

class cls = object
  method f = print_string "cls.f called"; print_newline()
  method g x =
    print_string "cls.g called with "; print_int x; print_newline();
    x+42
end

let wrap_caml_object() =
  let camlobj = new cls in
  (* Wrap caml object into instance of Testcb (see Testcb.java) *)
  let c = find_class "Testcb" in
  let i = get_methodID c "<init>" "(Lfr/inria/caml/camljava/Callback;)V" in
  let o = alloc_object c in
  call_nonvirtual_void_method o c i [|Obj(wrap_object camlobj)|];
  o

let test() =
  (* Static method invocation *)
  let c = find_class "Test" in
  let f = get_static_methodID c "f" "()V" in
  print_string "Calling Test.f()"; print_newline();
  call_static_void_method c f [||];
  let g = get_static_methodID c "g" "(II)I" in
  print_string "Calling Test.g(12,45)"; print_newline();
  let r = call_static_int_method c g [|Camlint 12; Camlint 45|] in
  print_string "Result is: "; print_string (Int32.to_string r); 
  print_newline();
  (* Static field access *)
  let a = get_static_fieldID c "a" "I" in
  print_string "Current value of Test.a is: ";
  print_string (Int32.to_string (get_static_int_field c a));
  print_newline();
  print_string "Setting Test.a to 12"; print_newline();
  set_static_int_field c a (Int32.of_int 12);
  print_string "Current value of Test.a is: ";
  print_string (Int32.to_string (get_static_int_field c a));
  print_newline();
  (* Object creation *)
  print_string "Creating an instance of Test..."; print_newline();
  let o = alloc_object c in
  let init = get_methodID c "<init>" "()V" in
  call_nonvirtual_void_method o c init [||];
  (* Virtual method invocation *)
  let h = get_methodID c "h" "()I" in
  print_string "Calling testinstance.h()"; print_newline();
  let r = call_int_method o h [||] in
  print_string "Result is: "; print_string (Int32.to_string r); 
  print_newline();
  (* Instance field access *)
  let b = get_fieldID c "b" "I" in
  print_string "Setting testinstance.b to 45"; print_newline();
  set_int_field o b (Int32.of_int 45);
  print_string "Calling testinstance.h()"; print_newline();
  let r = call_int_method o h [||] in
  print_string "Result is: "; print_string (Int32.to_string r); 
  print_newline();
  print_string "Current value of testinstance.b is: ";
  print_string (Int32.to_string (get_int_field o b));
  print_newline()
  (* Callbacks -- CURRENTLY BROKEN *)
(****
  print_string "Wrapping Caml object into Java object..."; print_newline();
  let cb = wrap_caml_object() in
  let k = get_static_methodID c "k" "(LTestcb;I)I" in
  print_string "Calling Test.k(<caml object>, 2)"; print_newline();
  let r = call_static_int_method c k [|Obj cb; Camlint 2|] in
  print_string "Result is: "; print_string (Int32.to_string r); 
  print_newline()
****)

let _ =
  test()
