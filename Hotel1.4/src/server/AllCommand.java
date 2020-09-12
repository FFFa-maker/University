package server;

import java.util.Date;

public interface AllCommand {
    boolean addAdmin(String admin, String password);
    boolean deleteAdmin(String admin);
    boolean addRoom(int RoomID, String type);
    int showReservation();
    boolean reserveRoom(int num, Date start, Date end, int rule);
    void end();
    boolean setRule(int rule);
    boolean addRoomType(String type, int capacity);
}
