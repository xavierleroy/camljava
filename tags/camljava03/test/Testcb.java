import fr.inria.caml.camljava.Callback;

class Testcb {
    private Callback cb;
    private static long _f = Callback.getCamlMethodID("f");
    private static long _g = Callback.getCamlMethodID("g");
    public Testcb(Callback c) { cb = c; }
    public void f()
    { Object[] args = { }; cb.callVoid(_f, args); }
    public int g(int x)
    { Object[] args = { new fr.inria.caml.camljava.Camlint(x) };
      return cb.callCamlint(_g, args); }
}
