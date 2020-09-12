package server;

import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;

public class Order {
    private static int all;
    private final int ID;
    private final String name;
    private final ArrayList<Room> roomlist;
    private final int num;
    private final Date starttime;
    private final Date endtime;

    public Order(String name, int num, Date start, Date end, ArrayList<Room> room){
        all++;
        this.ID = all;
        this.name = name;
        this.num = num;
        this.starttime = start;
        this.endtime = end;
        this.roomlist = room;
        SimpleDateFormat sdf = new SimpleDateFormat("yyyy M dd");
        Main.out.print("订单号 " + ID
                + " 预定旅客名 "+ name
                +" 预定人数 " + num
                + " 预定入住日期 " + sdf.format(start)
                + " 预定退房日期 " + sdf.format(end)
                + " 预定房间号"
        );
        for(Room r:roomlist){
            Main.out.print(" " + r.getID());
        }
        Main.out.println("\n###");
    }

    public int getID() {
        return ID;
    }

    public String getName() {
        return name;
    }

    public int getNum() {
        return num;
    }

    public ArrayList<Room> getRoomlist() {
        return roomlist;
    }

    public Date getStarttime() {
        return starttime;
    }

    public Date getEndtime() {
        return endtime;
    }
}
