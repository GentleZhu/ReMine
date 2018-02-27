package nlptools;

import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.nio.file.DirectoryStream;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.ArrayList;
import java.util.List;

class FileManager {
    private List<Path> files = new ArrayList<>();

    FileManager(Path path) throws IOException {
        listFiles(path);
    }

    List<Path> getFiles() {
        return this.files;
    }

    private void listFiles(Path path) throws IOException {
        try (DirectoryStream<Path> stream = Files.newDirectoryStream(path)) {
            for (Path entry : stream) {
                if (Files.isDirectory(entry)) {
                    listFiles(entry);
                }
                if (Files.isRegularFile(entry)) {
                    files.add(entry);
                }
            }
        }
    }

    static String readFile(Path path) throws IOException {
        InputStreamReader isr = new InputStreamReader(
                new FileInputStream(path.toFile()));
        int c;
        StringBuilder res = new StringBuilder();
        while ((c = isr.read()) != -1) {
            res.append((char) c);
        }
        isr.close();
        return res.toString().replace('�', '\'');
    }
}
