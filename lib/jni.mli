(***********************************************************************)
(*                                                                     *)
(*             OCamlJava: Objective Caml / Java interface              *)
(*                                                                     *)
(*            Xavier Leroy, projet Cristal, INRIA Rocquencourt         *)
(*                                                                     *)
(*  Copyright 2001 Institut National de Recherche en Informatique et   *)
(*  en Automatique.  All rights reserved.  This file is distributed    *)
(*  under the terms of the GNU Library General Public License.         *)
(*                                                                     *)
(***********************************************************************)

(* $Id: jni.mli,v 1.2 2005-10-21 08:19:11 xleroy Exp $ *)

(* Low-level Java interface (JNI level) *)

external set_debug: bool -> unit = "camljava_set_debug"

(* Object operations *)

type obj
        (* The type of Java object references *)
val null: obj
        (* The [null] object reference *)
exception Null_pointer
        (* Exception raised by the operations below when they
           encounter a null object reference in arguments that must
           be non-null. *)
external is_null: obj -> bool = "camljava_IsNull"
        (* Determine if the given object reference is [null] *)
external is_same_object: obj -> obj -> bool = "camljava_IsSameObject"
        (* Determine if two object references are the same 
           (as per [==] in Java). *)

(* String operations.  Java strings are represented in Caml
   by their UTF8 encoding. *)

external string_to_java: string -> obj = "camljava_MakeJavaString"
external string_from_java: obj -> string = "camljava_ExtractJavaString"
        (* Conversion between Caml strings and Java strings. *)
val null_string: string
        (* A distinguished Caml string that represents the [null]
           Java string reference. *)
val is_null_string: string -> bool
        (* Determine whether its argument is the distinguished Caml string
           representing the [null] Java string reference. *)

(* Class operations *)

type clazz
        (* The type of class identifiers *)

external find_class: string -> clazz
        = "camljava_FindClass"
        (* Find a class given its fully qualified name, e.g. 
           "java/lang/Object".  Note the use of slashes [/] to separate
           components of the name. *)
external get_superclass: clazz -> clazz
        = "camljava_GetSuperclass"
        (* Return the super-class of the given class. *)
external is_assignable_from: clazz -> clazz -> bool
        = "camljava_IsAssignableFrom"
        (* Assignment compatibility predicate. *)
external get_object_class: obj -> clazz = "camljava_GetObjectClass"
        (* Return the class of an object. *)
external is_instance_of: obj -> clazz -> bool = "camljava_IsInstanceOf"
        (* Determine if the given object reference is an instance of the
           given class *)
external alloc_object: clazz -> obj = "camljava_AllocObject"
        (* Allocate a new instance of the given class *)

(* Field and method identifiers *)

type fieldID
        (* The type of field identifiers *)
type methodID
        (* The type of method identifiers *)

external get_fieldID: clazz -> string -> string -> fieldID
        = "camljava_GetFieldID"
        (* [get_fieldID cls name descr] returns the identifier of
           the instance field named [name] with descriptor (type) [descr]
           in class [cls]. *)
external get_static_fieldID: clazz -> string -> string -> fieldID
        = "camljava_GetStaticFieldID"
        (* Same, for a static field. *)
external get_methodID: clazz -> string -> string -> methodID
        = "camljava_GetMethodID"
        (* [get_methodID cls name descr] returns the identifier of
           the virtual method named [name] with descriptor (type) [descr]
           in class [cls]. *)
external get_static_methodID: clazz -> string -> string -> methodID
        = "camljava_GetStaticMethodID"
        (* Same, for a static method. *)

(* Field access *)

external get_object_field: obj -> fieldID -> obj
        = "camljava_GetObjectField"
external get_boolean_field: obj -> fieldID -> bool
        = "camljava_GetBooleanField"
external get_byte_field: obj -> fieldID -> int
        = "camljava_GetByteField"
external get_char_field: obj -> fieldID -> int
        = "camljava_GetCharField"
external get_short_field: obj -> fieldID -> int
        = "camljava_GetShortField"
external get_int_field: obj -> fieldID -> int32
        = "camljava_GetIntField"
external get_camlint_field: obj -> fieldID -> int
        = "camljava_GetCamlintField"
external get_long_field: obj -> fieldID -> int64
        = "camljava_GetLongField"
external get_float_field: obj -> fieldID -> float
        = "camljava_GetFloatField"
external get_double_field: obj -> fieldID -> float
        = "camljava_GetDoubleField"

external set_object_field: obj -> fieldID -> obj -> unit
        = "camljava_SetObjectField"
external set_boolean_field: obj -> fieldID -> bool -> unit
        = "camljava_SetBooleanField"
external set_byte_field: obj -> fieldID -> int -> unit
        = "camljava_SetByteField"
external set_char_field: obj -> fieldID -> int -> unit
        = "camljava_SetCharField"
external set_short_field: obj -> fieldID -> int -> unit
        = "camljava_SetShortField"
external set_int_field: obj -> fieldID -> int32 -> unit
        = "camljava_SetIntField"
external set_camlint_field: obj -> fieldID -> int -> unit
        = "camljava_SetCamlintField"
external set_long_field: obj -> fieldID -> int64 -> unit
        = "camljava_SetLongField"
external set_float_field: obj -> fieldID -> float -> unit
        = "camljava_SetFloatField"
external set_double_field: obj -> fieldID -> float -> unit
        = "camljava_SetDoubleField"

external get_static_object_field: clazz -> fieldID -> obj
        = "camljava_GetStaticObjectField"
external get_static_boolean_field: clazz -> fieldID -> bool
        = "camljava_GetStaticBooleanField"
external get_static_byte_field: clazz -> fieldID -> int
        = "camljava_GetStaticByteField"
external get_static_char_field: clazz -> fieldID -> int
        = "camljava_GetStaticCharField"
external get_static_short_field: clazz -> fieldID -> int
        = "camljava_GetStaticShortField"
external get_static_int_field: clazz -> fieldID -> int32
        = "camljava_GetStaticIntField"
external get_static_camlint_field: clazz -> fieldID -> int
        = "camljava_GetStaticCamlintField"
external get_static_long_field: clazz -> fieldID -> int64
        = "camljava_GetStaticLongField"
external get_static_float_field: clazz -> fieldID -> float
        = "camljava_GetStaticFloatField"
external get_static_double_field: clazz -> fieldID -> float
        = "camljava_GetStaticDoubleField"

external set_static_obj_field: clazz -> fieldID -> obj -> unit
        = "camljava_SetStaticObjectField"
external set_static_boolean_field: clazz -> fieldID -> bool -> unit
        = "camljava_SetStaticBooleanField"
external set_static_byte_field: clazz -> fieldID -> int -> unit
        = "camljava_SetStaticByteField"
external set_static_char_field: clazz -> fieldID -> int -> unit
        = "camljava_SetStaticCharField"
external set_static_short_field: clazz -> fieldID -> int -> unit
        = "camljava_SetStaticShortField"
external set_static_int_field: clazz -> fieldID -> int32 -> unit
        = "camljava_SetStaticIntField"
external set_static_camlint_field: clazz -> fieldID -> int -> unit
        = "camljava_SetStaticCamlintField"
external set_static_long_field: clazz -> fieldID -> int64 -> unit
        = "camljava_SetStaticLongField"
external set_static_float_field: clazz -> fieldID -> float -> unit
        = "camljava_SetStaticFloatField"
external set_static_double_field: clazz -> fieldID -> float -> unit
        = "camljava_SetStaticDoubleField"

(* Method invocation *)

type argument =
    Boolean of bool
  | Byte of int
  | Char of int
  | Short of int
  | Camlint of int
  | Int of int32
  | Long of int64
  | Float of float
  | Double of float
  | Obj of obj
        (* Datatype representing one argument of a Java method. *)

external call_object_method: obj -> methodID -> argument array -> obj
        = "camljava_CallObjectMethod"
external call_boolean_method: obj -> methodID -> argument array -> bool
        = "camljava_CallBooleanMethod"
external call_byte_method: obj -> methodID -> argument array -> int
        = "camljava_CallByteMethod"
external call_char_method: obj -> methodID -> argument array -> int
        = "camljava_CallCharMethod"
external call_short_method: obj -> methodID -> argument array -> int
        = "camljava_CallShortMethod"
external call_int_method: obj -> methodID -> argument array -> int32
        = "camljava_CallIntMethod"
external call_camlint_method: obj -> methodID -> argument array -> int
        = "camljava_CallCamlintMethod"
external call_long_method: obj -> methodID -> argument array -> int64
        = "camljava_CallLongMethod"
external call_float_method: obj -> methodID -> argument array -> float
        = "camljava_CallFloatMethod"
external call_double_method: obj -> methodID -> argument array -> float
        = "camljava_CallDoubleMethod"
external call_void_method: obj -> methodID -> argument array -> unit
        = "camljava_CallVoidMethod"

external call_static_object_method:
                 clazz -> methodID -> argument array -> obj
        = "camljava_CallStaticObjectMethod"
external call_static_boolean_method:
                 clazz -> methodID -> argument array -> bool
        = "camljava_CallStaticBooleanMethod"
external call_static_byte_method:
                 clazz -> methodID -> argument array -> int
        = "camljava_CallStaticByteMethod"
external call_static_char_method:
                 clazz -> methodID -> argument array -> int
        = "camljava_CallStaticCharMethod"
external call_static_short_method:
                 clazz -> methodID -> argument array -> int
        = "camljava_CallStaticShortMethod"
external call_static_int_method:
                 clazz -> methodID -> argument array -> int32
        = "camljava_CallStaticIntMethod"
external call_static_camlint_method:
                 clazz -> methodID -> argument array -> int
        = "camljava_CallStaticCamlintMethod"
external call_static_long_method:
                 clazz -> methodID -> argument array -> int64
        = "camljava_CallStaticLongMethod"
external call_static_float_method:
                 clazz -> methodID -> argument array -> float
        = "camljava_CallStaticFloatMethod"
external call_static_double_method:
                 clazz -> methodID -> argument array -> float
        = "camljava_CallStaticDoubleMethod"
external call_static_void_method:
                 clazz -> methodID -> argument array -> unit
        = "camljava_CallStaticVoidMethod"

external call_nonvirtual_object_method:
                 obj -> clazz -> methodID -> argument array -> obj
        = "camljava_CallNonvirtualObjectMethod"
external call_nonvirtual_boolean_method:
                 obj -> clazz -> methodID -> argument array -> bool
        = "camljava_CallNonvirtualBooleanMethod"
external call_nonvirtual_byte_method: 
                 obj -> clazz -> methodID -> argument array -> int
        = "camljava_CallNonvirtualByteMethod"
external call_nonvirtual_char_method:
                 obj -> clazz -> methodID -> argument array -> int
        = "camljava_CallNonvirtualCharMethod"
external call_nonvirtual_short_method:
                 obj -> clazz -> methodID -> argument array -> int
        = "camljava_CallNonvirtualShortMethod"
external call_nonvirtual_int_method:
                 obj -> clazz -> methodID -> argument array -> int32
        = "camljava_CallNonvirtualIntMethod"
external call_nonvirtual_camlint_method:
                 obj -> clazz -> methodID -> argument array -> int
        = "camljava_CallNonvirtualCamlintMethod"
external call_nonvirtual_long_method:
                 obj -> clazz -> methodID -> argument array -> int64
        = "camljava_CallNonvirtualLongMethod"
external call_nonvirtual_float_method:
                 obj -> clazz -> methodID -> argument array -> float
        = "camljava_CallNonvirtualFloatMethod"
external call_nonvirtual_double_method:
                 obj -> clazz -> methodID -> argument array -> float
        = "camljava_CallNonvirtualDoubleMethod"
external call_nonvirtual_void_method:
                 obj -> clazz -> methodID -> argument array -> unit
        = "camljava_CallNonvirtualVoidMethod"

(* Arrays *)

external get_array_length: obj -> int = "camljava_GetArrayLength"

external new_object_array: int -> clazz -> obj
        = "camljava_NewObjectArray"
external get_object_array_element: obj -> int -> obj
        = "camljava_GetObjectArrayElement"
external set_object_array_element: obj -> int -> obj -> unit
        = "camljava_SetObjectArrayElement"
external new_boolean_array: int -> obj
        = "camljava_NewBooleanArray"
external get_boolean_array_element: obj -> int -> bool
        = "camljava_GetBooleanArrayElement"
external set_boolean_array_element: obj -> int -> bool -> unit
        = "camljava_SetBooleanArrayElement"
external new_byte_array: int -> obj
        = "camljava_NewByteArray"
external get_byte_array_element: obj -> int -> int
        = "camljava_GetByteArrayElement"
external set_byte_array_element: obj -> int -> int -> unit
        = "camljava_SetByteArrayElement"
external get_byte_array_region: obj -> int -> string -> int -> int -> unit
        = "camljava_GetByteArrayRegion"
external set_byte_array_region: string -> int -> obj -> int -> int -> unit
        = "camljava_SetByteArrayRegion"
external new_char_array: int -> obj
        = "camljava_NewCharArray"
external get_char_array_element: obj -> int -> int
        = "camljava_GetCharArrayElement"
external set_char_array_element: obj -> int -> int -> unit
        = "camljava_SetCharArrayElement"
external new_short_array: int -> obj
        = "camljava_NewShortArray"
external get_short_array_element: obj -> int -> int
        = "camljava_GetShortArrayElement"
external set_short_array_element: obj -> int -> int -> unit
        = "camljava_SetShortArrayElement"
external new_int_array: int -> obj
        = "camljava_NewIntArray"
external get_int_array_element: obj -> int -> int32
        = "camljava_GetIntArrayElement"
external set_int_array_element: obj -> int -> int32 -> unit
        = "camljava_SetIntArrayElement"
external get_camlint_array_element: obj -> int -> int
        = "camljava_GetCamlintArrayElement"
external set_camlint_array_element: obj -> int -> int -> unit
        = "camljava_SetCamlintArrayElement"
external new_long_array: int -> obj
        = "camljava_NewLongArray"
external get_long_array_element: obj -> int -> int64
        = "camljava_GetLongArrayElement"
external set_long_array_element: obj -> int -> int64 -> unit
        = "camljava_SetLongArrayElement"
external new_float_array: int -> obj
        = "camljava_NewFloatArray"
external get_float_array_element: obj -> int -> float
        = "camljava_GetFloatArrayElement"
external set_float_array_element: obj -> int -> float -> unit
        = "camljava_SetFloatArrayElement"
external new_double_array: int -> obj
        = "camljava_NewDoubleArray"
external get_double_array_element: obj -> int -> float
        = "camljava_GetDoubleArrayElement"
external set_double_array_element: obj -> int -> float -> unit
        = "camljava_SetDoubleArrayElement"

(* Auxiliaries for Java->OCaml callbacks *)

val wrap_object: < .. > -> obj

