                  CamlJava - an OCaml/Java interface
                  ==================================

DESCRIPTION:

This is a very preliminary release of CamlJava, an OCaml/Java
interface based on the following schema:

         Caml/C interface       JNI (Java Native Interface)
  Caml <------------------> C <-----------------------------> Java

Currently, CamlJava provides a low-level, weakly-typed OCaml interface 
very similar to the JNI.  Java object references are mapped to an
abstract type, and various JNI-like operations are provided to allow
Java method invocation, field access, and more.  A basic callback
facility (allowing Java code to invoke methods on Caml objects) is
also provided, although some stub Java code must be written by hand.

In the future, a higher-level, strongly-typed interface will be
provided, whereas Java classes are mapped directly to Caml classes.
This raises fairly delicate type mapping issues, though, so don't hold
your breath.


REQUIREMENTS:

- This release of CamlJava requires Objective Caml version 3.08 or later.

- A Java implementation that supports JNI (Java Native Interface).
  So far, only Sun's JDK has been tested.

INSTALLATION ON A UNIX PLATFORM:

- Edit Makefile.config to define parameters depending on your Java
  installation.  As distributed, the library is set up for
  Sun's JDK version 1.4.1 on a Linux x86 platform.

- make
  become superuser
  make install

- For testing:
  make tst


INSTALLATION ON A WINDOWS PLATFORM:

- Works with the MSVC port of OCaml for Windows.  
  GNU make and Cygwin tools are required to do the installation.

- Edit Makefile.config.msvc to define parameters depending on your Java
  installation.  

- make -f Makefile.msvc
  make -f Makefile.msvc install

- For testing:
  make -f Makefile.msvc tst


USAGE:

The module is named "Jni".  A good knowledge of the JNI is assumed; see Sun's
JNI book or http://java.sun.com/products/jdk/1.2/docs/guide/jni/
Then, the comments in lib/jni.mli should make sense.

Usage:          ocamlc -I +camljava jni.cma ...
            or  ocamlopt -I +camljava jni.cmxa ...

See the programs in test/ for examples of use.


LICENSE:  GNU Library General Public License version 2.


FEEDBACK:  e-mail the author, Xavier.Leroy@inria.fr.



