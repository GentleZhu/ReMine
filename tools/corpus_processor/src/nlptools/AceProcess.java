package nlptools;

import com.google.common.base.Joiner;
import edu.illinois.cs.cogcomp.core.datastructures.textannotation.TextAnnotation;
import edu.illinois.cs.cogcomp.nlp.corpusreaders.ACEReader;
import edu.stanford.nlp.simple.*;

import java.io.BufferedWriter;
import java.io.FileWriter;
import java.util.ArrayList;
import java.util.List;


public class AceProcess {
    public static void main(String[] args) throws Exception {
        String corpusHome = args[0];
        String outputFilename = args[1];
        BufferedWriter tokensWriter = new BufferedWriter(new FileWriter(outputFilename + ".tokens.txt"));
        BufferedWriter lemmasWriter = new BufferedWriter(new FileWriter(outputFilename + ".lemmas.txt"));
        BufferedWriter posWriter = new BufferedWriter(new FileWriter(outputFilename + ".pos.txt"));
        BufferedWriter nerWriter = new BufferedWriter(new FileWriter(outputFilename + ".ner.txt"));
        BufferedWriter depWriter = new BufferedWriter(new FileWriter(outputFilename + ".dep.txt"));
        ACEReader reader = new ACEReader(corpusHome, false);
        while (reader.hasNext()) {
            TextAnnotation doc = reader.next();
            String doc_text = doc.getText();
            Document document = new Document(doc_text);
            for (Sentence sent: document.sentences()){
                List<String> tokens = sent.words();
                List<String> lemmas = sent.lemmas();
                List<String> posTags = sent.posTags();
                List<String> nerTags = sent.nerTags();
                List<String> deps = new ArrayList<>();
                for (int i = 0; i < tokens.size(); i++) {
                    deps.add(Integer.toString(sent.governor(i).get() + 1) + '_' + sent.incomingDependencyLabel(i).get());
                }
                // Writing to file
                tokensWriter.write(Joiner.on(' ').join(tokens) + '\n');
                lemmasWriter.write(Joiner.on(' ').join(lemmas) + '\n');
                posWriter.write(Joiner.on(' ').join(posTags) + '\n');
                nerWriter.write(Joiner.on(' ').join(nerTags) + '\n');
                depWriter.write(Joiner.on(' ').join(deps) + '\n');
            }
        }
        tokensWriter.close();
        lemmasWriter.close();
        posWriter.close();
        nerWriter.close();
        depWriter.close();
    }
}
