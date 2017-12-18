package fr.inria.caml.camljava;

public class Caml {
	private Caml() {}

	/**
	 * OCaml startup function
	 * This function (or Jni.init on the OCaml side)
	 *  must be called before any other camljava functions
	 * Note: Java strings (with their custom UTF-8)
	 *  are passed to OCaml without convertion
	 */
	public static native void startup(String[] argv);
}
