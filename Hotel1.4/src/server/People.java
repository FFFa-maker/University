package server;

import java.util.Date;

public abstract class People implements AllCommand{
    private final Hotel hotel;
    private final String name;
    private final String password;

    public People(Hotel hotel, String name, String password){
        this.hotel = hotel;
        this.name = name;
        this.password = password;
    }

    public Hotel getHotel() {
        return hotel;
    }

    public String getName() {
        return name;
    }

    public String getPassword() {
        return password;
    }

    @Override
    public boolean addAdmin(String admin, String password) {
        Main.out.println("FAIL\n###");
        return false;
    }

    @Override
    public boolean deleteAdmin(String admin) {
        Main.out.println("FAIL\n###");
        return false;
    }

    @Override
    public boolean addRoom(int RoomID, String type) {
        Main.out.println("FAIL\n###");
        return false;
    }

    @Override
    public int showReservation() {
        Main.out.println("FAIL\n###");
        return 0;
    }

    @Override
    public boolean reserveRoom(int num, Date start, Date end, int rule) {
        Main.out.println("FAIL\n###");
        return false;
    }

    @Override
    public void end() {
        Main.out.println("FAIL\n###");
    }

    @Override
    public boolean setRule(int rule) {
        Main.out.println("FAIL\n###");
        return false;
    }

    @Override
    public boolean addRoomType(String type, int capacity) {
        Main.out.println("FAIL\n###");
        return false;
    }
}
