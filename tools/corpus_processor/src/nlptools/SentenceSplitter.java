package nlptools;

import com.google.common.base.Charsets;
import com.google.common.base.Joiner;
import com.google.common.io.Files;
import edu.stanford.nlp.ling.CoreAnnotations.SentencesAnnotation;
import edu.stanford.nlp.ling.CoreAnnotations.TextAnnotation;
import edu.stanford.nlp.ling.CoreAnnotations.TokensAnnotation;
import edu.stanford.nlp.ling.CoreLabel;
import edu.stanford.nlp.pipeline.Annotation;
import edu.stanford.nlp.pipeline.StanfordCoreNLP;
import edu.stanford.nlp.util.CoreMap;

import java.io.IOException;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.List;
import java.util.Properties;

public class SentenceSplitter {

    private StanfordCoreNLP pipeline;

    private SentenceSplitter() throws IOException {
        // Create StanfordCoreNLP object properties, with POS tagging
        // (required for lemmatization), and lemmatization
        Properties props;
        props = new Properties();
        props.put("annotators", "tokenize, ssplit");
        props.put("ssplit.boundaryTokenRegex", "\\.|[!?;:]+");

        /*
         * This is a pipeline that takes in a string and returns various analyzed linguistic forms.
         * The String is tokenized via a tokenizer (such as PTBTokenizerAnnotator),
         * and then other sequence model style annotation can be used to add things like lemmas,
         * POS tags, and named entities. These are returned as a list of CoreLabels.
         * Other analysis components build and store parse trees, dependency graphs, etc.
         *
         * This class is designed to apply multiple Annotators to an Annotation.
         * The idea is that you first build up the pipeline by adding Annotators,
         * and then you take the objects you wish to annotate and pass them in and
         * get in return a fully annotated object.
         *
         *  StanfordCoreNLP loads a lot of models, so you probably
         *  only want to do this once per execution
         */
        this.pipeline = new StanfordCoreNLP(props);
    }


    private List<String> splitBySentence(String documentText) {
        List<String> sentenceStrings = new ArrayList<>();
        Annotation document = new Annotation(documentText);
        this.pipeline.annotate(document);
        List<CoreMap> sentences = document.get(SentencesAnnotation.class);
        for (CoreMap sentence : sentences) {
            sentenceStrings.add(sentence.get(TextAnnotation.class).replaceAll("[\\t\\n\\r]+", " ").replaceAll("_", ""));
        }
        return sentenceStrings;
    }

    private List<List<String>> tokenizeBySentence(String documentText) {
        List<List<String>> tokensBySentence = new ArrayList<>();
        // Create an empty Annotation just with the given text
        Annotation document = new Annotation(documentText);
        // run all Annotators on this text
        this.pipeline.annotate(document);
        // Iterate over all of the sentences found
        List<CoreMap> sentences = document.get(SentencesAnnotation.class);
        for (CoreMap sentence : sentences) {
            // Iterate over all tokens in a sentence
            List<String> sentenceTokens = new ArrayList<>();
            for (CoreLabel token : sentence.get(TokensAnnotation.class)) {
                // Retrieve and add the lemma for each word into the
                // list of lemmas
                sentenceTokens.add(token.get(TextAnnotation.class));
            }
            tokensBySentence.add(sentenceTokens);
        }
        return tokensBySentence;
    }

    public static void main(String[] args) throws IOException {
        if (args.length < 2) {
            System.out.println("ssplit: Missing source text directory, output directory");
            return;
        }
        Path sourcePath = Paths.get(args[0]);
        Path outputPath = Paths.get(args[1]);
        SentenceSplitter ssplit = new SentenceSplitter();

        FileManager fileManager = new FileManager(sourcePath);
        List<Path> files = fileManager.getFiles();
        int totalFiles = files.size();
        int count = 0;
        for (Path path : files) {
            count += 1;
            System.out.println(count + "/" + totalFiles);

            String text = FileManager.readFile(path);
            List<String> sentences = ssplit.splitBySentence(text);
            String sentenceLines = Joiner.on('\n').join(sentences);

            Path filename = path.getFileName();
            Path lemmaFilePath = outputPath.resolve(filename + ".ssplit");
            Files.write(sentenceLines, lemmaFilePath.toFile(), Charsets.UTF_8);
        }
    }
}
