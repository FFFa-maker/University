package server;

import java.util.Date;

public class Root extends People{

    public Root(Hotel hotel){
        super(hotel, "root", "root");
    }

    @Override
    public boolean addAdmin(String admin, String password) {
        if(getHotel().getUserList().containsKey(admin)){
            Main.out.println("FAIL\n###");
            return false;
        }
        getHotel().getUserList().put(admin, new Admin(getHotel(),admin, password));
        Main.out.println("OK\n###");
        return true;
    }

    @Override
    public boolean deleteAdmin(String admin) {
        if(getHotel().getUserList().containsKey(admin)){
            getHotel().getUserList().remove(admin);
            Main.out.println("OK\n###");
            return true;
        }
        Main.out.println("FAIL\n###");
        return false;
    }

    @Override
    public void end() {
        System.exit(0);
    }
}
