package server;

import java.io.*;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.Date;

public class Main {
    static Hotel hotel = new Hotel();
    static People currentUser;
    static BufferedReader in;
    static PrintWriter out;
    static int rule = 1;


    public static void wrongCommand(){
        Main.out.println("WRONG COMMAND\n###");
        System.out.println("WRONG COMMAND");
    }

    public static void wrongParameter(){
        Main.out.println("WRONG PARAMETER\n###");
        System.out.println("WRONG PARAMETER");
    }

    public static void getCommand() throws IOException{
        try{
            String command = in.readLine();
            String[] commands = command.split(" ");
            if(commands[0].equals("create")){
                if(commands.length == 3){
                    if(currentUser instanceof Root){
                        ((Root) currentUser).addAdmin(commands[1], commands[2]);
                    }else{
                        hotel.createUser(commands[1], commands[2]);
                    }
                }
                else{
                    wrongParameter();
                }
            }
            else if(commands[0].equals("login")){
                if(commands.length == 3){
                    if(currentUser instanceof None){
                        hotel.login(commands[1], commands[2]);
                    }else{
                        out.println("FAIL\n###");
                    }
                }else{
                    wrongParameter();
                }
            }
            else if(commands[0].equals("logout")){
                if(commands.length == 1){
                    if(currentUser instanceof None){
                        out.println("FAIL\n###");
                    }else{
                        if(currentUser instanceof Admin){
                            ((Admin) currentUser).sort(rule);
                        }
                        currentUser = None.getInstance();
                        out.println("OK\n###");
                    }
                }else{
                    wrongParameter();
                }
            }
            else if(commands[0].equals("delete")){
                if(commands.length < 2||!commands[1].equals("admin")){
                    wrongCommand();
                }else if(commands.length == 3){
                    currentUser.deleteAdmin(commands[2]);
                }else{
                    wrongParameter();
                }
            }
            else if(commands[0].equals("addroomtype")){
                if(commands.length == 3){
                    try{
                        currentUser.addRoomType(commands[1], Integer.parseInt(commands[2]));
                    }catch (NumberFormatException e){
                        wrongParameter();
                    }
                }else{
                    wrongParameter();
                }
            }
            else if(commands[0].equals("addroom")){
                if(commands.length == 3){
                    currentUser.addRoom(Integer.parseInt(commands[1]), commands[2]);

                }
                else{
                    wrongParameter();
                }
            }
            else if(commands[0].equals("reserve_room")){
                if(commands.length == 8){
                    try{
                        int num = Integer.parseInt(commands[1]);
                        int sy = Integer.parseInt(commands[2]);
                        int sm = Integer.parseInt(commands[3]);
                        int sd = Integer.parseInt(commands[4]);
                        int ey = Integer.parseInt(commands[5]);
                        int em = Integer.parseInt(commands[6]);
                        int ed = Integer.parseInt(commands[7]);
                        currentUser.reserveRoom(num, new Date(sy - 1900, sm - 1, sd, 0, 0, 0), new Date(ey - 1900, em - 1, ed, 23, 59, 59), rule);
                    }catch (NumberFormatException e){
                        wrongParameter();
                    }
                }else{
                    wrongParameter();
                }
            }
            else if(commands[0].equals("show_reservation")){
                if(commands.length == 1){
                    if(currentUser instanceof Roomer){
                        currentUser.showReservation();
                    }else{
                        out.println("FAIL\n###");
                    }
                }
                else{
                    wrongParameter();
                }
            }
            else if(commands[0].equals("show_reservations")){
                if(commands.length == 1){
                    if(currentUser instanceof Admin){
                        currentUser.showReservation();
                    }else{
                        out.println("FAIL\n###");
                    }
                }else{
                    wrongParameter();
                }
            }
            else if(commands[0].equals("end")){
                if(commands.length == 1){
                    currentUser.end();
                }
                else{
                    wrongParameter();
                }
            }
            else if(commands[0].equals("set_rule")){
                if(commands.length == 2){
                    try{
                        currentUser.setRule(Integer.parseInt(commands[1]));
                    }catch (NumberFormatException e){
                        wrongParameter();
                    }
                }else{
                    wrongParameter();
                }
            }
            else{
                wrongCommand();
            }
        }catch(IOException e){
            e.printStackTrace();
            throw new IOException();
        }
    }

    public static void main(String[] args) {
        try
        {
            ServerSocket server = new ServerSocket(10086);
            Socket socket = server.accept();
            InputStream In = socket.getInputStream();
            OutputStream Out = socket.getOutputStream();
            InputStreamReader isr = new InputStreamReader(In);
            in = new BufferedReader(isr);
            out = new PrintWriter(Out, true);
            out.println("Server Connected, please enter command:\n###");
            currentUser = None.getInstance();
            while (true)
            {
                getCommand();
            }
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }
    }
}
