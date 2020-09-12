package server;

import java.util.ArrayList;
import java.util.HashMap;

public class Hotel {
    private String name;
    private final HashMap<String, People> userList;
    private final ArrayList<Room> orderRoomList;//房间号从小到大
    private final HashMap<String, Integer> typeList;
    private final ArrayList<Order> orderList;

    public Hotel(){
        name = "";
        userList = new HashMap<>();
        orderRoomList = new ArrayList<>();
        typeList = new HashMap<>();
        orderList = new ArrayList<>();
        userList.put("root", new Root(this));
    }

    public void setName(String name) {
        this.name = name;
    }

    public String getName() {
        return name;
    }

    public HashMap<String, People> getUserList() {
        return userList;
    }

    public ArrayList<Room> getOrderRoomList() {
        return orderRoomList;
    }


    public HashMap<String, Integer> getTypeList() {
        return typeList;
    }

    public ArrayList<Order> getOrderList() {
        return orderList;
    }

    public boolean createUser(String user, String password){
        if(userList.containsKey(user)){
            Main.out.println("FAIL\n###");
            return false;
        }else{
            userList.put(user, new Roomer(this, user, password));
            Main.out.println("OK\n###");
            return true;
        }
    }

    public boolean login(String name, String password){
        if(!userList.containsKey(name)){
            Main.out.println("FAIL\n###");
            return false;
        }else if(userList.get(name).getPassword().equals(password)){
            Main.currentUser = userList.get(name);
            Main.out.println("OK\n###");

            return true;
        }else{
            Main.out.println("FAIL\n###");
            return false;
        }
    }
}
