package server;

import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;

public class Roomer extends People{
    public Roomer(Hotel hotel, String name, String password){
        super(hotel, name, password);
    }

    @Override
    public int showReservation() {
        int i = 0;

        SimpleDateFormat sdf = new SimpleDateFormat("yyyy M dd");

        for(Order o: getHotel().getOrderList()){

            if(o.getName().equals(this.getName())){
                i++;
                StringBuilder sb = new StringBuilder();
                sb.append(o.getID()).append(" ").append(o.getName());
                for(Room r: o.getRoomlist()){
                    sb.append(" ").append(r.getID());
                }
                sb.append(" ").append(sdf.format(o.getStarttime())).append(" ").append(sdf.format(o.getEndtime()));
                Main.out.println(sb);
//                System.out.println(sb);
            }
        }
        if(i == 0){
            Main.out.println("none\n###");
//            System.out.println("无订单");
        }
        else{
            Main.out.println("###");
        }
        return 0;
    }

    @Override
    public boolean reserveRoom(int num, Date start, Date end, int rule) {
        if(start.after(end)){
            Main.out.println("FAIL\n###");
        }
        ArrayList<Room> rooms, roomsCanReserve;
        rooms = getHotel().getOrderRoomList();
        int all = num;
        roomsCanReserve = new ArrayList<>();
        for(int i = 0; i < rooms.size(); i++){
            Room r = rooms.get(i);
            if(!r.isIfReserved()){
                roomsCanReserve.add(r);
                all -= r.getCapacity();
            }else{
                for(int j = 0; j < r.getTimePairs().size(); j++){
                    TimePair tp = r.getTimePairs().get(j);
                    if(j == r.getTimePairs().size() - 1){
                        if(start.after(tp.getEnd())){
                            roomsCanReserve.add(r);
                            all -= r.getCapacity();
                        }
                    }else{
                        TimePair newttp = r.getTimePairs().get(j+1);
                        if(start.after(tp.getEnd()) && start.before(newttp.getStart())){
                            if(end.before(newttp.getStart())){
                                roomsCanReserve.add(r);
                                all -= r.getCapacity();
                            }
                            break;
                        }
                    }
                }
            }
        }
        if(all > 0){
            Main.out.println("FAIL\n###");
            return false;
        }

        ArrayList<Room> roomWillFill = makeOrder(num, roomsCanReserve);
        for(Room r: roomWillFill){
            r.setIfReserved(true);
            r.getTimePairs().add(new TimePair(start, end));
            r.getTimePairs().sort((u1, u2)->{
                if(u1.getStart().before(u2.getStart()))
                    return -1;
                else if(u1.getStart().after(u2.getStart()))
                    return 1;
                else
                    return 0;
            });
        }
        getHotel().getOrderList().add(new Order(this.getName(), num, start, end, roomWillFill));

        return true;
    }
    private ArrayList<Room> makeOrder(int num, ArrayList<Room> rooms){
        ArrayList<Room> roomdq = new ArrayList<>();
        ArrayList<Room> roomList = new ArrayList<>();
        do {
            dfs(rooms, 0, num, roomdq, roomList);
            num++;
        }while(roomList.size() == 0);
        return roomList;
    }
    private void dfs(ArrayList<Room> rooms, int begin, int num, ArrayList<Room> roomdq, ArrayList<Room> roomList){
        if(roomList.size()==0){
            if(num == 0){
                roomList.addAll(roomdq);
                return;
            }
            for(int i = begin; i < rooms.size(); i++){
                if(num - rooms.get(i).getCapacity() < 0){
                    continue;
                }
                roomdq.add(rooms.get(i));
                dfs(rooms, i + 1, num - rooms.get(i).getCapacity(), roomdq, roomList);
                roomdq.remove(roomdq.size()-1);
            }
        }
        return;
    }

}
