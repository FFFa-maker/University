package server;

import java.text.SimpleDateFormat;
import java.util.Date;

public class Admin extends People{
    public Admin(Hotel hotel, String name, String password){
        super(hotel,name,password);
    }

    @Override
    public boolean addRoom(int RoomID, String type) {
        if(RoomID < 101){
            Main.out.println("FAIL\n###");
        }
        boolean flag = true;
        for(Room r:getHotel().getOrderRoomList()){
            if(r.getID() == RoomID){
                flag = false;
                Main.out.println("FAIL\n###");
                break;
            }
        }
        getHotel().getOrderRoomList().add(new Room(getHotel(), RoomID, type));
        Main.out.println("OK\n###");
        return flag;
    }

    @Override
    public int showReservation() {
        int i = 0;
        SimpleDateFormat sdf = new SimpleDateFormat("yyyy M dd");
        for(Order o:getHotel().getOrderList()){
            i++;
            StringBuilder sb = new StringBuilder();
            sb.append(o.getID()).append(" ").append(o.getName());
            for(Room r:o.getRoomlist()){
                sb.append(" ").append(r.getID());
            }
            sb.append(" ").append(sdf.format(o.getStarttime())).append(" ").append(sdf.format(o.getEndtime()));
            Main.out.println(sb);
        }
        if(i == 0){
            Main.out.println("none\n###");
        }
        else{
            Main.out.println("###");
        }
        return i;
    }

    @Override
    public boolean setRule(int rule) {
        if(rule == 1||rule == 2){
            Main.rule = rule;
            Main.out.println("OK\n###");
            return true;
        }
        return false;
    }

    @Override
    public boolean addRoomType(String type, int capacity) {
        getHotel().getTypeList().put(type, capacity);
        Main.out.println("OK\n###");
        return false;
    }

    public void sort(int i){
        if(i == 2){
            getHotel().getOrderRoomList().sort((u1, u2)->{
                if(u1.getCapacity() > u2.getCapacity())
                    return -1;
                else if(u1.getCapacity() == u2.getCapacity()){
                    if(u1.getID() < u2.getID())
                        return -1;
                    else if(u1.getID() > u2.getID())
                        return 1;
                    else
                        return 0;
                }else{
                    return 1;
                }
            });
        }else{
            getHotel().getOrderRoomList().sort((u1, u2)->{
                if(u1.getCapacity() > u2.getCapacity())
                    return -1;
                else if(u1.getCapacity() == u2.getCapacity()){
                    if(u1.getID() < u2.getID())
                        return 1;
                    else if(u1.getID() > u2.getID())
                        return -1;
                    else
                        return 0;
                }else{
                    return 1;
                }
            });
        }
    }
}
