package server;

import java.util.ArrayList;
import java.util.Date;

public class Room {
    private Hotel hotel;
    private int ID;
    private String type;
    private int capacity;
    private boolean ifReserved;
    private ArrayList<TimePair> timePairs;

    public Room(Hotel hotel, int ID, String type){
        if(hotel.getTypeList().containsKey(type)){
            this.hotel = hotel;
            this.ID = ID;
            this.type = type;
            this.capacity = hotel.getTypeList().get(type);
            this.ifReserved = false;
            this.timePairs = new ArrayList<>();
            this.timePairs.add(new TimePair(new Date(), new Date()));
        }
    }

    public void setIfReserved(boolean ifReserved) {
        this.ifReserved = ifReserved;
    }

    public Hotel getHotel() {
        return hotel;
    }

    public int getID() {
        return ID;
    }

    public int getCapacity() {
        return capacity;
    }

    public String getType() {
        return type;
    }

    public boolean isIfReserved() {
        return ifReserved;
    }

    public ArrayList<TimePair> getTimePairs() {
        return timePairs;
    }
}
