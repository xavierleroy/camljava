#include <stddef.h>
#include <string.h>
#include <jni.h>
#include <caml/mlvalues.h>
#include <caml/memory.h>
#include <caml/alloc.h>
#include <caml/custom.h>
#include <caml/fail.h>
#include <caml/callback.h>

static JavaVM * jvm;
static JNIEnv * jenv;

#define Val_jboolean(b) ((b) == JNI_FALSE ? Val_false : Val_true)
#define Jboolean_val(v) (Val_bool(v) ? JNI_TRUE : JNI_FALSE)

/********** Threading *************/

static JNIEnv * g_jenv;

void init_threading() {
  g_jenv = jenv;
}

void camljava_check_main_thread() {
  if(jenv != g_jenv) {
    fprintf(stderr, "CamlJava: callbacks from threads other than main are not allowed: ABORT.\n");
    fflush(stderr);
    // raise a Java exception would be better
    exit(2);
  }
  return;
}

/************ Wrapping of Java objects as Caml values *************/

#define JObject(v) (*((jobject *) Data_custom_val(v)))

static void finalize_jobject(value v)
{
  jobject obj = JObject(v);
  if (obj != NULL) (*jenv)->DeleteGlobalRef(jenv, obj);
}

static struct custom_operations jobject_ops = {
  "java.lang.Object",
  finalize_jobject,
  custom_compare_default,       /* TODO? call equals() or compareTo() */
  custom_hash_default,          /* TODO? call hashCode() */
  custom_serialize_default,     /* TODO? use Java serialization intf */
  custom_deserialize_default    /* TODO? use Java serialization intf */
};

static value alloc_jobject(jobject obj)
{
  value v = alloc_custom(&jobject_ops, sizeof(jobject), 0, 1);
  if (obj != NULL) {
    obj = (*jenv)->NewGlobalRef(jenv, obj);
    if (obj == NULL) raise_out_of_memory();
  }
  JObject(v) = obj;
  return v;
}

value camljava_GetNull(value unit)
{
  return alloc_jobject(NULL);
}

value camljava_IsNull(value vobj)
{
  return Val_bool(JObject(vobj) == NULL);
}

/*********** Reflecting Java exceptions as Caml exceptions *************/

static int debug = 0;

value camljava_set_debug(value v) {
  debug = Bool_val(v);
  return Val_unit;
}

static void check_java_exception(void)
{
  jthrowable exn;
  value vobj;
  static value * camljava_raise_exception = NULL;

  exn = (*jenv)->ExceptionOccurred(jenv);
  if (exn != NULL) {
    if(debug) {
      /* For debugging */
      (*jenv)->ExceptionDescribe(jenv);
    }
    (*jenv)->ExceptionClear(jenv);
    /* TODO: check Caml exception embedded into Java exception */
    if (camljava_raise_exception == NULL) {
      camljava_raise_exception = caml_named_value("camljava_raise_exception");
      if (camljava_raise_exception == NULL)
        invalid_argument("Java_lang not linked in");
    }
    vobj = alloc_jobject(exn);
    (*jenv)->DeleteLocalRef(jenv, exn);
    callback(*camljava_raise_exception, vobj);
  }
}

static void check_non_null(value jobj)
{
  static value * camljava_null_pointer;
  if (JObject(jobj) != NULL) return;
  if (camljava_null_pointer == NULL) {
    camljava_null_pointer = caml_named_value("camljava_null_pointer");
    if (camljava_null_pointer == NULL)
      invalid_argument("Java not linked in");
  }
  raise_constant(*camljava_null_pointer);
}

/*********** Class operations ************/

value camljava_FindClass(value vname)
{
  jclass c = (*jenv)->FindClass(jenv, String_val(vname));
  if (c == NULL) check_java_exception();
  return alloc_jobject(c);
}

value camljava_GetSuperclass(value vclass)
{
  jclass c = (*jenv)->GetSuperclass(jenv, JObject(vclass));
  if (c == NULL) check_java_exception();
  return alloc_jobject(c);
}

value camljava_IsAssignableFrom(value vclass1, value vclass2)
{
  jboolean b =
    (*jenv)->IsAssignableFrom(jenv, JObject(vclass1), JObject(vclass2));
  return Val_jboolean(b);
}

/*********** Field IDs ***************/

#define JField(v) (*((jfieldID *) (v)))

static value alloc_jfieldID(jfieldID id)
{
  value v = alloc((sizeof(jfieldID) + sizeof(value) - 1) / sizeof(value),
                  Abstract_tag);
  JField(v) = id;
  return v;
}

value camljava_GetFieldID(value vclass, value vname, value vsig)
{
  jfieldID id = (*jenv)->GetFieldID(jenv, JObject(vclass),
                                    String_val(vname), String_val(vsig));
  if (id == NULL) check_java_exception();
  return alloc_jfieldID(id);
}

value camljava_GetStaticFieldID(value vclass, value vname, value vsig)
{
  jfieldID id = (*jenv)->GetStaticFieldID(jenv, JObject(vclass),
                                          String_val(vname), String_val(vsig));
  if (id == NULL) check_java_exception();
  return alloc_jfieldID(id);
}

/*********** Field access *************/

#define GETFIELD(name,restyp,resconv)                                       \
value camljava_##name(value vobj, value vfield)                             \
{                                                                           \
  restyp res;                                                               \
  check_non_null(vobj);                                                     \
  res = (*jenv)->name(jenv, JObject(vobj), JField(vfield));                 \
  return resconv(res);                                                      \
}

GETFIELD(GetObjectField, jobject, alloc_jobject)
GETFIELD(GetBooleanField, jboolean, Val_jboolean)
GETFIELD(GetByteField, jbyte, Val_int)
GETFIELD(GetCharField, jchar, Val_int)
GETFIELD(GetShortField, jshort, Val_int)
GETFIELD(GetIntField, jint, copy_int32)
GETFIELD(GetLongField, jlong, copy_int64)
GETFIELD(GetFloatField, jfloat, copy_double)
GETFIELD(GetDoubleField, jdouble, copy_double)

value camljava_GetCamlintField(value vobj, value vfield)
{
  jint res;
  check_non_null(vobj);
  res = (*jenv)->GetIntField(jenv, JObject(vobj), JField(vfield));
  return Val_int(res);
}

#define SETFIELD(name,argtyp,argconv)                                       \
value camljava_##name(value vobj, value vfield, value vnewval)              \
{                                                                           \
  argtyp arg = argconv(vnewval);                                            \
  check_non_null(vobj);                                                     \
  (*jenv)->name(jenv, JObject(vobj), JField(vfield), arg);                  \
  return Val_unit;                                                          \
}

SETFIELD(SetObjectField, jobject, JObject)
SETFIELD(SetBooleanField, jboolean, Jboolean_val)
SETFIELD(SetByteField, jbyte, Int_val)
SETFIELD(SetCharField, jchar, Int_val)
SETFIELD(SetShortField, jshort, Int_val)
SETFIELD(SetIntField, jint, Int32_val)
SETFIELD(SetLongField, jlong, Int64_val)
SETFIELD(SetFloatField, jfloat, Double_val)
SETFIELD(SetDoubleField, jdouble, Double_val)

value camljava_SetCamlintField(value vobj, value vfield, value vnewval)
{
  jint arg = Int_val(vnewval);
  check_non_null(vobj);
  (*jenv)->SetIntField(jenv, JObject(vobj), JField(vfield), arg);
  return Val_unit;
}

#define GETSTATICFIELD(name,restyp,resconv)                                   \
value camljava_##name(value vclass, value vfield)                             \
{                                                                             \
  restyp res = (*jenv)->name(jenv, JObject(vclass), JField(vfield));          \
  return resconv(res);                                                        \
}

GETSTATICFIELD(GetStaticObjectField, jobject, alloc_jobject)
GETSTATICFIELD(GetStaticBooleanField, jboolean, Val_jboolean)
GETSTATICFIELD(GetStaticByteField, jbyte, Val_int)
GETSTATICFIELD(GetStaticCharField, jchar, Val_int)
GETSTATICFIELD(GetStaticShortField, jshort, Val_int)
GETSTATICFIELD(GetStaticIntField, jint, copy_int32)
GETSTATICFIELD(GetStaticLongField, jlong, copy_int64)
GETSTATICFIELD(GetStaticFloatField, jfloat, copy_double)
GETSTATICFIELD(GetStaticDoubleField, jdouble, copy_double)

value camljava_GetStaticCamlintField(value vclass, value vfield)
{
  jint res = (*jenv)->GetStaticIntField(jenv, JObject(vclass), JField(vfield));
  return Val_int(res);
}

#define SETSTATICFIELD(name,argtyp,argconv)                                   \
value camljava_##name(value vclass, value vfield, value vnewval)              \
{                                                                             \
  argtyp arg = argconv(vnewval);                                              \
  (*jenv)->name(jenv, JObject(vclass), JField(vfield), arg);                  \
  return Val_unit;                                                            \
}

SETSTATICFIELD(SetStaticObjectField, jobject, JObject)
SETSTATICFIELD(SetStaticBooleanField, jboolean, Jboolean_val)
SETSTATICFIELD(SetStaticByteField, jbyte, Int_val)
SETSTATICFIELD(SetStaticCharField, jchar, Int_val)
SETSTATICFIELD(SetStaticShortField, jshort, Int_val)
SETSTATICFIELD(SetStaticIntField, jint, Int32_val)
SETSTATICFIELD(SetStaticLongField, jlong, Int64_val)
SETSTATICFIELD(SetStaticFloatField, jfloat, Double_val)
SETSTATICFIELD(SetStaticDoubleField, jdouble, Double_val)

value camljava_SetStaticCamlintField(value vclass, value vfield, value vnewval)
{
  jint arg = Val_int(vnewval);
  (*jenv)->SetStaticIntField(jenv, JObject(vclass), JField(vfield), arg);
  return Val_unit;
}

/*********** Method IDs ***************/

#define JMethod(v) (*((jmethodID *) (v)))

static value alloc_jmethodID(jmethodID id)
{
  value v = alloc((sizeof(jmethodID) + sizeof(value) - 1) / sizeof(value),
                  Abstract_tag);
  JMethod(v) = id;
  return v;
}

value camljava_GetMethodID(value vclass, value vname, value vsig)
{
  jmethodID id = (*jenv)->GetMethodID(jenv, JObject(vclass),
                                      String_val(vname), String_val(vsig));
  if (id == NULL) check_java_exception();
  return alloc_jmethodID(id);
}

value camljava_GetStaticMethodID(value vclass, value vname, value vsig)
{
  jmethodID id =
    (*jenv)->GetStaticMethodID(jenv, JObject(vclass),
                               String_val(vname), String_val(vsig));
  if (id == NULL) check_java_exception();
  return alloc_jmethodID(id);
}

/*************** The jvalue union ***************/

enum {
  Tag_Boolean,
  Tag_Byte,
  Tag_Char,
  Tag_Short,
  Tag_Camlint,
  Tag_Int,
  Tag_Long,
  Tag_Float,
  Tag_Double,
  Tag_Object
};

static void jvalue_val(value v, /*out*/ jvalue * j)
{
  switch (Tag_val(v)) {
  case Tag_Boolean:  j->z = Jboolean_val(Field(v, 0));
  case Tag_Byte:     j->b = Int_val(Field(v, 0)); break;
  case Tag_Char:     j->c = Int_val(Field(v, 0)); break;
  case Tag_Short:    j->s = Int_val(Field(v, 0)); break;
  case Tag_Camlint:  j->i = Int_val(Field(v, 0)); break;
  case Tag_Int:      j->i = Int32_val(Field(v, 0)); break;
  case Tag_Long:     j->j = Int64_val(Field(v, 0)); break;
  case Tag_Float:    j->f = Double_val(Field(v, 0)); break;
  case Tag_Double:   j->d = Double_val(Field(v, 0)); break;
  case Tag_Object:   j->l = JObject(Field(v, 0)); break;
  }
}

#define NUM_DEFAULT_ARGS 8

static jvalue * convert_args(value vargs, jvalue default_args[])
{
  mlsize_t nargs = Wosize_val(vargs);
  jvalue * args;
  mlsize_t i;

  if (nargs <= NUM_DEFAULT_ARGS)
    args = default_args;
  else
    args = stat_alloc(nargs * sizeof(jvalue));
  for (i = 0; i < nargs; i++) jvalue_val(Field(vargs, i), &(args[i]));
  return args;
}

/************* Method invocation **************/

#define CALLMETHOD(callname,restyp,resconv)                                 \
value camljava_##callname(value vobj, value vmeth, value vargs)             \
{                                                                           \
  jvalue default_args[NUM_DEFAULT_ARGS];                                    \
  jvalue * args;                                                            \
  restyp res;                                                               \
  check_non_null(vobj);                                                     \
  args = convert_args(vargs, default_args);                                 \
  res = (*jenv)->callname##A(jenv, JObject(vobj), JMethod(vmeth), args);    \
  if (args != default_args) stat_free(args);                                \
  check_java_exception();                                                   \
  return resconv(res);                                                      \
}

CALLMETHOD(CallObjectMethod, jobject, alloc_jobject)
CALLMETHOD(CallBooleanMethod, jboolean, Val_jboolean)
CALLMETHOD(CallByteMethod, jbyte, Val_int)
CALLMETHOD(CallCharMethod, jchar, Val_int)
CALLMETHOD(CallShortMethod, jshort, Val_int)
CALLMETHOD(CallIntMethod, jint, copy_int32)
CALLMETHOD(CallLongMethod, jlong, copy_int64)
CALLMETHOD(CallFloatMethod, jfloat, copy_double)
CALLMETHOD(CallDoubleMethod, jdouble, copy_double)

value camljava_CallCamlintMethod(value vobj, value vmeth, value vargs)
{
  jvalue default_args[NUM_DEFAULT_ARGS];
  jvalue * args;
  jint res;
  check_non_null(vobj);
  args = convert_args(vargs, default_args);
  res = (*jenv)->CallIntMethodA(jenv, JObject(vobj), JMethod(vmeth), args);
  if (args != default_args) stat_free(args);
  check_java_exception();
  return Val_int(res);
}

value camljava_CallVoidMethod(value vobj, value vmeth, value vargs)
{
  jvalue default_args[NUM_DEFAULT_ARGS];
  jvalue * args;
  check_non_null(vobj);
  args = convert_args(vargs, default_args);
  (*jenv)->CallVoidMethodA(jenv, JObject(vobj), JMethod(vmeth), args);
  if (args != default_args) stat_free(args);
  check_java_exception();
  return Val_unit;
}

#define CALLSTATICMETHOD(callname,restyp,resconv)                           \
value camljava_##callname(value vclass, value vmeth, value vargs)           \
{                                                                           \
  jvalue default_args[NUM_DEFAULT_ARGS];                                    \
  jvalue * args = convert_args(vargs, default_args);                        \
  restyp res =                                                              \
    (*jenv)->callname##A(jenv, JObject(vclass), JMethod(vmeth), args);      \
  if (args != default_args) stat_free(args);                                \
  check_java_exception();                                                   \
  return resconv(res);                                                      \
}

CALLSTATICMETHOD(CallStaticObjectMethod, jobject, alloc_jobject)
CALLSTATICMETHOD(CallStaticBooleanMethod, jboolean, Val_int)
CALLSTATICMETHOD(CallStaticByteMethod, jbyte, Val_int)
CALLSTATICMETHOD(CallStaticCharMethod, jchar, Val_int)
CALLSTATICMETHOD(CallStaticShortMethod, jshort, Val_int)
CALLSTATICMETHOD(CallStaticIntMethod, jint, copy_int32)
CALLSTATICMETHOD(CallStaticLongMethod, jlong, copy_int64)
CALLSTATICMETHOD(CallStaticFloatMethod, jfloat, copy_double)
CALLSTATICMETHOD(CallStaticDoubleMethod, jdouble, copy_double)

value camljava_CallStaticCamlintMethod(value vclass, value vmeth, value vargs)
{
  jvalue default_args[NUM_DEFAULT_ARGS];
  jvalue * args = convert_args(vargs, default_args);
  jint res =
    (*jenv)->CallStaticIntMethodA(jenv, JObject(vclass), JMethod(vmeth), args);
  if (args != default_args) stat_free(args);
  check_java_exception();
  return Val_int(res);
}

value camljava_CallStaticVoidMethod(value vclass, value vmeth, value vargs)
{
  jvalue default_args[NUM_DEFAULT_ARGS];
  jvalue * args = convert_args(vargs, default_args);
  (*jenv)->CallStaticVoidMethodA(jenv, JObject(vclass), JMethod(vmeth), args);
  if (args != default_args) stat_free(args);
  check_java_exception();
  return Val_unit;
}

#define CALLNONVIRTUALMETHOD(callname,restyp,resconv)                       \
value camljava_##callname(value vobj, value vclass, value vmeth, value vargs)\
{                                                                           \
  jvalue default_args[NUM_DEFAULT_ARGS];                                    \
  jvalue * args;                                                            \
  restyp res;                                                               \
  check_non_null(vobj);                                                     \
  args = convert_args(vargs, default_args);                                 \
  res = (*jenv)->callname##A(jenv, JObject(vobj), JObject(vclass),          \
                             JMethod(vmeth), args);                         \
  if (args != default_args) stat_free(args);                                \
  check_java_exception();                                                   \
  return resconv(res);                                                      \
}

CALLNONVIRTUALMETHOD(CallNonvirtualObjectMethod, jobject, alloc_jobject)
CALLNONVIRTUALMETHOD(CallNonvirtualBooleanMethod, jboolean, Val_int)
CALLNONVIRTUALMETHOD(CallNonvirtualByteMethod, jbyte, Val_int)
CALLNONVIRTUALMETHOD(CallNonvirtualCharMethod, jchar, Val_int)
CALLNONVIRTUALMETHOD(CallNonvirtualShortMethod, jshort, Val_int)
CALLNONVIRTUALMETHOD(CallNonvirtualIntMethod, jint, copy_int32)
CALLNONVIRTUALMETHOD(CallNonvirtualLongMethod, jlong, copy_int64)
CALLNONVIRTUALMETHOD(CallNonvirtualFloatMethod, jfloat, copy_double)
CALLNONVIRTUALMETHOD(CallNonvirtualDoubleMethod, jdouble, copy_double)

value camljava_CallNonvirtualCamlintMethod(value vobj, value vclass,
                                           value vmeth, value vargs)
{
  jvalue default_args[NUM_DEFAULT_ARGS];
  jvalue * args;
  jint res;
  check_non_null(vobj);
  args = convert_args(vargs, default_args);
  res = (*jenv)->CallNonvirtualIntMethodA(jenv, JObject(vobj), JObject(vclass),
                                          JMethod(vmeth), args);
  if (args != default_args) stat_free(args);
  check_java_exception();
  return Val_int(res);
}

value camljava_CallNonvirtualVoidMethod(value vobj, value vclass,
                                        value vmeth, value vargs)
{
  jvalue default_args[NUM_DEFAULT_ARGS];
  jvalue * args;
  check_non_null(vobj);
  args = convert_args(vargs, default_args);
  (*jenv)->CallNonvirtualVoidMethodA(jenv, JObject(vobj), JObject(vclass),
                                     JMethod(vmeth), args);
  if (args != default_args) stat_free(args);
  check_java_exception();
  return Val_unit;
}

/************** Strings ********************/

/* Note: by lack of wide strings in Caml, we map Java strings to
   UTF8-encoded Caml strings */

static value camljava_null_string;

value camljava_RegisterNullString(value null_string)
{
  camljava_null_string = null_string;
  register_global_root(&camljava_null_string);
  return Val_unit;
}

value camljava_MakeJavaString (value vstr)
{
  jstring jstr;
  if (vstr == camljava_null_string)
    jstr = NULL;
  else {
    jstr = (*jenv)->NewStringUTF(jenv, String_val(vstr));
    if (jstr == NULL) check_java_exception();
  }
  return alloc_jobject(jstr);
}

static value extract_java_string (JNIEnv * env, jstring jstr)
{
  jsize len;
  value res;
  jboolean isCopy;
  const char * chrs;

  if (jstr == NULL) return camljava_null_string;
  len = (*env)->GetStringUTFLength(env, jstr);
  res = alloc_string(len);
  chrs = (*env)->GetStringUTFChars(env, jstr, &isCopy);
  memcpy(String_val(res), chrs, len);
  (*env)->ReleaseStringUTFChars(env, jstr, chrs);
  return res;
}

value camljava_ExtractJavaString (value vobj)
{
  value res;

  Begin_root(vobj)              /* prevent deallocation of Java string */
    res = extract_java_string(jenv, (jstring) JObject(vobj));
  End_roots();
  return res;
}

/******************** Arrays *******************/

value camljava_GetArrayLength(value varray)
{
  jsize len;
  check_non_null(varray);
  len = (*jenv)->GetArrayLength(jenv, (jarray) JObject(varray));
  return Val_int(len);
}

value camljava_NewObjectArray(value vsize, value vclass)
{
  jobjectArray arr =
    (*jenv)->NewObjectArray(jenv, Int_val(vsize), 
                           (jclass) JObject(vclass), NULL);
  if (arr == NULL) check_java_exception();
  return alloc_jobject(arr);
}

value camljava_GetObjectArrayElement(value varray, value vidx)
{
  jobject res;
  check_non_null(varray);
  res = (*jenv)->GetObjectArrayElement(jenv, (jobjectArray) JObject(varray),
                                       Int_val(vidx));
  check_java_exception();
  return alloc_jobject(res);
}

value camljava_SetObjectArrayElement(value varray, value vidx, value vnewval)
{
  check_non_null(varray);
  (*jenv)->SetObjectArrayElement(jenv, (jobjectArray) JObject(varray),
                                Int_val(vidx), JObject(vnewval));
  check_java_exception();
  return Val_unit;
}

#define ARRAYNEWGETSET(name,array_typ,elt_typ,from_value,to_value)            \
value camljava_New##name##Array(value vsize)                                  \
{                                                                             \
  array_typ arr = (*jenv)->New##name##Array(jenv, Int_val(vsize));            \
  if (arr == NULL) check_java_exception();                                    \
  return alloc_jobject(arr);                                                  \
}                                                                             \
                                                                              \
value camljava_Get##name##ArrayElement(value varray, value vidx)              \
{                                                                             \
  elt_typ elt;                                                                \
  check_non_null(varray);                                                     \
  (*jenv)->Get##name##ArrayRegion(jenv, (array_typ) JObject(varray),          \
                                  Int_val(vidx), 1, &elt);                    \
  check_java_exception();                                                     \
  return to_value(elt);                                                       \
}                                                                             \
                                                                              \
value camljava_Set##name##ArrayElement(value varray, value vidx,              \
                                       value vnewval)                         \
{                                                                             \
  elt_typ elt;                                                                \
  check_non_null(varray);                                                     \
  elt = from_value(vnewval);                                                  \
  (*jenv)->Set##name##ArrayRegion(jenv, (array_typ) JObject(varray),          \
                                  Int_val(vidx), 1, &elt);                    \
  check_java_exception();                                                     \
  return Val_unit;                                                            \
}

ARRAYNEWGETSET(Boolean, jbooleanArray, jboolean, Jboolean_val, Val_jboolean)
ARRAYNEWGETSET(Byte, jbyteArray, jbyte, Int_val, Val_int)
ARRAYNEWGETSET(Char, jcharArray, jchar, Int_val, Val_int)
ARRAYNEWGETSET(Short, jshortArray, jshort, Int_val, Val_int)
ARRAYNEWGETSET(Int, jintArray, jint, Int32_val, copy_int32)
ARRAYNEWGETSET(Long, jlongArray, jlong, Int64_val, copy_int64)
ARRAYNEWGETSET(Float, jfloatArray, jfloat, Double_val, copy_double)
ARRAYNEWGETSET(Double, jdoubleArray, jdouble, Double_val, copy_double)

value camljava_GetCamlintArrayElement(value varray, value vidx)
{
  jint elt;
  check_non_null(varray);
  (*jenv)->GetIntArrayRegion(jenv, (jintArray) JObject(varray),
                             Int_val(vidx), 1, &elt);
  check_java_exception();
  return Val_int(elt);
}

value camljava_SetCamlintArrayElement(value varray, value vidx,
                                  value vnewval)
{
  jint elt = Int_val(vnewval);
  check_non_null(varray);
  (*jenv)->SetIntArrayRegion(jenv, (jintArray) JObject(varray),
                             Int_val(vidx), 1, &elt);
  check_java_exception();
  return Val_unit;
}

value camljava_GetByteArrayRegion(value varray, value vsrcidx,
                                  value vstr, value vdstidx,
                                  value vlength)
{
  long srcidx = Long_val(vsrcidx);
  long dstidx = Long_val(vdstidx);
  long length = Long_val(vlength);

  check_non_null(varray);
  if (dstidx < 0 || length < 0 || dstidx + length > string_length(vstr))
    invalid_argument("Jni.get_byte_array_region");
  (*jenv)->GetByteArrayRegion(jenv, (jbyteArray) JObject(varray),
                              srcidx, length, (jbyte *) &Byte(vstr, dstidx));
  check_java_exception();
  return Val_unit;
}

value camljava_SetByteArrayRegion(value vstr, value vsrcidx,
                                  value varray, value vdstidx,
                                  value vlength)
{
  long srcidx = Long_val(vsrcidx);
  long dstidx = Long_val(vdstidx);
  long length = Long_val(vlength);

  check_non_null(varray);
  if (srcidx < 0 || length < 0 || srcidx + length > string_length(vstr))
    invalid_argument("Jni.set_byte_array_region");
  (*jenv)->SetByteArrayRegion(jenv, (jbyteArray) JObject(varray),
                              dstidx, length, (jbyte *) &Byte(vstr, srcidx));
  check_java_exception();
  return Val_unit;
}

/************************ Initialization *************************/

value camljava_Init(value vclasspath)
{
  JavaVMInitArgs vm_args;
  JavaVMOption options[1];
  int retcode;
  char * classpath;
  char * setclasspath = "-Djava.class.path=";

  /* Set the class path */
  classpath = 
    stat_alloc(strlen(setclasspath) + string_length(vclasspath) + 1);
  strcpy(classpath, setclasspath);
  strcat(classpath, String_val(vclasspath));
  options[0].optionString = classpath;
  vm_args.version = JNI_VERSION_1_2;
  vm_args.options = options;
  vm_args.nOptions = 1;
  vm_args.ignoreUnrecognized = 1;
  /* Load and initialize a Java VM, return a JNI interface pointer in env */
  retcode = JNI_CreateJavaVM(&jvm, (void **) &jenv, &vm_args);
  stat_free(classpath);
  if (retcode < 0) failwith("Java.init");
  init_threading(); // by O'Jacare
  return Val_unit;
}

value camljava_Shutdown(value unit)
{
  (*jvm)->DestroyJavaVM(jvm);
  return Val_unit;
}

/****************** Object operations ********************/

value camljava_AllocObject(value vclass)
{
  jobject res = (*jenv)->AllocObject(jenv, JObject(vclass));
  if (res == NULL) check_java_exception();
  return alloc_jobject(res);
}

value camljava_GetObjectClass(value vobj)
{
  jclass cls;
  check_non_null(vobj);
  cls = (*jenv)->GetObjectClass(jenv, JObject(vobj));
  if (cls == NULL) check_java_exception();
  return alloc_jobject(cls);
}

value camljava_IsInstanceOf(value vobj, value vclass)
{
  jboolean res = (*jenv)->IsInstanceOf(jenv, JObject(vobj), JObject(vclass));
  check_java_exception();
  return Val_jboolean(res);
}

value camljava_IsSameObject(value vobj1, value vobj2)
{
  return Val_jboolean((*jenv)->IsSameObject(jenv, JObject(vobj1),
                                            JObject(vobj2)));
}

/********************* Callback from Java to Caml ******************/

static jclass caml_boolean, caml_byte, caml_char, caml_short, caml_int,
  caml_camlint, caml_long, caml_float, caml_double,
  java_lang_string, caml_exception;
static jfieldID caml_boolean_contents, caml_byte_contents,
  caml_char_contents, caml_short_contents, caml_int_contents,
  caml_camlint_contents, caml_long_contents,
  caml_float_contents, caml_double_contents;
static int caml_classes_initialized = 0;

static int init_caml_classes(JNIEnv * env)
{
#define INIT_CAML_CLASS(cls,fld,cname,fsig)                                 \
    cls = (*env)->FindClass(env, cname);                                    \
    cls = (*env)->NewGlobalRef(env, cls);                                   \
    if (cls == NULL) return -1;                                             \
    fld = (*env)->GetFieldID(env, cls, "contents", fsig);                   \
    if (fld == NULL) return -1;

  INIT_CAML_CLASS(caml_boolean, caml_boolean_contents,
                  "fr/inria/caml/camljava/Boolean", "Z");
  INIT_CAML_CLASS(caml_byte, caml_byte_contents,
                  "fr/inria/caml/camljava/Byte", "B");
  INIT_CAML_CLASS(caml_char, caml_char_contents,
                  "fr/inria/caml/camljava/Char", "C");
  INIT_CAML_CLASS(caml_short, caml_short_contents,
                  "fr/inria/caml/camljava/Short", "S");
  INIT_CAML_CLASS(caml_int, caml_int_contents,
                  "fr/inria/caml/camljava/Int", "I");
  INIT_CAML_CLASS(caml_camlint, caml_camlint_contents,
                  "fr/inria/caml/camljava/Camlint", "I");
  INIT_CAML_CLASS(caml_long, caml_long_contents,
                  "fr/inria/caml/camljava/Long", "J");
  INIT_CAML_CLASS(caml_float, caml_float_contents,
                  "fr/inria/caml/camljava/Float", "F");
  INIT_CAML_CLASS(caml_double, caml_double_contents,
                  "fr/inria/caml/camljava/Double", "D");
  java_lang_string = (*env)->FindClass(env, "java/lang/String");
  if (java_lang_string == NULL) return -1;
  caml_exception = (*env)->FindClass(env, "fr/inria/caml/camljava/Exception");
  if (caml_exception == NULL) return -1;
  return 0;
#undef INIT_CAML_CLASS
}

#define CALLBACK_OUT_OF_MEMORY Make_exception_result(0)

static value camljava_callback(JNIEnv * env,
                               jlong obj_proxy,
                               jlong method_id,
                               jobjectArray jargs)
{
  JNIEnv * savedenv;
  int n, i;
  value * cargs;
  jobject arg;
  value carg, clos, res;

  savedenv = jenv;
  jenv = env;

  camljava_check_main_thread(); // by O'Jacare

  if (!caml_classes_initialized) {
    if (init_caml_classes(env) == -1) return -1;
    caml_classes_initialized = 1;
  }
  n = 1 + (*env)->GetArrayLength(env, jargs);
  cargs = malloc(n * sizeof(value));
  if (cargs == NULL) {
    (*env)->ThrowNew(env,
                     (*env)->FindClass(env, "java/lang/OutOfMemoryError"),
                     "Out of memory in Java->Caml callback");
    jenv = savedenv;
    return CALLBACK_OUT_OF_MEMORY;
  }
  cargs[0] = *((value *) ((value) obj_proxy));
  for (i = 1; i < n; i++) cargs[i] = Val_unit;
  Begin_roots_block(cargs, n)
    for (i = 1; i < n; i++) {
      arg = (*env)->GetObjectArrayElement(env, jargs, i - 1);
      if (arg == NULL)
        carg = alloc_jobject(arg);
      else if ((*env)->IsInstanceOf(env, arg, caml_boolean))
        carg = Val_jboolean((*env)->GetBooleanField(env, arg,
                                                    caml_boolean_contents));
      else if ((*env)->IsInstanceOf(env, arg, caml_byte))
        carg = Val_int((*env)->GetByteField(env, arg, caml_byte_contents));
      else if ((*env)->IsInstanceOf(env, arg, caml_char))
        carg = Val_int((*env)->GetCharField(env, arg, caml_char_contents));
      else if ((*env)->IsInstanceOf(env, arg, caml_short))
        carg = Val_int((*env)->GetShortField(env, arg, caml_short_contents));
      else if ((*env)->IsInstanceOf(env, arg, caml_int))
        carg = copy_int32((*env)->GetIntField(env, arg, caml_int_contents));
      else if ((*env)->IsInstanceOf(env, arg, caml_camlint))
        carg = Val_int((*env)->GetIntField(env, arg, caml_camlint_contents));
      else if ((*env)->IsInstanceOf(env, arg, caml_long))
        carg = copy_int64((*env)->GetLongField(env, arg,
                                               caml_long_contents));
      else if ((*env)->IsInstanceOf(env, arg, caml_float))
        carg = copy_double((*env)->GetFloatField(env, arg,
                                                 caml_float_contents));
      else if ((*env)->IsInstanceOf(env, arg, caml_double))
        carg = copy_double((*env)->GetDoubleField(env, arg,
                                                caml_double_contents));
      else if ((*env)->IsInstanceOf(env, arg, java_lang_string))
        carg = extract_java_string(env, (jstring) arg);
      else
        carg = alloc_jobject(arg);
      cargs[i] = carg;
    }
  End_roots();
  clos = caml_get_public_method(cargs[0], (value) method_id);
  res = callbackN_exn(clos, n, cargs);
  free(cargs);
  jenv = savedenv;
  return res;
}

static void map_caml_exception(JNIEnv * env, value exn)
{
  value name;

  exn = Extract_exception(exn);
  name = Field(Field(exn, 0), 0);
  (*env)->ThrowNew(env, caml_exception, String_val(name));
}

void camljava_CallbackVoid(JNIEnv * env, jclass cls,
                           jlong obj_proxy, jlong method_id,
                           jobjectArray args)
{
  value res = camljava_callback(env, obj_proxy, method_id, args);
  if (Is_exception_result(res)) map_caml_exception(env, res);
}

#define CALLBACK(name,restyp,conv)                                          \
restyp camljava_Callback##name(JNIEnv * env, jclass cls,                    \
                                jlong obj_proxy, jlong method_id,           \
                                jobjectArray args)                          \
{                                                                           \
  value res = camljava_callback(env, obj_proxy, method_id, args);           \
  if (Is_exception_result(res)) {                                           \
    map_caml_exception(env, res);                                           \
    return 0; /*dummy return value*/                                        \
  } else                                                                    \
    return conv(res);                                                       \
}

CALLBACK(Boolean, jboolean, Jboolean_val)
CALLBACK(Byte, jbyte, Int_val)
CALLBACK(Char, jchar, Int_val)
CALLBACK(Short, jshort, Int_val)
CALLBACK(Camlint, jint, Int_val)
CALLBACK(Int, jint, Int32_val)
CALLBACK(Long, jlong, Int64_val)
CALLBACK(Float, jfloat, Double_val)
CALLBACK(Double, jdouble, Double_val)
CALLBACK(Object, jobject, JObject)

/****************** Auxiliary functions for callbacks *****************/

value camljava_WrapCamlObject(value vobj)
{
  value * wrapper = stat_alloc(sizeof(value));
  *wrapper = vobj;
  register_global_root(wrapper);
  return copy_int64((int64) (value) wrapper);
}

void camljava_FreeWrapper(JNIEnv * env, jclass cls, jlong wrapper)
{
  value * w = (value *) (value) wrapper;
  remove_global_root(w);
  stat_free(w);
}

jlong camljava_GetCamlMethodID(JNIEnv * env, jclass cls, jstring jname)
{
  jboolean isCopy;
  const char * chrs;
  value res;

  chrs = (*env)->GetStringUTFChars(env, jname, &isCopy);
  res = caml_hash_variant((char *) chrs);
  (*env)->ReleaseStringUTFChars(env, jname, chrs);
  return res;
}

/***************** Registration of native methods with the JNI ************/

static JNINativeMethod camljava_natives[] =
{ { "callbackVoid", "(JJ[Ljava/lang/Object;)V", (void*)camljava_CallbackVoid },
  { "callbackBoolean", "(JJ[Ljava/lang/Object;)Z", (void*)camljava_CallbackBoolean },
  { "callbackByte", "(JJ[Ljava/lang/Object;)B", (void*)camljava_CallbackByte },
  { "callbackChar", "(JJ[Ljava/lang/Object;)C", (void*)camljava_CallbackChar },
  { "callbackShort", "(JJ[Ljava/lang/Object;)S", (void*)camljava_CallbackShort },
  { "callbackCamlint", "(JJ[Ljava/lang/Object;)I", (void*)camljava_CallbackCamlint },
  { "callbackInt", "(JJ[Ljava/lang/Object;)I", (void*)camljava_CallbackInt },
  { "callbackLong", "(JJ[Ljava/lang/Object;)J", (void*)camljava_CallbackLong },
  { "callbackFloat", "(JJ[Ljava/lang/Object;)F", (void*)camljava_CallbackFloat },
  { "callbackDouble", "(JJ[Ljava/lang/Object;)D", (void*)camljava_CallbackDouble },
  { "callbackObject", "(JJ[Ljava/lang/Object;)Ljava/lang/Object;",
    (void*)camljava_CallbackObject },
  { "freeWrapper", "(J)V", (void*)camljava_FreeWrapper },
  { "getCamlMethodID", "(Ljava/lang/String;)J", (void*)camljava_GetCamlMethodID }
};

value camljava_RegisterNatives(value unit)
{
  jclass cls = (*jenv)->FindClass(jenv, "fr/inria/caml/camljava/Callback");
  if (cls == NULL) check_java_exception();
  (*jenv)->RegisterNatives(jenv, cls, camljava_natives,
                           sizeof(camljava_natives) / sizeof(JNINativeMethod));
  return Val_unit;
}

/********************* OS-specific hacks ************************/

#ifdef JDK122_LINUX_HACK

#include <setjmp.h>

extern void __libc_siglongjmp (sigjmp_buf env, int val)
     __attribute__ ((noreturn));

void siglongjmp(sigjmp_buf env, int val)
{
  __libc_siglongjmp(env, val);
}

#endif
