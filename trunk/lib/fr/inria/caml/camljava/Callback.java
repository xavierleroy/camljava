package fr.inria.caml.camljava;

public class Callback {
    public Callback(long obj) { objref = obj; }

    protected void finalize() { freeWrapper(objref); }

    public native static long getCamlMethodID(String method_name);

    public void callVoid(long methid, Object args[])
    { callbackVoid(objref, methid, args); }

    public boolean callBoolean(long methid, Object args[])
    { return callbackBoolean(objref, methid, args); }

    public byte callByte(long methid, Object args[])
    { return callbackByte(objref, methid, args); }

    public char callChar(long methid, Object args[])
    { return callbackChar(objref, methid, args); }

    public short callShort(long methid, Object args[])
    { return callbackShort(objref, methid, args); }

    public int callCamlint(long methid, Object args[])
    { return callbackCamlint(objref, methid, args); }

    public int callInt(long methid, Object args[])
    { return callbackInt(objref, methid, args); }

    public long callLong(long methid, Object args[])
    { return callbackLong(objref, methid, args); }

    public float callFloat(long methid, Object args[])
    { return callbackFloat(objref, methid, args); }

    public double callDouble(long methid, Object args[])
    { return callbackDouble(objref, methid, args); }

    public Object callObject(long methid, Object args[])
    { return callbackObject(objref, methid, args); }

    private long objref;
    private native static
        void callbackVoid(long obj, long methid, Object args[]);
    private native static
        boolean callbackBoolean(long obj, long methid, Object args[]);
    private native static
        byte callbackByte(long obj, long methid, Object args[]);
    private native static
        char callbackChar(long obj, long methid, Object args[]);
    private native static
        short callbackShort(long obj, long methid, Object args[]);
    private native static
        int callbackCamlint(long obj, long methid, Object args[]);
    private native static
        int callbackInt(long obj, long methid, Object args[]);
    private native static
        long callbackLong(long obj, long methid, Object args[]);
    private native static
        float callbackFloat(long obj, long methid, Object args[]);
    private native static
        double callbackDouble(long obj, long methid, Object args[]);
    private native static
        Object callbackObject(long obj, long methid, Object args[]);
    private native static void freeWrapper(long obj);
}
