package fr.inria.caml.camljava;

import java.util.Vector;
import java.io.*;
import java.util.jar.*;
import java.util.HashSet;

public class Readclass {

  public static void main(String argv[])
  {
    Vector path = new Vector(10);
    String cmdline;

    addPath(path, System.getProperty("sun.boot.class.path"));
    addPath(path, System.getProperty("java.class.path"));
    while (true) {
      try {
        cmdline = readLine(System.in);
        if (cmdline == null) return;
        if (cmdline.length() < 1) continue;
        switch (cmdline.charAt(0)) {
        case '?':
          System.out.write(1);
          break;
        case 'Q':
          return;
        case 'R':
          readClass(path, cmdline.substring(1));
          System.err.println("Done R");
          break;
        case 'P':
          readPackage(path, cmdline.substring(1));
          System.err.println("Done P");
          break;
        }
        System.out.flush();
      } catch (IOException e) {
        /*nothing*/;
      }
    }
  }

  private static void addPath(Vector pathvect, String path)
  {
    if (path == null) return;
    for (int i = 0; i < path.length(); /*nothing*/) {
      int j = path.indexOf(java.io.File.pathSeparatorChar, i);
      if (j == -1) {
        addElementToPath(pathvect, path.substring(i));
        break;
      } else {
        addElementToPath(pathvect, path.substring(i, j));
        i = j + 1;
      }
    }
  }
  
  private static void addElementToPath(Vector pathvect, String element)
  {
    if (! pathvect.contains(element)) pathvect.add(element);
  }
  
  private static void readClass(Vector path, String classname)
  {
    for (int i = path.size() - 1; i >= 0; i--) {
      try {
        File pathcomp = new File((String) (path.elementAt(i)));
        if (!pathcomp.exists()) continue;
        if (pathcomp.isDirectory()) {
          File f =
            new File(pathcomp,
                     classname.replace('/', File.separatorChar) + ".class");
          if (! f.exists()) continue;
          byte [] data = readFile(f);
          System.out.write(1);
          System.out.write(data);
          return;
        }
        else if (pathcomp.isFile()) {
          JarFile jf = new JarFile(pathcomp);
          JarEntry je = jf.getJarEntry(classname + ".class");
          if (je == null) { jf.close(); continue; }
          byte [] data = readStream(jf.getInputStream(je), je.getSize());
          jf.close();
          System.out.write(1);
          System.out.write(data);
          return;
        }
      } catch (IOException e) {
        /*nothing*/;
      }
    }
    System.out.write(0);
  }

  private static void readPackage(Vector path, String packagename)
  {
    HashSet classes = new HashSet();
    for (int i = path.size() - 1; i >= 0; i--) {
      try {
        File pathcomp = new File((String) (path.elementAt(i)));
        if (!pathcomp.exists()) continue;
        if (pathcomp.isDirectory()) {
          File d = new File(pathcomp,
                            packagename.replace('/', File.separatorChar));
          String [] contents = d.list();
          if (contents == null) continue;
          for (int j = 0; j < contents.length; j++) {
            String f = contents[i];
            if (! f.endsWith(".class")) continue;
            if (! classes.add(packagename + '/' + f)) continue;
            byte [] data = readFile(new File(d, f));
            System.out.write(1);
            System.out.write(data);
          }
        }
        else if (pathcomp.isFile()) {
          JarInputStream js =
            new JarInputStream(new FileInputStream(pathcomp));
          JarEntry je;
          while ((je = js.getNextJarEntry()) != null) {
            String entryname = je.getName();
            // System.err.println(entryname);
            int lastslash = entryname.lastIndexOf('/');
            if (lastslash == -1) continue;
            if (! packagename.equals(entryname.substring(0, lastslash)))
              continue;
            if (! entryname.endsWith(".class")) continue;
            if (! classes.add(entryname)) continue;
            byte [] data = readStream(js, je.getSize());
            System.out.write(1);
            System.out.write(data);
          }
          js.close();
        }
      } catch (IOException e) {
        /*nothing*/;
      }
    }
    System.out.write(0);
  }

  private static byte[] readFile(File f) throws IOException
  {
    FileInputStream s = new FileInputStream(f);
    try {
      return readStream(s, f.length());
    } finally {
      s.close();
    }
  }

  private static byte[] readStream(InputStream s, long length)
  throws IOException
  {
    int len = (int) length;
    byte[] buffer = new byte[len];
    for (int i = 0; i < len; /*nothing*/) {
      int nread = s.read(buffer, i, len - i);
      i += nread;
    }
    return buffer;
  }

  private static String readLine(InputStream s) throws IOException
  {
    StringBuffer sb = new StringBuffer();
    while (true) {
      int i = s.read();
      if (i == -1) return null;
      System.err.write(i);  System.err.flush();
      if (i == 10) return sb.toString();
      sb.append((char) i);
    }
  }
}

      
