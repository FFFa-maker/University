package server;

import java.util.Date;

public class None extends People{
    private static None none = new None();
    private None() {
        super(Main.hotel, null, null);
    }
    public static None getInstance(){
        return none;
    }
}
