package server;

import java.util.Date;

public class TimePair {
    private final Date start;
    private final Date end;
    public TimePair(Date start, Date end){
        this.start = start;
        this.end = end;
    }
    public Date getStart() {
        return start;
    }

    public Date getEnd() {
        return end;
    }
}
