import java.util.Scanner;


public class Main {
    static Network network = new Network("http://127.0.0.1:8000");
    public static void main(String[] args) {
        var in = new Scanner(System.in);
        while (true) {
            System.out.print(">sql ");
            var line = in.nextLine();
            var cmd = CommandFactory.build(line);
            if (cmd != null) {
                var type = cmd.execute();
                switch (type) {
                    case SUCCESS:
                        System.out.println("success");
                        break;
                    default:
                        System.out.println(Command.map.get(type));
                }
            } else {
                System.out.println("empty command");
            }
        }
    }
}