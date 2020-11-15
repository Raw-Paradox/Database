import java.util.Arrays;
import java.util.List;
import java.util.Map;

import static java.lang.System.exit;

interface Command {
    enum Type {
        SUCCESS,
        INVALID_CMD,
        INVALID_PARAM,
        UNKNOWN_ERROR,
    }

    Map<Type, String> map = Map.of(Type.SUCCESS, "SUCCESS",
            Type.INVALID_CMD, "INVALID_CMD",
            Type.INVALID_PARAM, "INVALID_PARAM",
            Type.UNKNOWN_ERROR, "UNKNOWN_ERROR");

    Type execute();

    List<Person> getData();
}



class CommandFactory {
    static Command build(String line) {
        if (line.isEmpty()) {
            return null;
        }
        if (line.charAt(0) == '.') {
            return new MetaCommand(line.substring(1));
        } else {
            return new CommonCommand(line);
        }
    }
}

class MetaCommand implements Command {
    //    static java.CommandFactory factory = java.MetaCommand::new;
    private enum Type {
        EXIT,
    }

    static Map<String, Type> typeMap = Map.of("exit", Type.EXIT);

    Type type;

    MetaCommand(String line) {
        type = typeMap.get(line);
    }

    @Override
    public Command.Type execute() {
        if (type == null) {
            return Command.Type.INVALID_CMD;
        }
        switch (type) {
            case EXIT:
                exit(0);
                break;
        }
        return Command.Type.UNKNOWN_ERROR;
    }

    @Override
    public List<Person> getData() {
        return null;
    }
}

// hard-coded database: Person(id int, username varchar(32), email varchar(255))

class CommonCommand implements Command {
    //    static java.CommandFactory factory = java.CommonCommand::new;
    private enum Type {
        INSERT,
    }

    static Map<String, Type> typeMap = Map.of("insert", Type.INSERT);

    Type type;
    List<String> args;

    CommonCommand(String line) {
        var array = Arrays.asList(line.split(" "));
        type = typeMap.get(array.get(0));
        args = array.subList(1, array.size());
    }

    @Override
    public Command.Type execute() {
        if (type == null) {
            return Command.Type.INVALID_CMD;
        }
        switch (type) {
            case INSERT:
                if (args.size() < 3 || args.get(1).length() > 32 || args.get(2).length() > 255) {
                    return Command.Type.INVALID_PARAM;
                }
                Main.network.insert(new Person(Integer.parseInt(args.get(0)), args.get(1), args.get(2)));
                System.out.printf("insert %d %s %s successfully\n", Integer.parseInt(args.get(0)), args.get(1), args.get(2));
                return Command.Type.SUCCESS;
        }
        return null;
    }

    @Override
    public List<Person> getData() {
        return null;
    }
}
