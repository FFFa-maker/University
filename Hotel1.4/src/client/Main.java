package client;

import java.io.*;
import java.net.InetAddress;
import java.net.Socket;
import java.net.SocketException;
import java.util.Objects;

public class Main {
    public static void main(String[] args)
    {
        String str;
        try
        {
            InetAddress addr = InetAddress.getByName("127.0.0.1");
            Socket socket = new Socket(addr, 10086);

            InputStream In = socket.getInputStream();
            OutputStream Out = socket.getOutputStream();
            InputStreamReader isr1 = new InputStreamReader(In);
            BufferedReader in = new BufferedReader(isr1);
            
            PrintWriter out = new PrintWriter(Out, true);

            InputStreamReader isr2 = new InputStreamReader(System.in);
            OutputStreamWriter osw = new OutputStreamWriter(System.out, "UTF-8");

            BufferedReader userin = new BufferedReader(isr2);
            PrintWriter userout = new PrintWriter(osw, true);

            while (true)
            {
                String line = null;
                while(!Objects.equals(line = in.readLine(), "###")){
                    userout.println(line);
                }
                str = userin.readLine();
                out.println(str);
            }
        }
        catch (SocketException e)
        {
            System.exit(0);
        }
        catch(IOException e){
            e.printStackTrace();
        }
    }
}

